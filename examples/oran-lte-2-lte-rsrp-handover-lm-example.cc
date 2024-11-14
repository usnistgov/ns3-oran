/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#include <ns3/core-module.h>
#include <ns3/internet-module.h>
#include <ns3/lte-module.h>
#include <ns3/mobility-module.h>
#include <ns3/network-module.h>
#include <ns3/oran-module.h>
#include <ns3/config-store.h>

#include <fstream>
#include <iostream>
#include <string>
#include <stdio.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("NewOranHandoverUsingRSRPlm");

/**
 * Example of the ORAN models.
 *
 * The scenario consists of an LTE UE moving back and forth
 * between 2 LTE eNBs. The LTE UE reports to the RIC its location
 * and current Cell ID. In the RIC, an LM will periodically check
 * the RSRP and RSRQ of UE, and if needed, issue a handover command.
 *
 * This example demonstrates how to configure processing delays for the LMs.
 */

// Tracing rsrp, rsrq, and sinr
void LogRsrpRsrqSinr(Ptr<OutputStreamWrapper> stream, uint16_t rnti, uint16_t cellId, double rsrp, double rsrq, uint8_t sinr) {
    *stream->GetStream() << Simulator::Now().GetSeconds() << "\tRNTI: " << rnti
                         << "\tCell ID: " << cellId
                         << "\tRSRP: " << rsrp << " dBm"
                         << "\tRSRQ: " << rsrq << " dB"
                         << "\tSINR: " << static_cast<int>(sinr) << " dB" << std::endl;
}
 
// Callback function to log positions
void LogPosition(Ptr<OutputStreamWrapper> stream, Ptr<Node> node,  Ptr<const MobilityModel> mobility) {
    Vector pos = mobility->GetPosition();
    *stream->GetStream() << Simulator::Now().GetSeconds() << "\t" << node->GetId()
                         << "\t" << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
}

