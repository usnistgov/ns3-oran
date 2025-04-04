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

// An essential include is test.h
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/oran-module.h"
#include "ns3/test.h"

using namespace ns3;

/**
 * @ingroup oran
 *
 * Class that tests that node location is reported, stored, and retrieved as expected.
 */
class OranTestCaseMobility1 : public TestCase
{
  public:
    /**
     * Constructor of the test
     */
    OranTestCaseMobility1();
    /**
     * Destructor of the test
     */
    virtual ~OranTestCaseMobility1();

  private:
    /**
     * Method that runs the simulation for the test
     */
    virtual void DoRun();
};

OranTestCaseMobility1::OranTestCaseMobility1()
    : TestCase("Oran Test Case Mobility 1")
{
}

OranTestCaseMobility1::~OranTestCaseMobility1()
{
}

void
OranTestCaseMobility1::DoRun()
{
    Time simTime = Seconds(14);
    double speed = 2;
    std::string dbFileName = "oran-repository.db";

    NodeContainer nodes;
    nodes.Create(1);

    // Install Mobility Model
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(0, 0, 0));

    MobilityHelper mobilityHelper;
    mobilityHelper.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobilityHelper.SetPositionAllocator(positionAlloc);
    mobilityHelper.Install(nodes);

    Ptr<ConstantVelocityMobilityModel> mobility =
        nodes.Get(0)->GetObject<ConstantVelocityMobilityModel>();

    // ORAN Models -- BEGIN
    if (!dbFileName.empty())
    {
        std::remove(dbFileName.c_str());
    }

    Ptr<OranNearRtRic> nearRtRic = nullptr;
    OranE2NodeTerminatorContainer e2NodeTerminators;
    Ptr<OranHelper> oranHelper = CreateObject<OranHelper>();

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

    // Activate and the components
    Simulator::Schedule(Seconds(0), &OranHelper::ActivateAndStartNearRtRic, oranHelper, nearRtRic);
    Simulator::Schedule(Seconds(1),
                        &OranHelper::ActivateE2NodeTerminators,
                        oranHelper,
                        e2NodeTerminators);
    Simulator::Schedule(Seconds(2),
                        &ConstantVelocityMobilityModel::SetVelocity,
                        mobility,
                        Vector(speed, speed, 0));
    Simulator::Schedule(Seconds(12),
                        &ConstantVelocityMobilityModel::SetVelocity,
                        mobility,
                        Vector(0, 0, 0));
    // ORAN Models -- END

    Simulator::Stop(simTime);
    Simulator::Run();

    std::map<Time, Vector> nodePositions =
        nearRtRic->Data()->GetNodePositions(1, Seconds(0), simTime, 12);
    Vector firstPosition = nodePositions[Seconds(2)];
    Vector lastPosition = nodePositions[Seconds(12)];

    // Check the node's first reported postion.
    NS_TEST_ASSERT_MSG_EQ_TOL(firstPosition.x,
                              0.0,
                              0.001,
                              "First position x-coordinate does not match.");
    NS_TEST_ASSERT_MSG_EQ_TOL(firstPosition.y,
                              0.0,
                              0.001,
                              "First position y-coordinate does not match.");
    NS_TEST_ASSERT_MSG_EQ_TOL(firstPosition.z,
                              0.0,
                              0.001,
                              "First position z-coordinate does not match.");

    // Check the node's last reported position.
    NS_TEST_ASSERT_MSG_EQ_TOL(lastPosition.x,
                              20.0,
                              0.001,
                              "Last position x-coordinate does not match.");
    NS_TEST_ASSERT_MSG_EQ_TOL(lastPosition.y,
                              20.0,
                              0.001,
                              "Last position y-coordinate does not match.");
    NS_TEST_ASSERT_MSG_EQ_TOL(lastPosition.z,
                              0.0,
                              0.001,
                              "Last position z-coordinate does not match.");

    Simulator::Destroy();
}

/**
 * @ingroup oran
 *
 * Test suite for the O-RAN module
 */
class OranTestSuite : public TestSuite
{
  public:
    /**
     * Constructor for the test suite
     */
    OranTestSuite();
};

OranTestSuite::OranTestSuite()
    : TestSuite("oran", Type::UNIT)
{
    AddTestCase(new OranTestCaseMobility1, Duration::QUICK);
}

static OranTestSuite soranTestSuite;
