/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/spectrum-module.h"
#include "ns3/mobility-module.h"
#include "ns3/position-allocator.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1
//    point-to-point
//

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FirstScriptExample");

int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    //Ajuste de sensibilidad de recepciòn de cada nodo con los valores acordes al dispositivo Adaptrum TVWS para la banda UHF, lo que permite una mejor recepciòn de la señal en condiciones de propagaciòn adversas
    Config::SetDefault("ns3::WifiPhy::CcaEdThreshold", DoubleValue(-98.0)); // CcaEdThreshold es el umbral de energía para la detección de portadora en dBm, y es acorde a las caracterìsticas del dispositivo Adaptrum TVWS para la banda UHF
    Config::SetDefault("ns3::WifiPhy::RxSensitivity", DoubleValue(-98.0)); // RxSensitivity es el umbral de potencia de recepciòn en dBm, y es acorde a las caracterìsticas del dispositivo Adaptrum TVWS para la banda UHF

    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
   
    //Dispositivo TVWS en la zona rural de Fusagasuga
    NodeContainer baseStation;
    baseStation.Create(1);

    //CPE en la zona rural de Fusagasuga
    NodeContainer ruralCPE;
    ruralCPE.Create(3);


    //configuraciòn para la estaciòn base TVWS
    //capa fìsica

    //se crea un canal de espectro para la estaciòn base TVWS y los CPE en la zona rural de Fusagasuga
    Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel>(); 

    //modelo de pèrdida de propagaciòn que simula las condiciones de atenuaciòn por distancia y entorno, con un exponente de pérdida de 3.0 -> algunos obstaculos
    Ptr<LogDistancePropagationLossModel> lossModel = CreateObject<LogDistancePropagationLossModel>();
    lossModel->SetAttribute("Exponent", DoubleValue(3.0)); // Exponente

    //se crea un modelo de perdida de propagacion de Friis para simular las condiciones de propagaciòn en zonas rurales, con una frecuencia de 515 MHz para la banda UHF
    Ptr<FriisPropagationLossModel> friisModel = CreateObject<FriisPropagationLossModel>();
    friisModel->SetAttribute("Frequency", DoubleValue(515e6)); // Frecuencia de 515 MHz para la banda UHF

    //se encadena el modelo de propagaciòn de Friis con el modelo de pèrdida de propagaciòn log-distance para simular las condiciones de propagaciòn en zonas rurales por distancias largas y algunos obstaculos
    friisModel->SetNext(lossModel); 
    channel->AddPropagationLossModel(friisModel); //se asigna el modelo de propagaciòn encadenado al canal de espectro

    
    //configuraciòn de MAC para el enlace fìsico inalambrico entre la estaciòn base TVWS y los CPE en la zona rural de Fusagasuga
    WifiMacHelper wifiMacHelper;
    
    //nombre de la red que transmite la estaciòn base TVWS
    Ssid ssid = Ssid("Adaptrum-TVWS-Fusagasuga");
    
    //hacemos que la base TVWS sea un punto de acceso y se ke asigna el nombre de la red con el ssid definido anteriormente
    wifiMacHelper.SetType("ns3::ApWifiMac",
                          "Ssid", SsidValue(ssid));


    //WifiHelper con un wifi manager ideal, pues el dispositivo Adaptrum TVWS es inteligente y se adapta a las condiciones del canal de espectro
    WifiHelper wifiHelper; 
    wifiHelper.SetStandard(WIFI_STANDARD_80211a);
    wifiHelper.SetRemoteStationManager("ns3::IdealWifiManager");

    //se inicializa un auxiliar para configurar el canal de espectro con las caracterìsticas de las zonas rurales
    SpectrumWifiPhyHelper wifiPhyHelper;

    //se asigna el espectro a la capa fìsica con el auxiliar de wifi para la banda UHF
    wifiPhyHelper.SetChannel(channel);
//    wifiPhyHelper.AddChannel(channel, {470, 698}); // Banda WiFi UHF (470-698 MHz)  
//    wifiPhyHelper.Set("ChannelSettings", StringValue("{channelNumber: 21, channelWidth: 6, BandName: BAND_470MHZ}"));
 
    
    //se asigna un rango de potencia de transmision de 20.0dBm al canal de espectro, pues es el màximo que soporta el dipositivo Adaptrum TVWS, lo que permite alcanzar distancias largas en zonas rurales
    wifiPhyHelper.Set("TxPowerStart", DoubleValue(20.0));
    wifiPhyHelper.Set("TxPowerEnd", DoubleValue(20.0));

    
    std::cout << "antes de instalar dispositivos" << std::endl;

    //se inserta el canal configurador, el MAC y el cntenedor de la etacin base TVWS para crear el dispositivo de red inalambrico
    NetDeviceContainer baseDevice = wifiHelper.Install(wifiPhyHelper, wifiMacHelper, baseStation);

    //configuraciòn para los CPE en la zona rural de Fusagasuga
    //se hace que los CPE sean estaciones y se asigna el nombre de la red
    wifiMacHelper.SetType("ns3::StaWifiMac",
                          "Ssid", SsidValue(ssid));

    //se inserta el canal configurador, el MAC y el cntenedor de los CPE para crear los dispositivos de red inalambricos
    NetDeviceContainer cpeDevices = wifiHelper.Install(wifiPhyHelper, wifiMacHelper, ruralCPE);


    std::cout << "antes de aplicar coordenadas" << std::endl;

    //crear helper de movilidad para agru
    MobilityHelper mobility;

    //se crea el posicionAlloc para asignar coordenadas a los nodos
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();

    //se asignan coordenadas a la estaciòn base TVWS y a los CPE en la zona rural de Fusagasuga
    positionAlloc->Add(Vector(0.0, 0.0, 0.0)); // Coordenadas para la estaciòn base TVWS

    positionAlloc->Add(Vector(700.0, 850.0, 0.0)); // Coordenadas para el primer CPE
    positionAlloc->Add(Vector(400.0, 100.0, 0.0)); // Coordenadas para el segundo CPE
    positionAlloc->Add(Vector(70.0, 700.0, 0.0)); // Coordenadas para el tercer CPE

    //se asigna el posicionAlloc al helper de movilidad y se instala en la estaciòn base TVWS y los CPE en la zona rural de Fusagasuga
    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(baseStation);
    mobility.Install(ruralCPE);

    //uso de AnimationInterface para visualizar la topologia de red en NetAnim
    AnimationInterface anim("fusagasuga-anim.xml");

    anim.UpdateNodeDescription(baseStation.Get(0), "Base TVWS");
    for(uint32_t i = 0; i < ruralCPE.GetN(); ++i)
    {
        anim.UpdateNodeDescription(ruralCPE.Get(i), "CPE Rural");
    }


    Simulator::Stop(Seconds(10.0));
    Simulator::Run();

    std::cout << "Simulation finished." << std::endl;

    Simulator::Destroy();
    return 0;
}
