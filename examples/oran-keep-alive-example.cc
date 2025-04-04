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

NS_LOG_COMPONENT_DEFINE("OranKeepAliveExample");

/**
 * Example to show how the Keep-Alive feature of the ORAN models works.
 * The E2 Node is set to send a Registration (aka Keep-Alive) message
 * every 2 to 3 seconds. However, the RIC will consider any E2 Node that
 * hasn't updated its registration in the last second as Deregistered.
 *
 * We can see the effects on the Database (multiple location entries will
 * be missing), and activating the WARN log in the OranE2NodeTerminator,
 * which will report each registration interval that is greater than the
 * RIC's threshold
 */

int
main(int argc, char* argv[])
{
    uint16_t numberOfNodes = 1;
    Time simTime = Seconds(105);
    bool verbose = false;
    std::string dbFileName = "oran-repository.db";

    // Command line arguments
    CommandLine cmd(__FILE__);
    cmd.AddValue("duration", "The length of the simulation.", simTime);
    cmd.AddValue("verbose", "Enable printing node location.", verbose);
    cmd.Parse(argc, argv);

    // Create nodes.
    NodeContainer nodes;
    nodes.Create(numberOfNodes);

    // Setup mobility model.
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(0, 0, 0));
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobility.SetPositionAllocator(positionAlloc);
    mobility.Install(nodes);
    nodes.Get(0)->GetObject<ConstantVelocityMobilityModel>()->SetVelocity(Vector(1, 0, 0));

    // Deploy ORAN
    Ptr<OranNearRtRic> nearRtRic = nullptr;
    OranE2NodeTerminatorContainer e2NodeTerminators;
    Ptr<OranHelper> oranHelper = CreateObject<OranHelper>();
    oranHelper->SetAttribute("Verbose", BooleanValue(true));
    oranHelper->SetAttribute("E2NodeInactivityThreshold", TimeValue(Seconds(1)));
    oranHelper->SetAttribute("E2NodeInactivityIntervalRv",
                             StringValue("ns3::ConstantRandomVariable[Constant=0.5]"));

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
                                    StringValue("ns3::UniformRandomVariable[Min=2|Max=3]"),
                                    "SendIntervalRv",
                                    StringValue("ns3::ConstantRandomVariable[Constant=2]"));

    oranHelper->AddReporter("ns3::OranReporterLocation",
                            "Trigger",
                            StringValue("ns3::OranReportTriggerPeriodic"));

    e2NodeTerminators.Add(oranHelper->DeployTerminators(nearRtRic, nodes));

    // Activate and the components
    Simulator::Schedule(Seconds(1), &OranHelper::ActivateAndStartNearRtRic, oranHelper, nearRtRic);
    Simulator::Schedule(Seconds(5),
                        &OranHelper::ActivateE2NodeTerminators,
                        oranHelper,
                        e2NodeTerminators);

    // Run the simulation.
    Simulator::Stop(simTime);
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
