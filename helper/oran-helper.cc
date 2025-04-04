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

#include "oran-helper.h"

#include "ns3/abort.h"
#include "ns3/boolean.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/oran-cmm.h"
#include "ns3/oran-data-repository.h"
#include "ns3/oran-e2-node-terminator.h"
#include "ns3/oran-lm.h"
#include "ns3/oran-near-rt-ric-e2terminator.h"
#include "ns3/oran-query-trigger.h"
#include "ns3/oran-reporter.h"
#include "ns3/pointer.h"
#include "ns3/string.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranHelper");

NS_OBJECT_ENSURE_REGISTERED(OranHelper);

TypeId
OranHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::OranHelper")
            .SetParent<Object>()
            .AddConstructor<OranHelper>()
            .AddAttribute("Verbose",
                          "Indicates if components should be verbose.",
                          BooleanValue(false),
                          MakeBooleanAccessor(&OranHelper::m_verbose),
                          MakeBooleanChecker())
            .AddAttribute(
                "LmQueryInterval",
                "Interval between periodic queries to the Logic Modules in the Near-RT RIC.",
                TimeValue(Seconds(5)),
                MakeTimeAccessor(&OranHelper::m_ricLmQueryInterval),
                MakeTimeChecker(MilliSeconds(10)))
            .AddAttribute("LmQueryMaxWaitTime",
                          "The maximum time to wait for Logic Modules to finish. A value of \"0\" "
                          "indicates no limit.",
                          TimeValue(Seconds(0)),
                          MakeTimeAccessor(&OranHelper::m_ricLmQueryMaxWaitTime),
                          MakeTimeChecker())
            .AddAttribute("LmQueryLateCommandPolicy",
                          "The filter to apply on UL CQIs received",
                          EnumValue(OranNearRtRic::DROP),
                          MakeEnumAccessor<OranNearRtRic::LateCommandPolicy>(
                              &OranHelper::m_ricLmQueryLateCommandPolicy),
                          MakeEnumChecker(OranNearRtRic::DROP, "DROP", OranNearRtRic::SAVE, "SAVE"))
            .AddAttribute("E2NodeInactivityThreshold",
                          "The amount of time since an E2 node's last registration request before "
                          "deregistration.",
                          TimeValue(Seconds(2)),
                          MakeTimeAccessor(&OranHelper::m_e2NodeInactivityThreshold),
                          MakeTimeChecker())
            .AddAttribute(
                "E2NodeInactivityIntervalRv",
                "The random variable used (in seconds) to periodically deregister inactive nodes.",
                StringValue("ns3::ConstantRandomVariable[Constant=2]"),
                MakePointerAccessor(&OranHelper::m_e2NodeInactivityIntervalRv),
                MakePointerChecker<RandomVariableStream>())
            .AddAttribute("RicTransmissionDelayRv",
                          "The random variable used (in seconds) to calculate the transmission "
                          "delay for a command.",
                          StringValue("ns3::ConstantRandomVariable[Constant=0]"),
                          MakePointerAccessor(&OranHelper::m_ricTransmissionDelayRv),
                          MakePointerChecker<RandomVariableStream>());

    return tid;
}

OranHelper::OranHelper()
    : Object(),
      m_lmFactories(std::vector<ObjectFactory>()),
      m_reporterFactories(std::vector<ObjectFactory>())
{
    NS_LOG_FUNCTION(this);

    // RIC Default component types
    m_dataRepositoryFactory.SetTypeId("ns3::OranDataRepositorySqlite");
    m_defaultLmFactory.SetTypeId("ns3::OranLmNoop");
    m_cmmFactory.SetTypeId("ns3::OranCmmNoop");

    // Default E2 Node Terminator type
    m_e2NodeTerminatorFactory.SetTypeId("ns3::OranE2NodeTerminatorWired");
}

OranHelper::~OranHelper()
{
    NS_LOG_FUNCTION(this);
}

void
OranHelper::DoDispose()
{
    NS_LOG_FUNCTION(this);

    Object::DoDispose();
}

