// This is a C++ program for a point-to-point network topology example
// It simulates a two-node network consisting of a UDP echo application client and a UDP echo application server
// The program is designed and simulated by G11 FOE-UOR as an undergraduate project in 2023

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PointToPointScriptExample"); // define log component for debugging purposes

int main (int argc, char *argv[])
{
CommandLine cmd (FILE);
cmd.Parse (argc, argv);

Time::SetResolution (Time::NS); // set time resolution
LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO); // enable logging for client application
LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO); // enable logging for server application

NodeContainer nodes; // create node container for two nodes
nodes.Create (2);

PointToPointHelper pointToPoint; // create point-to-point helper
pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps")); // set data rate to 5 Mbps
pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms")); // set delay to 2ms

NetDeviceContainer devices; // create network device container
devices = pointToPoint.Install (nodes); // install devices on the nodes

InternetStackHelper stack; // create internet stack helper
stack.Install (nodes); // install stack on the nodes

Ipv4AddressHelper address; // create IP address helper
address.SetBase ("10.1.1.0", "255.255.255.0"); // set IP address and subnet mask

Ipv4InterfaceContainer interfaces = address.Assign (devices); // assign IP addresses to the devices

UdpEchoServerHelper echoServer (9); // create server application on port

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

