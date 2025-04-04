/**
 * NIST-developed software is provided by NIST as a public service. You may
 * use, copy and distribute copies of the software in any medium, provided that
 * you keep intact this entire notice. You may improve, modify and create
 * derivative works of the software or any portion of the software, and you may
 * copy and distribute such modifications or works. Modified works should carry
 * a notice stating that you changed the software and should note the date and
 * nature of any such change. Please explicitly acknowledge the National
 * Institute of Standards and Technology as the source of the software.
 *
 * NIST-developed software is expressly provided "AS IS." NIST MAKES NO
 * WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF
 * LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST
 * NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE
 * UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST
 * DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE
 * SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE
 * CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
 *
 * You are solely responsible for determining the appropriateness of using and
 * distributing the software and you assume all risks associated with its use,
 * including but not limited to the risks and costs of program errors,
 * compliance with applicable laws, damage to or loss of data, programs or
 * equipment, and the unavailability or interruption of operation. This
 * software is not intended to be used in any situation where a failure could
 * cause risk of injury or damage to property. The software developed by NIST
 * employees is not subject to copyright protection within the United States.
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/oran-module.h"
#include "ns3/point-to-point-module.h"

#include <stdio.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("OranLte2LteDistanceHandoverHelperExample");

/**
 * Usage example of the ORAN models, configured with the ORAN Helper.
 *
 * The scenario consists of an LTE UE moving back and forth
 * between 2 LTE eNBs. The LTE UE reports to the RIC its location
 * and current Cell ID. In the RIC, an LM will periodically check
 * the position, and if needed, issue a handover command.
 */

void
TraceEnbRx(std::string context, uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t delay)
{
    std::cout << Simulator::Now().GetSeconds() << " s: " << context << " recieved " << bytes
              << " bytes from RNTI " << (uint32_t)rnti << std::endl;
}

void
NotifyHandoverEndOkEnb(std::string context, uint64_t imsi, uint16_t cellid, uint16_t rnti)
{
    std::cout << Simulator::Now().GetSeconds() << " s:" << context << " eNB CellId " << cellid
              << ": completed handover of UE with IMSI " << imsi << " RNTI " << rnti << std::endl;

    Config::Disconnect("NodeList/*/DeviceList/*/$ns3::LteNetDevice/$ns3::LteEnbNetDevice/LteEnbRrc/"
                       "UeMap/*/DataRadioBearerMap/*/LteRlc/RxPDU",
                       MakeCallback(&TraceEnbRx));

    Config::Connect("NodeList/*/DeviceList/*/$ns3::LteNetDevice/$ns3::LteEnbNetDevice/LteEnbRrc/"
                    "UeMap/*/DataRadioBearerMap/*/LteRlc/RxPDU",
                    MakeCallback(&TraceEnbRx));
}

void
RxTrace(std::string context, Ptr<const Packet> packet, const Address& address)
{
    InetSocketAddress isa = InetSocketAddress::ConvertFrom(address);

    std::cout << Simulator::Now().GetSeconds() << " s: " << context << " recieved "
              << packet->GetSize() << " bytes from " << isa.GetIpv4() << " on port "
              << (uint32_t)isa.GetPort() << std::endl;
}

void
QueryRcSink(std::string query, std::string args, int rc)
{
    std::cout << Simulator::Now().GetSeconds() << " Query "
              << ((rc == SQLITE_OK || rc == SQLITE_DONE) ? "OK" : "ERROR") << "(" << rc << "): \""
              << query << "\"";

    if (!args.empty())
    {
        std::cout << " (" << args << ")";
    }
    std::cout << std::endl;
}

void
ReverseVelocity(NodeContainer nodes, Time interval)
{
    for (uint32_t idx = 0; idx < nodes.GetN(); idx++)
    {
        Ptr<ConstantVelocityMobilityModel> mobility =
            nodes.Get(idx)->GetObject<ConstantVelocityMobilityModel>();
        mobility->SetVelocity(Vector(mobility->GetVelocity().x * -1, 0, 0));
    }

    Simulator::Schedule(interval, &ReverseVelocity, nodes, interval);
}

/**
 * ORAN handover example. Based on the LTE module's "lena-x2-handover.cc".
 */