void
OranHelper::AddLogicModule(std::string tid,
                           std::string n0,
                           const AttributeValue& v0,
                           std::string n1,
                           const AttributeValue& v1,
                           std::string n2,
                           const AttributeValue& v2,
                           std::string n3,
                           const AttributeValue& v3,
                           std::string n4,
                           const AttributeValue& v4,
                           std::string n5,
                           const AttributeValue& v5,
                           std::string n6,
                           const AttributeValue& v6,
                           std::string n7,
                           const AttributeValue& v7)
{
    NS_LOG_FUNCTION(this);

    ObjectFactory factory;

    factory.SetTypeId(tid);
    factory.Set(n0, v0);
    factory.Set(n1, v1);
    factory.Set(n2, v2);
    factory.Set(n3, v3);
    factory.Set(n4, v4);
    factory.Set(n5, v5);
    factory.Set(n6, v6);
    factory.Set(n7, v7);

    m_lmFactories.push_back(factory);
}

void
OranHelper::AddReporter(std::string tid,
                        std::string n0,
                        const AttributeValue& v0,
                        std::string n1,
                        const AttributeValue& v1,
                        std::string n2,
                        const AttributeValue& v2,
                        std::string n3,
                        const AttributeValue& v3,
                        std::string n4,
                        const AttributeValue& v4,
                        std::string n5,
                        const AttributeValue& v5,
                        std::string n6,
                        const AttributeValue& v6,
                        std::string n7,
                        const AttributeValue& v7)
{
    NS_LOG_FUNCTION(this);

    ObjectFactory factory;

    factory.SetTypeId(tid);
    factory.Set(n0, v0);
    factory.Set(n1, v1);
    factory.Set(n2, v2);
    factory.Set(n3, v3);
    factory.Set(n4, v4);
    factory.Set(n5, v5);
    factory.Set(n6, v6);
    factory.Set(n7, v7);

    m_reporterFactories.push_back(factory);
}

void
OranHelper::AddQueryTrigger(std::string name,
                            std::string tid,
                            std::string n0,
                            const AttributeValue& v0,
                            std::string n1,
                            const AttributeValue& v1,
                            std::string n2,
                            const AttributeValue& v2,
                            std::string n3,
                            const AttributeValue& v3,
                            std::string n4,
                            const AttributeValue& v4,
                            std::string n5,
                            const AttributeValue& v5,
                            std::string n6,
                            const AttributeValue& v6,
                            std::string n7,
                            const AttributeValue& v7)
{
    NS_LOG_FUNCTION(this);

    ObjectFactory factory;

    factory.SetTypeId(tid);
    factory.Set(n0, v0);
    factory.Set(n1, v1);
    factory.Set(n2, v2);
    factory.Set(n3, v3);
    factory.Set(n4, v4);
    factory.Set(n5, v5);
    factory.Set(n6, v6);
    factory.Set(n7, v7);

    m_queryTriggerFactories[name] = factory;
}

void
OranHelper::SetConflictMitigationModule(std::string tid,
                                        std::string n0,
                                        const AttributeValue& v0,
                                        std::string n1,
                                        const AttributeValue& v1,
                                        std::string n2,
                                        const AttributeValue& v2,
                                        std::string n3,
                                        const AttributeValue& v3,
                                        std::string n4,
                                        const AttributeValue& v4,
                                        std::string n5,
                                        const AttributeValue& v5,
                                        std::string n6,
                                        const AttributeValue& v6,
                                        std::string n7,
                                        const AttributeValue& v7)
{
    NS_LOG_FUNCTION(this);

    ObjectFactory factory;

    factory.SetTypeId(tid);
    factory.Set(n0, v0);
    factory.Set(n1, v1);
    factory.Set(n2, v2);
    factory.Set(n3, v3);
    factory.Set(n4, v4);
    factory.Set(n5, v5);
    factory.Set(n6, v6);
    factory.Set(n7, v7);

    m_cmmFactory = factory;
}

