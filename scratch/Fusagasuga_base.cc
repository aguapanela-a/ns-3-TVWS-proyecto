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

    //modelo de pèrdida de propagaciòn que simula las condiciones de propagaciòn en zonas rurales, con un exponente de pérdida de 3.0 -> algunos obstaculos
    Ptr<LogDistancePropagationLossModel> lossModel = CreateObject<LogDistancePropagationLossModel>();
    lossModel->SetAttribute("Exponent", DoubleValue(3.0)); // Exponente
    
    //configuraciòn de MAC para el enlace fìsico inalambrico entre la estaciòn base TVWS y los CPE en la zona rural de Fusagasuga
    WifiMacHelper wifiMacHelper;
    
    //nombre de la red que transmite la estaciòn base TVWS
    Ssid ssid = Ssid("Adaptrum-TVWS-Fusagasuga");
    
    //hacemos que la base TVWS sea un punto de acceso
    wifiMacHelper.SetType("ns3::ApWifiMac",
                          "Ssid", SsidValue(ssid));


    //WifiHelper con un wifi manager ideal, pues el dispositivo Adaptrum TVWS es inteligente y se adapta a las condiciones del canal de espectro
    WifiHelper wifiHelper; 
    wifiHelper.SetStandard(WIFI_STANDARD_80211a);
    wifiHelper.SetRemoteStationManager("ns3::IdealWifiManager");
    
    //se crea un modelo de perdida de propagacion de Friis para simular las condiciones de propagaciòn en zonas rurales, con una frecuencia de 515 MHz para la banda UHF
    Ptr<FriisPropagationLossModel> friisModel = CreateObject<FriisPropagationLossModel>();
    friisModel->SetAttribute("Frequency", DoubleValue(515e6)); // Frecuencia de 515 MHz para la banda UHF

    //se encadena el modelo de propagaciòn de Friis con el modelo de pèrdida de propagaciòn log-distance para simular las condiciones de propagaciòn en zonas rurales por distancias largas y algunos obstaculos
    friisModel->SetNext(lossModel); 
    channel->AddPropagationLossModel(friisModel); //se asigna el modelo de propagaciòn encadenado al canal de espectro

    //se inicializa un auxiliar para configurar el canal de espectro con las caracterìsticas de las zonas rurales
    SpectrumWifiPhyHelper wifiPhyHelper;

    //se asigna el espectro a la capa fìsica con el auxiliar de wifi para la banda UHF
    wifiPhyHelper.SetChannel(channel);
//    wifiPhyHelper.AddChannel(channel, {470, 698}); // Banda WiFi UHF (470-698 MHz)  
//    wifiPhyHelper.Set("ChannelSettings", StringValue("{channelNumber: 21, channelWidth: 6, BandName: BAND_470MHZ}"));
 
    
    //se asigna un rango de potencia de transmision de 20dBm al canal de espectro con el auxiliar de wifi
    wifiPhyHelper.Set("TxPowerStart", DoubleValue(20));
    wifiPhyHelper.Set("TxPowerEnd", DoubleValue(20));


    //se inserta el canal configurador, el MAC y el cntenedor de la etacin base TVWS para crear el dispositivo de red inalambrico
    NetDeviceContainer baseDevice = wifiHelper.Install(wifiPhyHelper, wifiMacHelper, baseStation);


    //uso de AnimationInterface para visualizar la topologia de red en NetAnim
    AnimationInterface anim("fusagasuga-anim.xml");
    anim.SetConstantPosition(baseStation.Get(0), 0, 0); //estaciòn base TVWS
    anim.SetConstantPosition(ruralCPE.Get(0), 800, 800);  //en CPE en la zona rural
    anim.SetConstantPosition(ruralCPE.Get(1), 400, 0);  //en CPE en la zona rural
    anim.SetConstantPosition(ruralCPE.Get(2), 700, 50); //en CPE en la zona rural


    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
