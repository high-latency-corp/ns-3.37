/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 ResiliNets, ITTC, University of Kansas
 * Copyright (c) 2019 SIGNET Lab, Department of Information Engineering, University of Padova
 *
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
 *
 * Authors of the original TCP example:
 * Justin P. Rohrer, Truc Anh N. Nguyen <annguyen@ittc.ku.edu>, Siddharth Gangadhar <siddharth@ittc.ku.edu>
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  http://wiki.ittc.ku.edu/resilinets
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
å * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 *
 * Adapted to QUIC by:
 *          Alvise De Biasio <alvise.debiasio@gmail.com>
 *          Federico Chiariotti <chiariotti.federico@gmail.com>
 *          Michele Polese <michele.polese@gmail.com>
 *          Davide Marcato <davidemarcato@outlook.com>
 *
 */

#include <iostream>
#include <fstream>
#include <string>
#include <atomic>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/quic-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/error-model.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"
#include "ns3/enum.h"
#include "ns3/event-id.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"
#include "ns3/ipv4-flow-classifier.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("QuicVariantsComparisonBulkSend");
std::atomic_int receptionSize = 0;
// connect to a number of traces
static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}

static void
RttChange (Ptr<OutputStreamWrapper> stream, Time oldRtt, Time newRtt)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldRtt.GetSeconds () << "\t" << newRtt.GetSeconds () << std::endl;
}

static void
Rx (Ptr<OutputStreamWrapper> stream,  Ptr<const Packet> p, const QuicHeader& q, Ptr<const QuicSocketBase> qsb)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << p->GetSize() << std::endl;
  receptionSize += p->GetSize();
}



void
PacketArrivalCallback(Ptr<const Packet> packet, const Address& from) {
    receptionSize += packet->GetSize();
}

void
ThroughputCallback(Ptr<OutputStreamWrapper> stream){
   auto tput = receptionSize * 8.0 / 1000 / 1000;
   receptionSize =0;
   *stream->GetStream () << Simulator::Now ().GetSeconds () << "," << tput << std::endl;
}


static void
Traces(uint32_t serverId, std::string pathVersion, std::string finalPart)
{
  AsciiTraceHelper asciiTraceHelper;

  std::ostringstream pathCW;
  pathCW << "/NodeList/" << serverId << "/$ns3::QuicL4Protocol/SocketList/0/QuicSocketBase/CongestionWindow";
  NS_LOG_INFO("Matches cw " << Config::LookupMatches(pathCW.str().c_str()).GetN());

  std::ostringstream fileCW;
  fileCW << pathVersion << "QUIC-cwnd-change"  << serverId << "" << finalPart;

  std::ostringstream pathRTT;
  pathRTT << "/NodeList/" << serverId << "/$ns3::QuicL4Protocol/SocketList/0/QuicSocketBase/RTT";

  std::ostringstream fileRTT;
  fileRTT << pathVersion << "QUIC-rtt"  << serverId << "" << finalPart;

  std::ostringstream pathRCWnd;
  pathRCWnd<< "/NodeList/" << serverId << "/$ns3::QuicL4Protocol/SocketList/0/QuicSocketBase/RWND";

  std::ostringstream fileRCWnd;
  fileRCWnd<<pathVersion << "QUIC-rwnd-change"  << serverId << "" << finalPart;

  std::ostringstream fileName;
  fileName << pathVersion << "QUIC-rx-data" << serverId << "" << finalPart;
  std::ostringstream pathRx;
  pathRx << "/NodeList/" << serverId << "/$ns3::QuicL4Protocol/SocketList/*/QuicSocketBase/Rx";
  NS_LOG_INFO("Matches rx " << Config::LookupMatches(pathRx.str().c_str()).GetN());

  Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (fileName.str ().c_str ());
  Config::ConnectWithoutContext (pathRx.str ().c_str (), MakeBoundCallback (&Rx, stream));

  Ptr<OutputStreamWrapper> stream1 = asciiTraceHelper.CreateFileStream (fileCW.str ().c_str ());
  Config::ConnectWithoutContext (pathCW.str ().c_str (), MakeBoundCallback(&CwndChange, stream1));

  Ptr<OutputStreamWrapper> stream2 = asciiTraceHelper.CreateFileStream (fileRTT.str ().c_str ());
  Config::ConnectWithoutContext (pathRTT.str ().c_str (), MakeBoundCallback(&RttChange, stream2));

  Ptr<OutputStreamWrapper> stream4 = asciiTraceHelper.CreateFileStream (fileRCWnd.str ().c_str ());
  Config::ConnectWithoutContextFailSafe (pathRCWnd.str ().c_str (), MakeBoundCallback(&CwndChange, stream4));
}