void
OranHelper::SetDataRepository(std::string tid,
                              std::string n0,
                              const AttributeValue& v0,
                              std::string n1,
                              const AttributeValue& v1,
                              std::string n2,
                              const AttributeValue& v2,
                              std::string n3,
                              const AttributeValue& v3,
                              std::string n4,
                              const AttributeValue& v4,
                              std::string n5,
                              const AttributeValue& v5,
                              std::string n6,
                              const AttributeValue& v6,
                              std::string n7,
                              const AttributeValue& v7)
{
    NS_LOG_FUNCTION(this);

    ObjectFactory factory;

    factory.SetTypeId(tid);
    factory.Set(n0, v0);
    factory.Set(n1, v1);
    factory.Set(n2, v2);
    factory.Set(n3, v3);
    factory.Set(n4, v4);
    factory.Set(n5, v5);
    factory.Set(n6, v6);
    factory.Set(n7, v7);

    m_dataRepositoryFactory = factory;
}

void
OranHelper::SetDefaultLogicModule(std::string tid,
                                  std::string n0,
                                  const AttributeValue& v0,
                                  std::string n1,
                                  const AttributeValue& v1,
                                  std::string n2,
                                  const AttributeValue& v2,
                                  std::string n3,
                                  const AttributeValue& v3,
                                  std::string n4,
                                  const AttributeValue& v4,
                                  std::string n5,
                                  const AttributeValue& v5,
                                  std::string n6,
                                  const AttributeValue& v6,
                                  std::string n7,
                                  const AttributeValue& v7)
{
    NS_LOG_FUNCTION(this);

    ObjectFactory factory;

    factory.SetTypeId(tid);
    factory.Set(n0, v0);
    factory.Set(n1, v1);
    factory.Set(n2, v2);
    factory.Set(n3, v3);
    factory.Set(n4, v4);
    factory.Set(n5, v5);
    factory.Set(n6, v6);
    factory.Set(n7, v7);

    m_defaultLmFactory = factory;
}

void
OranHelper::SetE2NodeTerminator(std::string tid,
                                std::string n0,
                                const AttributeValue& v0,
                                std::string n1,
                                const AttributeValue& v1,
                                std::string n2,
                                const AttributeValue& v2,
                                std::string n3,
                                const AttributeValue& v3,
                                std::string n4,
                                const AttributeValue& v4,
                                std::string n5,
                                const AttributeValue& v5,
                                std::string n6,
                                const AttributeValue& v6,
                                std::string n7,
                                const AttributeValue& v7)
{
    NS_LOG_FUNCTION(this);

    ObjectFactory factory;

    factory.SetTypeId(tid);
    factory.Set(n0, v0);
    factory.Set(n1, v1);
    factory.Set(n2, v2);
    factory.Set(n3, v3);
    factory.Set(n4, v4);
    factory.Set(n5, v5);
    factory.Set(n6, v6);
    factory.Set(n7, v7);

    m_e2NodeTerminatorFactory = factory;
}

