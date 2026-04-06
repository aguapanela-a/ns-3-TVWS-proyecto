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
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

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

    //  ->  propiedades de simulación para el escenario de la zona rural de Fusagasuga con TVWS <- //

    int timeStart = 5; // Tiempo de inicio en segundos
    double timeEnd = 20.0;  // Tiempo de fin en segundos
    int pingAmount = 10; // Cantidad de pings enviados por el cliente UDP Echo
    int pingInterval = 1; // Intervalo entre pings en segundos
    int packetSize = 1024; // Tamaño de cada ping en bytes (1 KB)
    double txPower = 23.0; // Potencia de transmisión en dBm
    int frecuencyFriis = 515; // Frecuencia de propagación de Friis en MHz
    double pathLossExponent = 3.0; // Exponente de pérdida de propagación
    int clientNodes = 3; // Cantidad de nodos cliente (CPEs) en la zona rural de Fusagasuga


    //Ajuste de sensibilidad de recepciòn de cada nodo con los valores acordes al dispositivo Adaptrum TVWS para la banda UHF, lo que permite una mejor recepciòn de la señal en condiciones de propagaciòn adversas
    //Config::SetDefault("ns3::WifiPhy::CcaEdThreshold", DoubleValue(-98.0)); // CcaEdThreshold es el umbral de energía para la detección de portadora en dBm, y es acorde a las caracterìsticas del dispositivo Adaptrum TVWS para la banda UHF
    //Config::SetDefault("ns3::WifiPhy::RxSensitivity", DoubleValue(-98.0)); // RxSensitivity es el umbral de potencia de recepciòn en dBm, y es acorde a las caracterìsticas del dispositivo Adaptrum TVWS para la banda UHF

    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    LogComponentEnable("WifiPhy", LOG_LEVEL_INFO);
   
    //Dispositivo TVWS en la zona rural de Fusagasuga
    NodeContainer baseStation;
    baseStation.Create(1);

    //CPE en la zona rural de Fusagasuga
    NodeContainer ruralCPE;
    ruralCPE.Create(clientNodes);


    //configuraciòn para la estaciòn base TVWS
    //capa fìsica

    //se crea un canal de espectro para la estaciòn base TVWS y los CPE en la zona rural de Fusagasuga
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();

    //se encadena el modelo de propagaciòn de Friis (515 mHz de la banda UHF) con el modelo de pèrdida de propagaciòn log-distance para simular las condiciones de propagaciòn en zonas rurales por distancias largas y algunos obstaculos
    channel.AddPropagationLoss("ns3::LogDistancePropagationLossModel", "Exponent", DoubleValue(pathLossExponent));
    channel.AddPropagationLoss("ns3::FriisPropagationLossModel", "Frequency", DoubleValue(frecuencyFriis * 1e6));


    //creaciòn del helper de la capa fisica para usar el canal configurado, y se asigna el canal al helper de la capa fìsica
    YansWifiPhyHelper yansWifiPhy;
    yansWifiPhy.SetChannel(channel.Create());

    //configuraciòn de la capa fìsica para el enlace inalambrico entre la estaciòn base TVWS y los CPE en la zona rural de Fusagasuga, con valores acordes a las caracterìsticas del dispositivo Adaptrum TVWS para la banda UHF, lo que permite una mejor recepciòn de la señal en condiciones de propagaciòn adversas
    //se asigna la potencia de transmisiòn
    yansWifiPhy.Set("TxPowerStart", DoubleValue(txPower));
    yansWifiPhy.Set("TxPowerEnd", DoubleValue(txPower));

    //Configuraciòn de antena externaLog-Periodic de 11dBi de acuerdo a las caracterìsticas de los dispositivos Adaptrum TVWS para la banda UHF, lo que permite una mejor recepciòn de la señal en condiciones de propagaciòn adversas
    yansWifiPhy.Set("TxGain", DoubleValue(11.0)); // Ganancia de transmisión en dBi
    yansWifiPhy.Set("RxGain", DoubleValue(11.0)); // Ganancia de recepción en dBi

    //se asigna la sensibilidad de recepciòn y el umbral de detecciòn de portadora
    yansWifiPhy.Set("RxSensitivity", DoubleValue(-98.0)); // RxSensitivity es el umbral de potencia de recepciòn en dBm, y es acorde a las caracterìsticas del dispositivo Adaptrum TVWS para la banda UHF      
    yansWifiPhy.Set("CcaEdThreshold", DoubleValue(-98.0)); // CcaEdThreshold es el umbral de energía para la detección de portadora en dBm, y es acorde a las caracterìsticas del dispositivo Adaptrum TVWS para la banda UHF
    
    //configuraciòn de MAC para el enlace fìsico inalambrico entre la estaciòn base TVWS y los CPE en la zona rural de Fusagasuga
    WifiMacHelper wifiMacHelper;
    
    //nombre de la red que transmite la estaciòn base TVWS
    Ssid ssid = Ssid("Adaptrum-TVWS-Fusagasuga");
    
    //hacemos que la base TVWS sea un punto de acceso y se ke asigna el nombre de la red con el ssid definido anteriormente
    wifiMacHelper.SetType("ns3::ApWifiMac",
                          "Ssid", SsidValue(ssid));
    //wifiMacHelper.SetType("ns3::AdhocWifiMac"); //hacemos que la base TVWS sea un nodo adhoc, lo que permite una mayor flexibilidad en la comunicaciòn con los CPE en la zona rural de Fusagasuga, pues el dispositivo Adaptrum TVWS es inteligente y se adapta a las condiciones del canal de espectro


    //WifiHelper con un wifi manager ideal, pues el dispositivo Adaptrum TVWS es inteligente y se adapta a las condiciones del canal de espectro
    WifiHelper wifiHelper; 
    wifiHelper.SetStandard(WIFI_STANDARD_80211be); //se configura el helper de wifi para usar el estandar 802.11be, que es el estandar mìnimo requerido para simular el dispositivo Adaptrum TVWS, lo que permite una mejor simulaciòn de las caracterìsticas del dispositivo en la zona rural de Fusagasuga

    //se configura ConstantRateWifiManager para que el dispositivo Adaptrum TVWS transmita a una tasa de datos constante de 1 Mbps, lo que permite
    wifiHelper.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                         "DataMode", StringValue("DsssRate1Mbps"),
                                         "ControlMode", StringValue("DsssRate1Mbps"));


       std::cout << "antes de aplicar coordenadas" << std::endl;

    //crear helper de movilidad para agrupar la asignaciòn de coordenadas a la estaciòn base TVWS y los CPE en la zona rural de Fusagasuga
    MobilityHelper mobility;

    //se crea el posicionAlloc para asignar coordenadas a los nodos
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();

    //se asignan coordenadas a la estaciòn base TVWS y a los CPE en la zona rural de Fusagasuga
    positionAlloc->Add(Vector(0.0, 0.0, 0.0)); // Coordenadas para la estaciòn base TVWS

    positionAlloc->Add(Vector(50.0, 50.0, 0.0)); // Coordenadas para el primer CPE
    positionAlloc->Add(Vector(400.0, 100.0, 0.0)); // Coordenadas para el segundo CPE
    positionAlloc->Add(Vector(70.0, 700.0, 0.0)); // Coordenadas para el tercer CPE

    //se asigna el posicionAlloc al helper de movilidad y se instala en la estaciòn base TVWS y los CPE en la zona rural de Fusagasuga
    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(baseStation);
    mobility.Install(ruralCPE);
    
    std::cout << "antes de instalar dispositivos" << std::endl;

    //se inserta el canal configurador, el MAC y el cntenedor de la etacin base TVWS para crear el dispositivo de red inalambrico
    NetDeviceContainer baseDevice = wifiHelper.Install(yansWifiPhy, wifiMacHelper, baseStation);

    //configuraciòn para los CPE en la zona rural de Fusagasuga
    //se hace que los CPE sean estaciones y se asigna el nombre de la red
    wifiMacHelper.SetType("ns3::StaWifiMac",
                          "Ssid", SsidValue(ssid));

    //wifiMacHelper.SetType("ns3::AdhocWifiMac"); //hacemos que los CPE sean nodos adhoc, lo que permite una mayor flexibilidad en la comunicaciòn con la estaciòn base TVWS, pues el dispositivo Adaptrum TVWS es inteligente y se adapta a las condiciones del canal de espectro

    //se inserta el canal configurador, el MAC y el cntenedor de los CPE para crear los dispositivos de red inalambricos
    NetDeviceContainer cpeDevices = wifiHelper.Install(yansWifiPhy, wifiMacHelper, ruralCPE);


    //configuraciòn de la pila de protocolos TCP/IP para la estaciòn base TVWS y los CPE en la zona rural de Fusagasuga
    InternetStackHelper stack;
    stack.Install(baseStation);
    stack.Install(ruralCPE);


    std::cout << "antes de asignar IPv4" << std::endl;
    //se crea una direcciòn de red IPv4 y una màscara de sub red a la estaciòn base TVWS y los CPE en la zona rural de Fusagasuga
    Ipv4AddressHelper address;
    address.SetBase("192.168.1.0","255.255.255.0");

    //se asigna la IPv4 al contenedor de dispositivos de red de la estaciòn base TVWS (al ser el primero quedarà con ip 192.168.1.1)
    Ipv4InterfaceContainer baseInterface = address.Assign(baseDevice);

    //se asigna la IPv4 al contenedor de dispositivos de red de los CPE en la zona rural de Fusagasuga (quedaran con ip .2, .3 y .4)
    Ipv4InterfaceContainer cpeInterface = address.Assign(cpeDevices);

    //se crea el servidor UDP Echo (eco o ping) en la estaciòn base para comprobar la conectividad con los CPE en la zona rural de Fusagasuga
    UdpEchoServerHelper echoServer(9); // El servidor escucha en el puerto 9



    std::cout << "antes de instalar aplicaciones" << std::endl;
    //crea un contenedor de aplicaciones para estaciòn base y guarda ahì el nodo 0 del contenedor baseStation con el servidor UDP Echo instalado, y se configura para que inicie en el segundo 1.0 y termine en el segundo 10.0 de la simulaciòn
    ApplicationContainer serverApps = echoServer.Install(baseStation.Get(0));
    serverApps.Start(Seconds(timeStart));
    serverApps.Stop(Seconds(timeEnd));

    //se crea el cliente UDP Echo (eco o ping) y se le pasa la direcciòn IPv4 que se asignò al primer dispositivo de red (estaciòn base) y el puerto por el cuàl debe hacer eco
    UdpEchoClientHelper echoClient(baseInterface.GetAddress(0), 9);

    echoClient.SetAttribute("MaxPackets", UintegerValue(pingAmount)); //harà màximo 5 pings
    echoClient.SetAttribute("Interval", TimeValue(Seconds(pingInterval))); //harà un ping cada segundo
    echoClient.SetAttribute("PacketSize", UintegerValue(packetSize)); //harà que cada ping tenga un tamaño de 1024 bytes (1 KB)

    //crea un contenedor de aplicaciones apra los CPEs y guarda ahì el nodo 0 del contenedor ruralCPE con el cliente UDP Echo instalado, y se configura para que inicie en el segundo 2.0 y termine en el segundo 10.0 de la simulaciòn
    ApplicationContainer clientApps = echoClient.Install(ruralCPE.Get(0));
    clientApps.Start(Seconds(timeStart + 1.0)); // Inicia un segundo después del servidor
    clientApps.Stop(Seconds(timeEnd));


    //uso de AnimationInterface para visualizar la topologia de red en NetAnim
    AnimationInterface anim("fusagasuga-anim.xml");

    //activar la visualizaciòn de paquetes en NetAnim
    anim.EnablePacketMetadata(true); // Permite visualizar la metadata de los paquetes en NetAnim, lo que ayuda a entender el flujo de datos entre la estaciòn base TVWS y los CPE en la zona rural de Fusagasuga

    
    anim.UpdateNodeDescription(baseStation.Get(0), "Base TVWS");
    for(uint32_t i = 0; i < ruralCPE.GetN(); ++i)
    {
        anim.UpdateNodeDescription(ruralCPE.Get(i), "CPE Rural");
    }



    std::cout << "lamada a simulator.run" << std::endl;

    //se llama a la funciòn PopulateRoutingTables para que se construya la base de datos de enrutamiento y se inicialicen las tablas de enrutamiento de los nodos en la simulaciòn, lo que permite que los paquetes puedan ser encaminados correctamente entre la estaciòn base TVWS y los CPE en la zona rural de Fusagasuga
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Simulator::Stop(Seconds(timeEnd + 1)); // Detiene la simulación un segundo después del último evento programado
    Simulator::Run();
    Simulator::Destroy();

    std::cout << "Simulation finished." << std::endl;
    return 0;
}
