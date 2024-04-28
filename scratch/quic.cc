/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions
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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 *
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/satellite-module.h"
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
#include <iostream>

using namespace ns3;

/**
 * \file sat-onoff-example.cc
 * \ingroup satellite
 *
 * \brief  OnOff example application to use satellite network.
 *         Data rate, packet size, sender(s), on/off times, test scenario and
 *         creation log file name can be given in command line as user argument.
 *         To see help for user arguments:
 *         execute command -> ./waf --run "on-off-example --PrintHelp"
 *
 *         On-Off application send packets from GW connected user
 *         to UT connected user and after that from UT connected user to GW connected
 *         user according to given parameters.
 */

NS_LOG_COMPONENT_DEFINE("OnOff-example");

int
main(int argc, char* argv[])
{
    uint32_t packetSize = 512;
    std::string dataRate = "500kb/s";
    std::string onTime = "1.0";
    std::string offTime = "0.5";
    std::string scenario = "simple";
    std::string sender = "both";
    std::string simDuration = "11s";
    std::string transport_prot = "TcpHybla";

    /// Set simulation output details
    auto simulationHelper = CreateObject<SimulationHelper>("example-onoff");
    Config::SetDefault("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue(true));
    Config::SetDefault("ns3::SatHelper::ScenarioCreationTraceEnabled", BooleanValue(true));

    // enable packet traces on satellite modules
    Config::SetDefault("ns3::SatHelper::PacketTraceEnabled", BooleanValue(true));
    transport_prot = std::string ("ns3::") + transport_prot;
    Config::SetDefault ("ns3::QuicL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName (transport_prot)));

    SatHelper::PreDefinedScenario_t satScenario = SatHelper::SIMPLE;

    // read command line parameters given by user
    CommandLine cmd;
    cmd.AddValue("packetSize", "Size of constant packet (bytes e.g 512)", packetSize);
    cmd.AddValue("dataRate", "Data rate (e.g. 500kb/s)", dataRate);
    cmd.AddValue("onTime", "Time for packet sending is on in seconds, (e.g. (1.0)", onTime);
    cmd.AddValue("offTime", "Time for packet sending is off in seconds, (e.g. (0.5)", offTime);
    cmd.AddValue("sender", "Packet sender (ut, gw, or both).", sender);
    cmd.AddValue("scenario", "Test scenario to use. (simple, larger or full", scenario);
    cmd.AddValue("simDuration", "Duration of the simulation (Time)", simDuration);
    cmd.AddValue ("transport_prot", "Transport protocol to use: TcpNewReno, "
                "TcpHybla, TcpHighSpeed, TcpHtcp, TcpVegas, TcpScalable, TcpVeno, "
                "TcpBic, TcpYeah, TcpIllinois, TcpLedbat", transport_prot);
    simulationHelper->AddDefaultUiArguments(cmd);
    cmd.Parse(argc, argv);

    simulationHelper->SetSimulationTime(Time(simDuration));
    simulationHelper->SetOutputTag(scenario);

    // select scenario, if correct one given, by default simple scenarion is used.
    if (scenario == "larger")
    {
        satScenario = SatHelper::LARGER;
    }
    else if (scenario == "full")
    {
        satScenario = SatHelper::FULL;
    }

    // Set up user given parameters for on/off functionality.
    Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(packetSize));
    Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue(dataRate));
    Config::SetDefault("ns3::OnOffApplication::OnTime",
                       StringValue("ns3::ConstantRandomVariable[Constant=" + onTime + "]"));
    Config::SetDefault("ns3::OnOffApplication::OffTime",
                       StringValue("ns3::ConstantRandomVariable[Constant=" + offTime + "]"));

    // enable info logs
    LogComponentEnable("OnOffApplication", LOG_LEVEL_INFO);
    LogComponentEnable("PacketSink", LOG_LEVEL_INFO);
    LogComponentEnable("OnOff-example", LOG_LEVEL_INFO);

    // remove next line from comments to run real time simulation
    // GlobalValue::Bind ("SimulatorImplementationType", StringValue
    // ("ns3::RealtimeSimulatorImpl"));

    // create satellite helper with given scenario default=simple

    // Creating the reference system. Note, currently the satellite module supports
    // only one reference system, which is named as "Scenario72". The string is utilized
    // in mapping the scenario to the needed reference system configuration files. Arbitrary
    // scenario name results in fatal error.
    Ptr<SatHelper> m_satHelper = simulationHelper->CreateSatScenario(satScenario);

    // --- Create applications according to given user parameters

    // assert if sender is not valid
    NS_ASSERT_MSG(((sender == "gw") || (sender == "ut") || (sender == "both")),
                  "Sender argument invalid.");

    std::string socketFactory ="ns3::QuicSocketFactory";

    // get users
    NodeContainer utUsers = m_satHelper->GetUtUsers();
    NodeContainer gwUsers = m_satHelper->GetGwUsers();
    uint16_t port = 9;
    InetSocketAddress gwUserAddr =
        InetSocketAddress(m_satHelper->GetUserAddress(gwUsers.Get(0)), port);

    PacketSinkHelper sinkHelper(socketFactory, Address());
    SatOnOffHelper onOffHelper(socketFactory, Address());
    ApplicationContainer sinkContainer;
    ApplicationContainer onOffContainer;

    // create OnOff applications on UT users
    for (uint32_t i = 0; i < utUsers.GetN(); i++)
    {
        InetSocketAddress utUserAddr =
                    InetSocketAddress(m_satHelper->GetUserAddress(utUsers.Get(i)), port);


                sinkHelper.SetAttribute("Local", AddressValue(Address(utUserAddr)));
                sinkContainer.Add(sinkHelper.Install(utUsers.Get(i)));
            
            onOffHelper.SetAttribute("Remote", AddressValue(Address(utUserAddr)));
            auto app = onOffHelper.Install(utUsers.Get(0)).Get(0);
            app->SetStartTime(Seconds(1));
            onOffContainer.Add(app);
    }
    sinkContainer.Start(Seconds(0));
    sinkContainer.Stop(Seconds(90));

    // prompt info of the used parameters
    std::cout << "--- sat-onoff-example ---" << std::endl;
    std::cout <<  " Scenario used: " << scenario<< std::endl;
    std::cout <<   "Sender: " << sender<< std::endl;
    std::cout << "  PacketSize: " << packetSize<< std::endl;
    std::cout << "  DataRate: " << dataRate<< std::endl;
    std::cout << "  OnTime: " << onTime<< std::endl;
    std::cout << "  OffTime: " << offTime<< std::endl;
    std::cout << "  Duration: " << simDuration<< std::endl;


    // run simulation and finally destroy it
    simulationHelper->RunSimulation();

    return 0;
}
