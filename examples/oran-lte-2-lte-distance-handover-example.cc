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

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/oran-module.h"

#include <stdio.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("OranLte2LteDistanceHandoverExample");

/**
 * Usage example of the ORAN models.
 *
 * The scenario consists of an LTE UE moving back and forth
 * between 2 LTE eNBs. The LTE UE reports to the RIC its location
 * and current Cell ID. In the RIC, an LM will periodically check
 * the position, and if needed, issue a handover command.
 */

void
NotifyHandoverEndOkEnb(std::string context, uint64_t imsi, uint16_t cellid, uint16_t rnti)
{
    std::cout << Simulator::Now().GetSeconds() << " " << context << " eNB CellId " << cellid
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

/**
 * ORAN handover example. Based on the LTE module's "lena-x2-handover.cc".
 */
int
main(int argc, char* argv[])
{
    uint16_t numberOfUes = 1;
    uint16_t numberOfEnbs = 2;
    Time simTime = Seconds(50);
    double distance = 50;
    Time interval = Seconds(15);
    double speed = 1.5;
    bool verbose = false;
    std::string dbFileName = "oran-repository.db";

    // Command line arguments
    CommandLine cmd(__FILE__);
    cmd.AddValue("verbose", "Enable printing SQL queries results", verbose);
    cmd.Parse(argc, argv);

    Config::SetDefault("ns3::LteHelper::UseIdealRrc", BooleanValue(false));

    Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
    Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
    lteHelper->SetEpcHelper(epcHelper);
    lteHelper->SetSchedulerType("ns3::RrFfMacScheduler");
    lteHelper->SetHandoverAlgorithmType("ns3::NoOpHandoverAlgorithm"); // disable automatic handover

    Ptr<Node> pgw = epcHelper->GetPgwNode();

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
    if (!dbFileName.empty())
    {
        std::remove(dbFileName.c_str());
    }
    Ptr<OranDataRepository> dataRepository = CreateObject<OranDataRepositorySqlite>();
    Ptr<OranLm> defaultLm = CreateObject<OranLmLte2LteDistanceHandover>();
    Ptr<OranCmm> cmm = CreateObject<OranCmmNoop>();
    Ptr<OranNearRtRic> nearRtRic = CreateObject<OranNearRtRic>();
    Ptr<OranNearRtRicE2Terminator> nearRtRicE2Terminator =
        CreateObject<OranNearRtRicE2Terminator>();

    dataRepository->SetAttribute("DatabaseFile", StringValue(dbFileName));
    if (verbose)
    {
        dataRepository->TraceConnectWithoutContext("QueryRc", MakeCallback(&QueryRcSink));
    }

    defaultLm->SetAttribute("Verbose", BooleanValue(true));
    defaultLm->SetAttribute("NearRtRic", PointerValue(nearRtRic));
    defaultLm->SetAttribute("ProcessingDelayRv",
                            StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    cmm->SetAttribute("NearRtRic", PointerValue(nearRtRic));
    cmm->SetAttribute("Verbose", BooleanValue(true));

    nearRtRicE2Terminator->SetAttribute("NearRtRic", PointerValue(nearRtRic));
    nearRtRicE2Terminator->SetAttribute("DataRepository", PointerValue(dataRepository));
    nearRtRicE2Terminator->SetAttribute("TransmissionDelayRv",
                                        StringValue("ns3::ConstantRandomVariable[Constant=0.001]"));

    nearRtRic->SetAttribute("DefaultLogicModule", PointerValue(defaultLm));
    nearRtRic->SetAttribute("E2Terminator", PointerValue(nearRtRicE2Terminator));
    nearRtRic->SetAttribute("DataRepository", PointerValue(dataRepository));
    nearRtRic->SetAttribute("LmQueryInterval", TimeValue(Seconds(5)));
    nearRtRic->SetAttribute("ConflictMitigationModule", PointerValue(cmm));
    nearRtRic->SetAttribute("E2NodeInactivityThreshold", TimeValue(Seconds(2)));
    nearRtRic->SetAttribute("E2NodeInactivityIntervalRv",
                            StringValue("ns3::ConstantRandomVariable[Constant=2]"));
    nearRtRic->SetAttribute("LmQueryMaxWaitTime",
                            TimeValue(Seconds(0))); // 0 means wait for all LMs to finish
    nearRtRic->SetAttribute("LmQueryLateCommandPolicy", EnumValue(OranNearRtRic::DROP));

    Simulator::Schedule(Seconds(1), &OranNearRtRic::Start, nearRtRic);

    for (uint32_t idx = 0; idx < ueNodes.GetN(); idx++)
    {
        Ptr<OranReporterLocation> locationReporter = CreateObject<OranReporterLocation>();
        Ptr<OranReporterLteUeCellInfo> lteUeCellInfoReporter =
            CreateObject<OranReporterLteUeCellInfo>();
        Ptr<OranE2NodeTerminatorLteUe> lteUeTerminator = CreateObject<OranE2NodeTerminatorLteUe>();

        locationReporter->SetAttribute("Terminator", PointerValue(lteUeTerminator));
        locationReporter->SetAttribute("Trigger", StringValue("ns3::OranReportTriggerPeriodic"));

        lteUeCellInfoReporter->SetAttribute("Terminator", PointerValue(lteUeTerminator));
        lteUeCellInfoReporter->SetAttribute(
            "Trigger",
            StringValue("ns3::OranReportTriggerLteUeHandover[InitialReport=true]"));

        lteUeTerminator->SetAttribute("NearRtRic", PointerValue(nearRtRic));
        lteUeTerminator->SetAttribute("RegistrationIntervalRv",
                                      StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        lteUeTerminator->SetAttribute("SendIntervalRv",
                                      StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        lteUeTerminator->SetAttribute("TransmissionDelayRv",
                                      StringValue("ns3::ConstantRandomVariable[Constant=0.001]"));

        lteUeTerminator->AddReporter(locationReporter);
        lteUeTerminator->AddReporter(lteUeCellInfoReporter);

        lteUeTerminator->Attach(ueNodes.Get(idx));

        Simulator::Schedule(Seconds(2), &OranE2NodeTerminatorLteUe::Activate, lteUeTerminator);
    }

    for (uint32_t idx = 0; idx < enbNodes.GetN(); idx++)
    {
        Ptr<OranReporterLocation> locationReporter = CreateObject<OranReporterLocation>();
        Ptr<OranE2NodeTerminatorLteEnb> lteEnbTerminator =
            CreateObject<OranE2NodeTerminatorLteEnb>();

        locationReporter->SetAttribute("Terminator", PointerValue(lteEnbTerminator));
        locationReporter->SetAttribute("Trigger", StringValue("ns3::OranReportTriggerPeriodic"));

        lteEnbTerminator->SetAttribute("NearRtRic", PointerValue(nearRtRic));
        lteEnbTerminator->SetAttribute("RegistrationIntervalRv",
                                       StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        lteEnbTerminator->SetAttribute("SendIntervalRv",
                                       StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        lteEnbTerminator->SetAttribute("TransmissionDelayRv",
                                       StringValue("ns3::ConstantRandomVariable[Constant=0.001]"));

        lteEnbTerminator->AddReporter(locationReporter);

        lteEnbTerminator->Attach(enbNodes.Get(idx));

        Simulator::Schedule(Seconds(1.5), &OranE2NodeTerminatorLteEnb::Activate, lteEnbTerminator);
    }

    // ORAN Models -- END

    // Trace the end of handovers
    Config::Connect("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverEndOk",
                    MakeCallback(&NotifyHandoverEndOkEnb));

    Simulator::Stop(simTime);
    Simulator::Run();

    Simulator::Destroy();
    return 0;
}