int main (int argc, char *argv[])
{
  std::string transport_prot = "TcpHybla";
  double error_p = 0.1;
  std::string bandwidth = "100Mbps";
  std::string delay = "2.5ms";
  std::string access_bandwidth = "100Mbps";
  std::string access_delay = "500ms";
  bool tracing = false;
  std::string prefix_file_name = "QuicVariantsComparison";
  double data_mbytes = 1024;
  uint32_t mtu_bytes = 1400;
  uint16_t num_flows = 1;
  float duration = 400;
  uint32_t run = 0;
  bool flow_monitor = true;
  bool pcap = true;
  std::string queue_disc_type = "ns3::PfifoFastQueueDisc";

  LogComponentEnable ("Config", LOG_LEVEL_ALL);
  CommandLine cmd;
  cmd.AddValue ("transport_prot", "Transport protocol to use: TcpNewReno, "
                "TcpHybla, TcpHighSpeed, TcpHtcp, TcpVegas, TcpScalable, TcpVeno, "
                "TcpBic, TcpYeah, TcpIllinois, TcpLedbat", transport_prot);
  cmd.AddValue ("error_p", "Packet error rate", error_p);
  cmd.AddValue ("bandwidth", "Bottleneck bandwidth", bandwidth);
  cmd.AddValue ("delay", "Bottleneck delay", delay);
  cmd.AddValue ("access_bandwidth", "Access link bandwidth", access_bandwidth);
  cmd.AddValue ("access_delay", "Access link delay", access_delay);
  cmd.AddValue ("tracing", "Flag to enable/disable tracing", tracing);
  cmd.AddValue ("prefix_name", "Prefix of output trace file", prefix_file_name);
  cmd.AddValue ("data", "Number of Megabytes of data to transmit", data_mbytes);
  cmd.AddValue ("mtu", "Size of IP packets to send in bytes", mtu_bytes);
  cmd.AddValue ("num_flows", "Number of flows", num_flows);
  cmd.AddValue ("duration", "Time to allow flows to run in seconds", duration);
  cmd.AddValue ("run", "Run index (for setting repeatable seeds)", run);
  cmd.AddValue ("flow_monitor", "Enable flow monitor", flow_monitor);
  cmd.AddValue ("pcap_tracing", "Enable or disable PCAP tracing", pcap);
  cmd.AddValue ("queue_disc_type", "Queue disc type for gateway (e.g. ns3::CoDelQueueDisc)", queue_disc_type);
  cmd.Parse (argc, argv);

  transport_prot = std::string ("ns3::") + transport_prot;
  SeedManager::SetSeed (31462);
  SeedManager::SetRun (1);

  // User may find it convenient to enable logging
  Time::SetResolution (Time::NS);
  LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  //LogComponentEnable("QuicVariantsComparison", LOG_LEVEL_ALL);
  LogComponentEnable("BulkSendApplication", LOG_LEVEL_INFO);
  LogComponentEnable("PfifoFastQueueDisc", LOG_LEVEL_ALL);
  LogComponentEnable ("QuicSocketBase", LOG_LEVEL_ALL);
  LogComponentEnable("TcpVegas", LOG_LEVEL_ALL);
  LogComponentEnable("QuicBbr", LOG_LEVEL_ALL);
  LogComponentEnable("QuicL5Protocol", LOG_LEVEL_ALL);

  // Set the simulation start and stop time
  float start_time = 0.0;
  float stop_time = start_time + duration;

  // 4 MB of TCP buffer
  Config::SetDefault ("ns3::QuicSocketBase::SocketRcvBufSize", UintegerValue (1 << 21));
  Config::SetDefault ("ns3::QuicSocketBase::SocketSndBufSize", UintegerValue (1 << 21));
  Config::SetDefault ("ns3::QuicStreamBase::StreamSndBufSize", UintegerValue (1 << 21));
  Config::SetDefault ("ns3::QuicStreamBase::StreamRcvBufSize", UintegerValue (1 << 21));

  TypeId tcpTid;
  NS_ABORT_MSG_UNLESS (TypeId::LookupByNameFailSafe (transport_prot, &tcpTid), "TypeId " << transport_prot << " not found");
  Config::SetDefault ("ns3::QuicL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName (transport_prot)));


  // Create gateways, sources, and sinks
  NodeContainer gateways;
  gateways.Create (2);
  NodeContainer sources;
  sources.Create (1);
  NodeContainer sinks;
  sinks.Create (1);

  // Configure the error model
  // Here we use RateErrorModel with packet error rate
  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
  uv->SetStream (50);
  RateErrorModel error_model;
  error_model.SetRandomVariable (uv);
  error_model.SetUnit (RateErrorModel::ERROR_UNIT_PACKET);
  error_model.SetRate (error_p);

  PointToPointHelper BottleneckLink;
  BottleneckLink.SetDeviceAttribute ("DataRate",  StringValue ("100Mbps"));
  BottleneckLink.SetChannelAttribute ("Delay", StringValue (delay));
  BottleneckLink.SetDeviceAttribute ("ReceiveErrorModel", PointerValue (&error_model));

  PointToPointHelper AccessLink;
  AccessLink.SetDeviceAttribute ("DataRate",  StringValue ("100Mbps"));
  AccessLink.SetChannelAttribute ("Delay", StringValue (access_delay));

  QuicHelper stack;
  stack.InstallQuic (sources);
  stack.InstallQuic (sinks);
  stack.InstallQuic (gateways);

  TrafficControlHelper tchPfifo;
  tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");

  TrafficControlHelper tchCoDel;
  tchCoDel.SetRootQueueDisc ("ns3::CoDelQueueDisc");

  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.255.255.0");

  // Configure the sources and sinks net devices
  // and the channels between the sources/sinks and the gateways
  PointToPointHelper LocalLink;
  LocalLink.SetDeviceAttribute ("DataRate", StringValue (access_bandwidth));
  LocalLink.SetChannelAttribute ("Delay", StringValue (access_delay));

  Ipv4InterfaceContainer sink_interfaces;

  DataRate access_b (access_bandwidth);
  DataRate bottle_b (bandwidth);
  Time access_d (access_delay);
  Time bottle_d (delay);

  uint32_t size = (std::min (access_b, bottle_b).GetBitRate () / 8) *
    ((access_d + bottle_d) * 2).GetSeconds ();

  Config::SetDefault ("ns3::PfifoFastQueueDisc::MaxSize",
                      QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, size / mtu_bytes)));
  Config::SetDefault ("ns3::CoDelQueueDisc::MaxSize",
                      QueueSizeValue (QueueSize (QueueSizeUnit::BYTES, size)));


      NetDeviceContainer devices;
      devices = AccessLink.Install (sources.Get (0), gateways.Get (0));
      tchPfifo.Install (devices);
      address.NewNetwork ();
      Ipv4InterfaceContainer interfaces = address.Assign (devices);

      devices = LocalLink.Install (gateways.Get (1), sinks.Get (0));

      tchPfifo.Install (devices);


      address.NewNetwork ();
      interfaces = address.Assign (devices);
      sink_interfaces.Add (interfaces.Get (1));

      devices = BottleneckLink.Install (gateways.Get (0), gateways.Get (1));
      tchPfifo.Install (devices);

      address.NewNetwork ();
      interfaces = address.Assign (devices);

  NS_LOG_INFO ("Initialize Global Routing.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  uint16_t port = 50000;
  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));

  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  // applications client and server

      AddressValue remoteAddress (InetSocketAddress (sink_interfaces.GetAddress (0, 0), port));
      BulkSendHelper ftp ("ns3::QuicSocketFactory", Address ());
      ftp.SetAttribute ("Remote", remoteAddress);
      ftp.SetAttribute ("SendSize", UintegerValue (mtu_bytes));
      ftp.SetAttribute ("MaxBytes", UintegerValue (0));
      clientApps.Add(ftp.Install (sources.Get (0)));
      PacketSinkHelper sinkHelper ("ns3::QuicSocketFactory", sinkLocalAddress);
      sinkHelper.SetAttribute ("Protocol", TypeIdValue (QuicSocketFactory::GetTypeId ()));
      serverApps.Add(sinkHelper.Install (sinks.Get (0)));


  serverApps.Start (Seconds (0));
  clientApps.Stop (Seconds (80.0));
  clientApps.Start (Seconds (2));


      auto n2 = sinks.Get (0);
      auto n1 = sources.Get (0);
      Time t = Seconds(2.11);
      Simulator::Schedule (t, &Traces, n2->GetId(),
            "./server", ".txt");
      Simulator::Schedule (t, &Traces, n1->GetId(),
            "./client", ".txt");

  AsciiTraceHelper asciiTraceHelper;
  auto stream =asciiTraceHelper.CreateFileStream("sinkTput.csv");
  for(int k =0; k < int(duration); k++)
  {
    Simulator::Schedule(Seconds(k), &ThroughputCallback, stream);

  }
  if (false)
    {
      BottleneckLink.EnablePcapAll (prefix_file_name + "_bn_" + transport_prot , true);
      LocalLink.EnablePcapAll (prefix_file_name + "_ll_" + transport_prot  , true);
      AccessLink.EnablePcapAll (prefix_file_name + "_al_" + transport_prot , true);
    }

  Ptr<FlowMonitor> monitor;
  // Flow monitor
  FlowMonitorHelper flowHelper;
  if (flow_monitor)
    {
     monitor = flowHelper.InstallAll ();
    }

  Simulator::Stop (Seconds (stop_time));
  monitor->CheckForLostPackets ();
  Simulator::Run ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowHelper.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)

    {

    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);

    std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
     std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1000/1000  << " Mbps\n";
    }
  if (flow_monitor)
    {
      flowHelper.SerializeToXmlFile (prefix_file_name + ".flowmonitor", true, true);
    }

  Simulator::Destroy ();
  return 0;
}
