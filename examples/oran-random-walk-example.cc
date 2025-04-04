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
#include "ns3/mobility-module.h"
#include "ns3/oran-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("OranRandomWalkExample");

/**
 * Usage example of the ORAN models.
 *
 * A simple topology of a node moving randomly
 * and reporting its location is used.
 */

void
CourseChange(Ptr<const MobilityModel> mobility)
{
    Vector pos = mobility->GetPosition();
    Vector vel = mobility->GetVelocity();
    std::cout << Simulator::Now() << ", model=" << mobility << ", POS: x=" << pos.x
              << ", y=" << pos.y << ", z=" << pos.z << "; VEL:" << vel.x << ", y=" << vel.y
              << ", z=" << vel.z << std::endl;
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
    uint16_t numberOfNodes = 1;
    Time simTime = Seconds(50);
    bool verbose = false;
    std::string dbFileName = "oran-repository.db";

    // Command line arguments
    CommandLine cmd(__FILE__);
    cmd.AddValue("duration", "The length of the simulation.", simTime);
    cmd.AddValue("nodes", "The number of nodes to consider.", numberOfNodes);
    cmd.AddValue("verbose", "Enable printing node location.", verbose);
    cmd.Parse(argc, argv);

    // Create nodes.
    NodeContainer nodes;
    nodes.Create(numberOfNodes);

    // Setup mobility model.
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
                                  "X",
                                  StringValue("100.0"),
                                  "Y",
                                  StringValue("100.0"),
                                  "Rho",
                                  StringValue("ns3::UniformRandomVariable[Min=0|Max=30]"));
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Mode",
                              StringValue("Time"),
                              "Time",
                              StringValue("2s"),
                              "Speed",
                              StringValue("ns3::ConstantRandomVariable[Constant=1.0]"),
                              "Bounds",
                              StringValue("0|100|0|100"));

    // Deploy mobility model.
    mobility.Install(nodes);

    // Deploy ORAN
    Ptr<OranNearRtRic> nearRtRic = nullptr;
    OranE2NodeTerminatorContainer e2NodeTerminators;
    Ptr<OranHelper> oranHelper = CreateObject<OranHelper>();
    oranHelper->SetAttribute("Verbose", BooleanValue(true));

    // RIC setup
    if (!dbFileName.empty())
    {
        std::remove(dbFileName.c_str());
    }

    oranHelper->SetDataRepository("ns3::OranDataRepositorySqlite",
                                  "DatabaseFile",
                                  StringValue(dbFileName));
    oranHelper->SetDefaultLogicModule("ns3::OranLmNoop");
    oranHelper->SetConflictMitigationModule("ns3::OranCmmNoop");

    nearRtRic = oranHelper->CreateNearRtRic();

    // Terminator nodes setup
    oranHelper->SetE2NodeTerminator("ns3::OranE2NodeTerminatorWired",
                                    "RegistrationIntervalRv",
                                    StringValue("ns3::ConstantRandomVariable[Constant=1]"),
                                    "SendIntervalRv",
                                    StringValue("ns3::ConstantRandomVariable[Constant=1]"));

    oranHelper->AddReporter("ns3::OranReporterLocation",
                            "Trigger",
                            StringValue("ns3::OranReportTriggerPeriodic"));

    e2NodeTerminators.Add(oranHelper->DeployTerminators(nearRtRic, nodes));

    // DB logging to the terminal
    if (verbose)
    {
        nearRtRic->Data()->TraceConnectWithoutContext("QueryRc", MakeCallback(&QueryRcSink));
    }

    // Activate and the components
    Simulator::Schedule(Seconds(1), &OranHelper::ActivateAndStartNearRtRic, oranHelper, nearRtRic);
    Simulator::Schedule(Seconds(2),
                        &OranHelper::ActivateE2NodeTerminators,
                        oranHelper,
                        e2NodeTerminators);

    // Link output callback to mobility model.
    if (verbose)
    {
        Config::ConnectWithoutContext("/NodeList/*/$ns3::MobilityModel/CourseChange",
                                      MakeCallback(&CourseChange));
    }

    // Run the simulation.
    Simulator::Stop(simTime);
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