int
main(int argc, char* argv[])
{
    uint16_t numberOfUes = 1;
    uint16_t numberOfEnbs = 2;
    uint32_t numberOfNetDevs = 2;
    Time simTime = Seconds(50);
    double distance = 50;
    Time interval = Seconds(15);
    double speed = 1.5;
    bool verbose = false;
    bool applicationOutput = false;
    bool enbOutput = true;

    std::string dbFileName = "oran-repository.db";

    // Command line arguments
    CommandLine cmd(__FILE__);
    cmd.AddValue("verbose", "Enable printing SQL queries results", verbose);
    cmd.AddValue("application-output",
                 "Enable printing application traffic information",
                 applicationOutput);
    cmd.AddValue("enb-output", "Enable eNodeB traffic information", enbOutput);
    cmd.Parse(argc, argv);

    Config::SetDefault("ns3::LteHelper::UseIdealRrc", BooleanValue(false));

    Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
    Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
    lteHelper->SetEpcHelper(epcHelper);
    lteHelper->SetSchedulerType("ns3::RrFfMacScheduler");
    lteHelper->SetHandoverAlgorithmType("ns3::NoOpHandoverAlgorithm"); // disable automatic handover

    Ptr<Node> pgw = epcHelper->GetPgwNode();

    // Create remote hosts
    NodeContainer remoteHostsContainer;
    remoteHostsContainer.Create(numberOfNetDevs);
    InternetStackHelper internet;
    internet.Install(remoteHostsContainer);

    // Create the Internet
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase("1.0.0.0", "255.0.0.0");

    Ipv4StaticRoutingHelper ipv4RoutingHelper;

    for (uint16_t i = 0; i < remoteHostsContainer.GetN(); i++)
    {
        // Create the P2P links between P-GW and servers
        PointToPointHelper p2ph;
        p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
        p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
        p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.010)));

        NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHostsContainer.Get(i));
        // Assign IPv6 addresses
        Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);
        // Define route from server to UEs
        Ptr<Node> remoteHost = remoteHostsContainer.Get(i);
        Ipv4Address remoteHostAddr = remoteHost->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
        Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
            ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
        // interface 0 is localhost, 1 is the p2p device
        remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"),
                                                   Ipv4Mask("255.0.0.0"),
                                                   1);
        // Define route from P-GW to the server
        Ptr<Ipv4StaticRouting> pgwStaticRouting =
            ipv4RoutingHelper.GetStaticRouting(pgw->GetObject<Ipv4>());
        pgwStaticRouting->AddHostRouteTo(remoteHostAddr, internetDevices.Get(0)->GetIfIndex());
    }

    NodeContainer ueNodes;
    NodeContainer enbNodes;
    enbNodes.Create(numberOfEnbs);
    ueNodes.Create(numberOfUes);

    // Install Mobility Model
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    for (uint16_t i = 0; i < numberOfEnbs; i++)
    {
        positionAlloc->Add(Vector(distance * i, 0.0, 20.0));
    }

    for (uint16_t i = 0; i < numberOfUes; i++)
    {
        // Coordinates of the middle point between the eNBs, minus the distance covered
        // in half of the interval for switching directions
        positionAlloc->Add(Vector((distance / 2) - (speed * (interval.GetSeconds() / 2)), 0, 1.5));
    }

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator(positionAlloc);
    mobility.Install(enbNodes);

    mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobility.Install(ueNodes);

    for (uint32_t idx = 0; idx < ueNodes.GetN(); idx++)
    {
        Ptr<ConstantVelocityMobilityModel> mobility =
            ueNodes.Get(idx)->GetObject<ConstantVelocityMobilityModel>();
        mobility->SetVelocity(Vector(speed, 0, 0));
    }

    // Schedule the first direction switch
    Simulator::Schedule(interval, &ReverseVelocity, ueNodes, interval);

    // Install LTE Devices in eNB and UEs
    NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice(enbNodes);

    NetDeviceContainer ueLteDevs;
    for (uint32_t i = 0; i < ueNodes.GetN(); i++)
    {
        for (uint32_t j = 0; j < numberOfNetDevs; j++)
        {
            ueLteDevs.Add(lteHelper->InstallUeDevice(ueNodes.Get(i)));
        }
    }
    // Install the IP stack on the UEs
    internet.Install(ueNodes);
    Ipv4InterfaceContainer ueIpIfaces;
    ueIpIfaces = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs));

    // Attach all UEs to the first eNodeB
    for (uint16_t i = 0; i < ueLteDevs.GetN(); i++)
    {
        lteHelper->Attach(ueLteDevs.Get(i), enbLteDevs.Get(0));
    }

    // Install and start applications on UEs and remote host
    uint16_t dlPort = 10000;
    uint16_t ulPort = 20000;

    // randomize a bit start times to avoid simulation artifacts
    // (e.g., buffer overflows due to packet transmissions happening
    // exactly at the same time)
    Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable>();
    startTimeSeconds->SetAttribute("Min", DoubleValue(0.05));
    startTimeSeconds->SetAttribute("Max", DoubleValue(0.06));

    ApplicationContainer clientApps;
    ApplicationContainer serverApps;

    for (uint32_t u = 0; u < ueNodes.GetN(); ++u)
    {
        Ptr<Node> ue = ueNodes.Get(u);
        Ptr<Ipv4StaticRouting> ueStaticRouting =
            ipv4RoutingHelper.GetStaticRouting(ue->GetObject<Ipv4>());

        for (uint32_t netDevIdx = 1; netDevIdx < ue->GetNDevices(); netDevIdx++)
        {
            Ptr<Node> remoteHost = remoteHostsContainer.Get(netDevIdx - 1);
            Ipv4Address remoteHostAddr = remoteHost->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
            Ipv4Address ueAddress = ue->GetObject<Ipv4>()->GetAddress(netDevIdx, 0).GetLocal();

            ueStaticRouting->AddHostRouteTo(remoteHostAddr,
                                            epcHelper->GetUeDefaultGatewayAddress(),
                                            netDevIdx);

            dlPort++;

            OnOffHelper dlClientHelper("ns3::UdpSocketFactory",
                                       InetSocketAddress(ueAddress, dlPort));
            dlClientHelper.SetAttribute("Local",
                                        AddressValue(InetSocketAddress(remoteHostAddr, dlPort)));
            clientApps.Add(dlClientHelper.Install(remoteHost));

            PacketSinkHelper dlPacketSinkHelper("ns3::UdpSocketFactory",
                                                InetSocketAddress(Ipv4Address::GetAny(), dlPort));
            serverApps.Add(dlPacketSinkHelper.Install(ue));

            ulPort++;

            OnOffHelper ulClientHelper("ns3::UdpSocketFactory",
                                       InetSocketAddress(remoteHostAddr, ulPort));
            ulClientHelper.SetAttribute("Local",
                                        AddressValue(InetSocketAddress(ueAddress, ulPort)));
            clientApps.Add(ulClientHelper.Install(ue));

            PacketSinkHelper ulPacketSinkHelper("ns3::UdpSocketFactory",
                                                InetSocketAddress(Ipv4Address::GetAny(), ulPort));
            serverApps.Add(ulPacketSinkHelper.Install(remoteHost));
        }
    }

    Time startTime = Seconds(startTimeSeconds->GetValue());
    serverApps.Start(startTime);
    clientApps.Start(startTime);
    clientApps.Stop(simTime);

    // Add X2 interface
    lteHelper->AddX2Interface(enbNodes);

    // ORAN Models -- BEGIN
    Ptr<OranNearRtRic> nearRtRic = nullptr;
    OranE2NodeTerminatorContainer e2NodeTerminatorsEnbs;
    OranE2NodeTerminatorContainer e2NodeTerminatorsUes;
    Ptr<OranHelper> oranHelper = CreateObject<OranHelper>();

    oranHelper->SetAttribute("Verbose", BooleanValue(true));
    oranHelper->SetAttribute("LmQueryInterval", TimeValue(Seconds(5)));
    oranHelper->SetAttribute("E2NodeInactivityThreshold", TimeValue(Seconds(2)));
    oranHelper->SetAttribute("E2NodeInactivityIntervalRv",
                             StringValue("ns3::ConstantRandomVariable[Constant=2]"));
    oranHelper->SetAttribute("LmQueryMaxWaitTime",
                             TimeValue(Seconds(0))); // 0 means wait for all LMs to finish
    oranHelper->SetAttribute("LmQueryLateCommandPolicy", EnumValue(OranNearRtRic::DROP));
    oranHelper->SetAttribute("RicTransmissionDelayRv",
                             StringValue("ns3::ConstantRandomVariable[Constant=0.001]"));

    // RIC setup
    if (!dbFileName.empty())
    {
        std::remove(dbFileName.c_str());
    }

    oranHelper->SetDataRepository("ns3::OranDataRepositorySqlite",
                                  "DatabaseFile",
                                  StringValue(dbFileName));
    oranHelper->SetDefaultLogicModule("ns3::OranLmLte2LteDistanceHandover",
                                      "ProcessingDelayRv",
                                      StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    oranHelper->SetConflictMitigationModule("ns3::OranCmmNoop");

    nearRtRic = oranHelper->CreateNearRtRic();

    // UE Nodes setup
    for (uint32_t netDevIdx = 0; netDevIdx < numberOfNetDevs; netDevIdx++)
    {
        oranHelper->SetE2NodeTerminator("ns3::OranE2NodeTerminatorLteUe",
                                        "RegistrationIntervalRv",
                                        StringValue("ns3::ConstantRandomVariable[Constant=1]"),
                                        "SendIntervalRv",
                                        StringValue("ns3::ConstantRandomVariable[Constant=1]"),
                                        "TransmissionDelayRv",
                                        StringValue("ns3::ConstantRandomVariable[Constant=0.001]"));

        if (netDevIdx == 0)
        {
            oranHelper->AddReporter("ns3::OranReporterLocation",
                                    "Trigger",
                                    StringValue("ns3::OranReportTriggerPeriodic"));
        }

        oranHelper->AddReporter(
            "ns3::OranReporterLteUeCellInfo",
            "Trigger",
            StringValue("ns3::OranReportTriggerLteUeHandover[InitialReport=true]"));

        e2NodeTerminatorsUes.Add(oranHelper->DeployTerminators(nearRtRic, ueNodes, netDevIdx));
    }

    // ENb Nodes setup
    oranHelper->SetE2NodeTerminator("ns3::OranE2NodeTerminatorLteEnb",
                                    "RegistrationIntervalRv",
                                    StringValue("ns3::ConstantRandomVariable[Constant=1]"),
                                    "SendIntervalRv",
                                    StringValue("ns3::ConstantRandomVariable[Constant=1]"),
                                    "TransmissionDelayRv",
                                    StringValue("ns3::ConstantRandomVariable[Constant=0.001]"));

    oranHelper->AddReporter("ns3::OranReporterLocation",
                            "Trigger",
                            StringValue("ns3::OranReportTriggerPeriodic"));

    e2NodeTerminatorsEnbs.Add(oranHelper->DeployTerminators(nearRtRic, enbNodes));

    // DB logging to the terminal
    if (verbose)
    {
        nearRtRic->Data()->TraceConnectWithoutContext("QueryRc", MakeCallback(&QueryRcSink));
    }

    // Activate and the components
    Simulator::Schedule(Seconds(1), &OranHelper::ActivateAndStartNearRtRic, oranHelper, nearRtRic);
    Simulator::Schedule(Seconds(1.5),
                        &OranHelper::ActivateE2NodeTerminators,
                        oranHelper,
                        e2NodeTerminatorsEnbs);
    Simulator::Schedule(Seconds(2),
                        &OranHelper::ActivateE2NodeTerminators,
                        oranHelper,
                        e2NodeTerminatorsUes);
    // ORAN Models -- END

    if (applicationOutput)
    {
        // Trace the reception of packets at the application layer
        Config::Connect("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",
                        MakeCallback(&RxTrace));
    }

    if (enbOutput)
    {
        // Trace the end of handovers
        Config::Connect("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverEndOk",
                        MakeCallback(&NotifyHandoverEndOkEnb));

        // Trace the receiption of PDUs at the eNB
        Simulator::Schedule(Seconds(0.5),
                            &Config::Connect,
                            "NodeList/*/DeviceList/*/$ns3::LteNetDevice/$ns3::LteEnbNetDevice/"
                            "LteEnbRrc/UeMap/*/DataRadioBearerMap/*/LteRlc/RxPDU",
                            MakeCallback(&TraceEnbRx));
    }

    Simulator::Stop(simTime);
    Simulator::Run();

    Simulator::Destroy();
    return 0;
}