void
NotifyHandoverEndOkEnb(uint64_t imsi, uint16_t cellid, uint16_t rnti)
{
    std::cout << Simulator::Now().As(Time::S) << " eNB CellId " << cellid
              << ": completed handover of UE with IMSI " << imsi << " RNTI " << rnti << std::endl;
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

int
main(int argc, char* argv[])
{
    uint16_t numberOfUes = 1;
    uint16_t numberOfEnbs = 2;
    Time simTime = Seconds(100);
    Time maxWaitTime = Seconds(0.010); 
    std::string processingDelayRv = "ns3::NormalRandomVariable[Mean=0.005|Variance=0.000031]";
    double distance = 50; // distance between eNBs
    Time interval = Seconds(15);
    double speed = 1.5; // speed of the ue
    bool dbLog = false;
    Time lmQueryInterval = Seconds(5);
    std::string dbFileName = "oran-repository.db";
    std::string lateCommandPolicy = "DROP";

    // Command line arguments
    CommandLine cmd(__FILE__);
    cmd.AddValue("db-log", "Enable printing SQL queries results", dbLog);
    cmd.AddValue("max-wait-time", "The maximum amount of time an LM has to run", maxWaitTime);
    cmd.AddValue("processing-delay-rv",
                 "The random variable that represents the LMs processing delay",
                 processingDelayRv);
    cmd.AddValue("lm-query-interval",
                 "The interval at which to query the LM for commands",
                 lmQueryInterval);
    cmd.AddValue("late-command-policy",
                 "The policy to use for handling commands received after the maximum wait time "
                 "(\"DROP\" or \"SAVE\")",
                 lateCommandPolicy);
    cmd.AddValue("sim-time", "The amount of time to simulate", simTime);
    cmd.Parse(argc, argv);

    LogComponentEnable("OranNearRtRic", (LogLevel)(LOG_PREFIX_TIME | LOG_WARN));

    Config::SetDefault("ns3::LteHelper::UseIdealRrc", BooleanValue(false));

    /*--- lte and epc helper ---*/
    Ptr<LteHelper> lteHelper = CreateObject<LteHelper>(); // create lteHelper
    Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>(); // create epcHelper
    lteHelper->SetEpcHelper(epcHelper); // connect lte to the evolved packet core, which is the core network
    lteHelper->SetSchedulerType("ns3::RrFfMacScheduler"); // Round-robin Frequency-first Mac Scheduler for resource distribution
    lteHelper->SetHandoverAlgorithmType("ns3::NoOpHandoverAlgorithm"); // disable automatic handover

    // Getting the PGW node; it acts as a gateway between LTE and external network, such as- internet.
    Ptr<Node> pgw = epcHelper->GetPgwNode(); // PGW: Packet Data Network Gateway

    
    /*---- Creating RAN nodes using NodeContainer ----*/
    NodeContainer ueNodes; 
    NodeContainer enbNodes;
    enbNodes.Create(numberOfEnbs);
    ueNodes.Create(numberOfUes);

    // Install Mobility Model
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    for (uint16_t i = 0; i < numberOfEnbs; i++)
    {
        positionAlloc->Add(Vector(distance * i, 0, 20));
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
    NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice(ueNodes);
    
    // Setting TxPower to 30dBm for all eNbs 
    // Assuming enbLteDevs is your NetDeviceContainer for eNodeBs
    for (NetDeviceContainer::Iterator it = enbLteDevs.Begin(); it != enbLteDevs.End(); ++it) {
        Ptr<NetDevice> device = *it;
        Ptr<LteEnbNetDevice> enbLteDevice = device->GetObject<LteEnbNetDevice>();
        if (enbLteDevice) {
            Ptr<LteEnbPhy> enbPhy = enbLteDevice->GetPhy();
            enbPhy->SetTxPower(25); // Set the transmission power to 30 dBm
        }
    }
    
    // Install the IP stack on the UEs
    InternetStackHelper internet;
    internet.Install(ueNodes);
    Ipv4InterfaceContainer ueIpIfaces;
    ueIpIfaces = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs));

    // Attach all UEs to the first eNodeB
    for (uint16_t i = 0; i < numberOfUes; i++)
    {
        lteHelper->Attach(ueLteDevs.Get(i), enbLteDevs.Get(0));
    }

    // Add X2 interface
    lteHelper->AddX2Interface(enbNodes);

    // ORAN Models -- BEGIN
    Ptr<OranNearRtRic> nearRtRic = nullptr;
    OranE2NodeTerminatorContainer e2NodeTerminatorsEnbs;
    OranE2NodeTerminatorContainer e2NodeTerminatorsUes;
    Ptr<OranHelper> oranHelper = CreateObject<OranHelper>();

    oranHelper->SetAttribute("Verbose", BooleanValue(true));
    oranHelper->SetAttribute("LmQueryInterval", TimeValue(lmQueryInterval));
    oranHelper->SetAttribute("E2NodeInactivityThreshold", TimeValue(Seconds(2)));
    oranHelper->SetAttribute("E2NodeInactivityIntervalRv",
                             StringValue("ns3::ConstantRandomVariable[Constant=2]"));
    oranHelper->SetAttribute("LmQueryMaxWaitTime",
                             TimeValue(maxWaitTime)); // 0 means wait for all LMs to finish
    oranHelper->SetAttribute("LmQueryLateCommandPolicy", StringValue(lateCommandPolicy));

    // RIC setup
    if (!dbFileName.empty())
    {
        std::remove(dbFileName.c_str());
    }

    oranHelper->SetDataRepository("ns3::OranDataRepositorySqlite",
                                  "DatabaseFile",
                                  StringValue(dbFileName));
    oranHelper->SetDefaultLogicModule("ns3::OranLmLte2LteRsrpHandover",
                                      "ProcessingDelayRv",
                                      StringValue(processingDelayRv));
    oranHelper->SetConflictMitigationModule("ns3::OranCmmNoop");

    nearRtRic = oranHelper->CreateNearRtRic();

    // UE Nodes setup
    for (uint32_t idx = 0; idx < ueNodes.GetN(); idx++)
    {
        Ptr<OranReporterLocation> locationReporter = CreateObject<OranReporterLocation>();
        Ptr<OranReporterLteUeCellInfo> lteUeCellInfoReporter =
            CreateObject<OranReporterLteUeCellInfo>();
        Ptr<OranReporterLteUeRsrpRsrq> rsrpRsrqReporter = CreateObject<OranReporterLteUeRsrpRsrq>();
        Ptr<OranE2NodeTerminatorLteUe> lteUeTerminator =
            CreateObject<OranE2NodeTerminatorLteUe>();

        locationReporter->SetAttribute("Terminator", PointerValue(lteUeTerminator));

        lteUeCellInfoReporter->SetAttribute("Terminator", PointerValue(lteUeTerminator));

        rsrpRsrqReporter->SetAttribute("Terminator", PointerValue(lteUeTerminator));

        for (uint32_t netDevIdx = 0; netDevIdx < ueNodes.Get(idx)->GetNDevices(); netDevIdx++)
        {
            Ptr<LteUeNetDevice> lteUeDevice = ueNodes.Get(idx)->GetDevice(netDevIdx)->GetObject<LteUeNetDevice>();
            if (lteUeDevice)
            {
                Ptr<LteUePhy> uePhy = lteUeDevice->GetPhy();
                uePhy->TraceConnectWithoutContext("ReportUeMeasurements", MakeCallback(&ns3::OranReporterLteUeRsrpRsrq::ReportRsrpRsrq, rsrpRsrqReporter));
            }
        }

        lteUeTerminator->SetAttribute("NearRtRic", PointerValue(nearRtRic));
        lteUeTerminator->SetAttribute("RegistrationIntervalRv",
                                      StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        lteUeTerminator->SetAttribute("SendIntervalRv",
                                      StringValue("ns3::ConstantRandomVariable[Constant=1]"));

        lteUeTerminator->AddReporter(locationReporter);
        lteUeTerminator->AddReporter(lteUeCellInfoReporter);
        lteUeTerminator->AddReporter(rsrpRsrqReporter);

        lteUeTerminator->Attach(ueNodes.Get(idx));

        Simulator::Schedule(Seconds(1), &OranE2NodeTerminatorLteUe::Activate, lteUeTerminator);
    }
    
    // ENb Nodes setup
    oranHelper->SetE2NodeTerminator("ns3::OranE2NodeTerminatorLteEnb",
                                    "RegistrationIntervalRv",
                                    StringValue("ns3::ConstantRandomVariable[Constant=1]"),
                                    "SendIntervalRv",
                                    StringValue("ns3::ConstantRandomVariable[Constant=1]"));

    oranHelper->AddReporter("ns3::OranReporterLocation",
                            "Trigger",
                            StringValue("ns3::OranReportTriggerPeriodic"));

    e2NodeTerminatorsEnbs.Add(oranHelper->DeployTerminators(nearRtRic, enbNodes));

    // DB logging to the terminal
    if (dbLog)
    {
        nearRtRic->Data()->TraceConnectWithoutContext("QueryRc", MakeCallback(&QueryRcSink));
    }

    // Activate and the components
    Simulator::Schedule(Seconds(1),
                        &OranHelper::ActivateAndStartNearRtRic,
                        oranHelper,
                        nearRtRic);
    Simulator::Schedule(Seconds(1.5),
                        &OranHelper::ActivateE2NodeTerminators,
                        oranHelper,
                        e2NodeTerminatorsEnbs);
    Simulator::Schedule(Seconds(2),
                        &OranHelper::ActivateE2NodeTerminators,
                        oranHelper,
                        e2NodeTerminatorsUes);
    // ORAN Models -- END

    // Trace the end of handovers
    Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverEndOk",
                                  MakeCallback(&NotifyHandoverEndOkEnb));
    
    // Assuming 'ueNodes' and 'enbNodes' are your NodeContainers
    Ptr<OutputStreamWrapper> mobilityTrace = Create<OutputStreamWrapper>("MobilityTrace.tr", std::ios::out);
    for (uint32_t i = 0; i < ueNodes.GetN(); ++i)
    {
        Ptr<MobilityModel> mob = ueNodes.Get(i)->GetObject<MobilityModel>();
        mob->TraceConnectWithoutContext("CourseChange", MakeBoundCallback(&LogPosition, mobilityTrace, ueNodes.Get(i)));
    }
    for (uint32_t i = 0; i < enbNodes.GetN(); ++i)
    {
        Ptr<MobilityModel> mob = enbNodes.Get(i)->GetObject<MobilityModel>();
        mob->TraceConnectWithoutContext("CourseChange", MakeBoundCallback(&LogPosition, mobilityTrace, ueNodes.Get(i)));
    }
     
    // Tracing rsrp, rsrq, and sinr while setting up uePhy
    Ptr<OutputStreamWrapper> rsrpSinrTrace = Create<OutputStreamWrapper>("RsrpRsrqSinrTrace.tr", std::ios::out);
    for (NetDeviceContainer::Iterator it = ueLteDevs.Begin(); it != ueLteDevs.End(); ++it)
    {
        Ptr<NetDevice> device = *it;
        Ptr<LteUeNetDevice> lteUeDevice = device->GetObject<LteUeNetDevice>();
        if (lteUeDevice)
        {
            Ptr<LteUePhy> uePhy = lteUeDevice->GetPhy();
            uePhy->TraceConnectWithoutContext("ReportCurrentCellRsrpSinr", MakeBoundCallback(&LogRsrpRsrqSinr, rsrpSinrTrace));
        }
    }
    
    /* Enabling Tracing for the simulation scenario */
    lteHelper->EnablePhyTraces();
    lteHelper->EnableMacTraces();
    lteHelper->EnableRlcTraces();
    lteHelper->EnablePdcpTraces();

    Simulator::Stop(simTime);
    Simulator::Run();

    Simulator::Destroy();
    
    return 0;
}
