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

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

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
    
    /*
    //cable punto a punto entre los dispositivos con velocidad de 5Mbps y retraso de 2ms
    PointToPointHelper pointToPoint; 
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    //instalamos el canal punto a punto entre los nodos
    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);

    //instalamos el stack de internet en los nodos (para que puedan usar el protocolo IP)
    InternetStackHelper stack;
    stack.Install(nodes);
    
    //asignamos direcciòn IP y máscara a los dispositivos usando el helper Ipv4AddressHelper
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

    //asignamos direcciones IP a los dispositivos y obtenemos un contenedor de interfaces para usarlas posteriormente
    Ipv4InterfaceContainer interfaces = address.Assign(devices);
    
    //instalamos una aplicación de servidor de eco UDP en el nodo 1 para escuchar en el puerto 9 (echo protocol)
    UdpEchoServerHelper echoServer(9);


    //esto lo que hace es instalar la aplicación de servidor de eco en el nodo 1, y luego iniciar la aplicación en el segundo 1.0 y detenerla en el segundo 10.0 para que no se quede corriendo toda la simulación
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    //instalamos una aplicación de cliente de eco UDP en el nodo 0 para enviar paquetes al servidor en el nodo 1, usando la dirección IP del nodo 1 y el puerto 9 (echo protocol)
    UdpEchoClientHelper echoClient(interfaces.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));  //lo que hace es configurar el cliente para que envíe solo un paquete al servidor, y luego se detenga
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0))); //lo que hace es configurar el cliente para que espere 1 segundo entre cada paquete que envía al servidor, pero como solo va a enviar un paquete, esto no tiene mucho efecto
    echoClient.SetAttribute("PacketSize", UintegerValue(1024)); //lo que hace es configurar el cliente para que envíe paquetes de 1024 bytes al servidor, pero como solo va a enviar un paquete, esto no tiene mucho efecto
    
    //esto lo que hace es instalar la aplicación de cliente de eco en el nodo 0, y luego iniciar la aplicación en el segundo 2.0 y detenerla en el segundo 10.0 para que no se quede corriendo toda la simulación
    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));
    */
   
    //Dispositivo TVWS en la zona rural de Fusagasuga
    NodeContainer baseStation;
    baseStation.Create(1);

    //CPE en la zona rural de Fusagasuga
    NodeContainer ruralCPE;
    ruralCPE.Create(3);




    AnimationInterface anim("fusagasuga-anim.xml");
    anim.SetConstantPosition(baseStation.Get(0), 0, 0); //estaciòn base TVWS
    anim.SetConstantPosition(ruralCPE.Get(0), 8000, 8000);  //en CPE en la zona rural
    anim.SetConstantPosition(ruralCPE.Get(1), 4000, 0);  //en CPE en la zona rural
    anim.SetConstantPosition(ruralCPE.Get(2), 7000, 500); //en CPE en la zona rural

    

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
