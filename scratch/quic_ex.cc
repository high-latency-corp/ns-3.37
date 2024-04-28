#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/socket.h"
#include "ns3/quic-socket.h"
#include "ns3/quic-socket-factory.h"
#include "ns3/quic-helper.h"

int
main (int argc, char *argv[])
{
    ns3::NodeContainer nodes;
    nodes.Create (1);
    
    ns3::QuicHelper stack;
    stack.InstallQuic (nodes);
    
    ns3::Ptr<ns3::Node> node = nodes.Get(0);
        
    ns3::TypeId tid = ns3::TypeId::LookupByName ("ns3::QuicSocketFactory");
    
    ns3::Ptr<ns3::Socket> m_socket = 0;
    m_socket = ns3::Socket::CreateSocket (node, tid);
    
    ns3::Simulator::Run ();
    ns3::Simulator::Destroy ();
    return 0;
} 