Ptr<OranNearRtRic>
OranHelper::CreateNearRtRic()
{
    NS_LOG_FUNCTION(this);

    Ptr<OranNearRtRic> nearRtRic = CreateObject<OranNearRtRic>();
    Ptr<OranDataRepository> dataRepository = m_dataRepositoryFactory.Create<OranDataRepository>();
    Ptr<OranLm> defaultLm = m_defaultLmFactory.Create<OranLm>();
    Ptr<OranCmm> cmm = m_cmmFactory.Create<OranCmm>();
    Ptr<OranNearRtRicE2Terminator> nearRtRicE2Terminator =
        CreateObject<OranNearRtRicE2Terminator>();

    defaultLm->SetAttribute("NearRtRic", PointerValue(nearRtRic));
    defaultLm->SetAttribute("Verbose", BooleanValue(m_verbose));

    cmm->SetAttribute("NearRtRic", PointerValue(nearRtRic));
    cmm->SetAttribute("Verbose", BooleanValue(m_verbose));

    nearRtRicE2Terminator->SetAttribute("NearRtRic", PointerValue(nearRtRic));
    nearRtRicE2Terminator->SetAttribute("DataRepository", PointerValue(dataRepository));
    nearRtRicE2Terminator->SetAttribute("TransmissionDelayRv",
                                        PointerValue(m_ricTransmissionDelayRv));

    nearRtRic->SetAttribute("DefaultLogicModule", PointerValue(defaultLm));
    nearRtRic->SetAttribute("E2Terminator", PointerValue(nearRtRicE2Terminator));
    nearRtRic->SetAttribute("DataRepository", PointerValue(dataRepository));
    nearRtRic->SetAttribute("ConflictMitigationModule", PointerValue(cmm));
    nearRtRic->SetAttribute("LmQueryInterval", TimeValue(m_ricLmQueryInterval));
    nearRtRic->SetAttribute("E2NodeInactivityThreshold", TimeValue(m_e2NodeInactivityThreshold));
    nearRtRic->SetAttribute("E2NodeInactivityIntervalRv",
                            PointerValue(m_e2NodeInactivityIntervalRv));
    nearRtRic->SetAttribute("LmQueryMaxWaitTime", TimeValue(m_ricLmQueryMaxWaitTime));
    nearRtRic->SetAttribute("LmQueryLateCommandPolicy", EnumValue(m_ricLmQueryLateCommandPolicy));

    for (auto lmFactory : m_lmFactories)
    {
        Ptr<OranLm> lm = lmFactory.Create<OranLm>();

        if (nearRtRic->AddLogicModule(lm) == OranNearRtRic::ADDLM_OK)
        {
            lm->SetAttribute("NearRtRic", PointerValue(nearRtRic));
            lm->SetAttribute("Verbose", BooleanValue(m_verbose));
        }
        else
        {
            NS_ABORT_MSG("Could not add additional logic module to Near-RT RIC.");
        }
    }

    m_lmFactories.clear();

    for (auto queryTriggerFactory : m_queryTriggerFactories)
    {
        Ptr<OranQueryTrigger> queryTrigger = queryTriggerFactory.second.Create<OranQueryTrigger>();

        if (!nearRtRic->AddQueryTrigger(queryTriggerFactory.first, queryTrigger))
        {
            NS_ABORT_MSG("Could not add the \"" << queryTriggerFactory.first
                                                << "\" query trigger to Near-RT RIC.");
        }
    }

    m_queryTriggerFactories.clear();

    return nearRtRic;
}

void
OranHelper::ActivateAndStartNearRtRic(Ptr<OranNearRtRic> nearRtRic)
{
    nearRtRic->Start();
}

void
OranHelper::DeactivateAndStopNearRtRic(Ptr<OranNearRtRic> nearRtRic)
{
    nearRtRic->Stop();
}

OranE2NodeTerminatorContainer
OranHelper::DeployTerminators(Ptr<OranNearRtRic> nearRtRic,
                              NodeContainer nodes,
                              uint32_t netDeviceIndex)
{
    NS_LOG_FUNCTION(this);

    OranE2NodeTerminatorContainer terminators;

    NS_ABORT_MSG_IF(nearRtRic == nullptr,
                    "Attempting to deploy E2 Node Terminators with a null Near-RT RIC");

    for (auto nodeIt = nodes.Begin(); nodeIt != nodes.End(); nodeIt++)
    {
        Ptr<OranE2NodeTerminator> terminator =
            m_e2NodeTerminatorFactory.Create<OranE2NodeTerminator>();
        Ptr<Node> node = *nodeIt;

        terminator->SetAttribute("NearRtRic", PointerValue(nearRtRic));

        for (auto reporterFactory : m_reporterFactories)
        {
            Ptr<OranReporter> reporter = reporterFactory.Create<OranReporter>();

            reporter->SetAttribute("Terminator", PointerValue(terminator));
            terminator->AddReporter(reporter);
        }

        terminator->Attach(*nodeIt, netDeviceIndex);

        terminators.Add(terminator);
    }

    m_reporterFactories.clear();

    return terminators;
}

void
OranHelper::ActivateE2NodeTerminators(OranE2NodeTerminatorContainer e2NodeTerminators)
{
    e2NodeTerminators.Activate();
}

void
OranHelper::DeactivateE2NodeTerminators(OranE2NodeTerminatorContainer e2NodeTerminators)
{
    e2NodeTerminators.Deactivate();
}

} // namespace ns3
