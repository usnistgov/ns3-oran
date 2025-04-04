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

#include "oran-e2-node-terminator.h"

#include "oran-near-rt-ric-e2terminator.h"
#include "oran-reporter.h"

#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/object-vector.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranE2NodeTerminator");

NS_OBJECT_ENSURE_REGISTERED(OranE2NodeTerminator);

TypeId
OranE2NodeTerminator::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::OranE2NodeTerminator")
            .SetParent<Object>()
            .AddAttribute("E2NodeId",
                          "The E2 Node ID of the terminator.",
                          UintegerValue(0),
                          MakeUintegerAccessor(&OranE2NodeTerminator::m_e2NodeId),
                          MakeUintegerChecker<uint64_t>())
            .AddAttribute("Name",
                          "The name of the terminator.",
                          StringValue("OranE2NodeTerminator"),
                          MakeStringAccessor(&OranE2NodeTerminator::m_name),
                          MakeStringChecker())
            .AddAttribute("NearRtRic",
                          "The Near-RT RIC.",
                          PointerValue(nullptr),
                          MakePointerAccessor(&OranE2NodeTerminator::m_nearRtRic),
                          MakePointerChecker<OranNearRtRic>())
            .AddAttribute("Reporters",
                          "The collection of associated reporters.",
                          ObjectVectorValue(),
                          MakeObjectVectorAccessor(&OranE2NodeTerminator::m_reporters),
                          MakeObjectVectorChecker<OranReporter>())
            .AddAttribute(
                "RegistrationIntervalRv",
                "The random variable used (in seconds) to periodically send registration requests.",
                StringValue("ns3::ConstantRandomVariable[Constant=1]"),
                MakePointerAccessor(&OranE2NodeTerminator::m_registrationIntervalRv),
                MakePointerChecker<RandomVariableStream>())
            .AddAttribute("SendIntervalRv",
                          "The random variable used (in seconds) to schedule when to send reports.",
                          StringValue("ns3::ConstantRandomVariable[Constant=1]"),
                          MakePointerAccessor(&OranE2NodeTerminator::m_sendIntervalRv),
                          MakePointerChecker<RandomVariableStream>())
            .AddAttribute("TransmissionDelayRv",
                          "The random variable used (in seconds) to calculate the transmission "
                          "delay for a report.",
                          StringValue("ns3::ConstantRandomVariable[Constant=0]"),
                          MakePointerAccessor(&OranE2NodeTerminator::m_transmissionDelayRv),
                          MakePointerChecker<RandomVariableStream>());

    return tid;
}

OranE2NodeTerminator::OranE2NodeTerminator()
    : Object(),
      m_active(false),
      m_node(nullptr),
      m_reports(std::vector<Ptr<OranReport>>()),
      m_registrationEvent(EventId()),
      m_sendEvent(EventId())
{
    NS_LOG_FUNCTION(this);
}

OranE2NodeTerminator::~OranE2NodeTerminator()
{
    NS_LOG_FUNCTION(this);
}

void
OranE2NodeTerminator::Activate()
{
    NS_LOG_FUNCTION(this);

    if (m_nearRtRic == nullptr)
    {
        m_active = false;
        Deactivate();
    }
    else
    {
        if (!m_active)
        {
            m_active = true;
            m_reports.clear();

            Register();

            for (auto r : m_reporters)
            {
                r->Activate();
            }
        }
    }
}

void
OranE2NodeTerminator::AddReporter(Ptr<OranReporter> reporter)
{
    NS_LOG_FUNCTION(this << reporter);

    m_reporters.push_back(reporter);
}

void
OranE2NodeTerminator::Attach(Ptr<Node> node, uint32_t netDeviceIndex)
{
    NS_LOG_FUNCTION(this);

    m_node = node;
    m_netDeviceIndex = netDeviceIndex;
}

void
OranE2NodeTerminator::Deactivate()
{
    NS_LOG_FUNCTION(this);

    if (m_active)
    {
        for (auto r : m_reporters)
        {
            r->Deactivate();
        }

        CancelNextSend();
        Deregister();

        m_active = false;
    }
}

bool
OranE2NodeTerminator::IsActive() const
{
    NS_LOG_FUNCTION(this);

    return m_active;
}

void
OranE2NodeTerminator::StoreReport(Ptr<OranReport> report)
{
    NS_LOG_FUNCTION(this << report);

    if (m_active)
    {
        m_reports.push_back(report);
    }
}

void
OranE2NodeTerminator::ReceiveDeregistrationResponse(uint64_t e2NodeId)
{
    NS_LOG_FUNCTION(this << e2NodeId);

    // TODO: Process failed deregistration response
    m_e2NodeId = 0;
}

void
OranE2NodeTerminator::ReceiveRegistrationResponse(uint64_t e2NodeId)
{
    NS_LOG_FUNCTION(this << e2NodeId);

    // TODO: Process failed registration response
    if (m_active)
    {
        if (m_e2NodeId != e2NodeId)
        {
            m_e2NodeId = e2NodeId;

            if (e2NodeId > 0)
            {
                for (auto r : m_reporters)
                {
                    r->NotifyRegistrationComplete();
                }
            }
        }

        ScheduleNextSend();
    }
}

void
OranE2NodeTerminator::CancelNextRegistration()
{
    NS_LOG_FUNCTION(this);

    if (m_registrationEvent.IsPending())
    {
        m_registrationEvent.Cancel();
    }
}

void
OranE2NodeTerminator::CancelNextSend()
{
    NS_LOG_FUNCTION(this);

    if (m_sendEvent.IsPending())
    {
        m_sendEvent.Cancel();
    }
}

void
OranE2NodeTerminator::Deregister()
{
    NS_LOG_FUNCTION(this);

    NS_ABORT_MSG_IF(m_nearRtRic == nullptr, "Attempting to deregister to NULL Near-RT RIC");

    CancelNextRegistration();

    Simulator::Schedule(Seconds(m_transmissionDelayRv->GetValue()),
                        &OranNearRtRicE2Terminator::ReceiveDeregistrationRequest,
                        m_nearRtRic->GetE2Terminator(),
                        m_e2NodeId);
}

void
OranE2NodeTerminator::DoDispose()
{
    NS_LOG_FUNCTION(this);

    CancelNextRegistration();
    CancelNextSend();

    m_node = nullptr;
    m_nearRtRic = nullptr;
    m_reports.clear();
    m_reporters.clear();
    m_registrationIntervalRv = nullptr;
    m_sendIntervalRv = nullptr;

    Object::DoDispose();
}

void
OranE2NodeTerminator::DoSendReports()
{
    NS_LOG_FUNCTION(this);

    if (m_active)
    {
        NS_ABORT_MSG_IF(m_nearRtRic == nullptr,
                        "Attempting to send a report to a null Near-RT RIC");

        for (const auto& r : m_reports)
        {
            Simulator::Schedule(Seconds(m_transmissionDelayRv->GetValue()),
                                &OranNearRtRicE2Terminator::ReceiveReport,
                                m_nearRtRic->GetE2Terminator(),
                                r);
        }

        m_reports.clear();
        ScheduleNextSend();
    }
}

void
OranE2NodeTerminator::Register()
{
    NS_LOG_FUNCTION(this);

    if (m_active)
    {
        NS_ABORT_MSG_IF(m_nearRtRic == nullptr,
                        "Attempting to send a report to a null Near-RT RIC");

        CancelNextRegistration();

        Simulator::Schedule(Seconds(m_transmissionDelayRv->GetValue()),
                            &OranNearRtRicE2Terminator::ReceiveRegistrationRequest,
                            m_nearRtRic->GetE2Terminator(),
                            GetNodeType(),
                            m_e2NodeId,
                            GetObject<OranE2NodeTerminator>());

        TimeValue e2NodeInactivityThresholdVal;
        Time registrationDelay = Seconds(m_registrationIntervalRv->GetValue());
        m_nearRtRic->GetAttribute("E2NodeInactivityThreshold", e2NodeInactivityThresholdVal);

        if (registrationDelay > e2NodeInactivityThresholdVal.Get())
        {
            NS_LOG_WARN("E2 Node Terminator registration delay is larger than Near-RT RIC "
                        "inactivity threshold.");
        }

        m_registrationEvent =
            Simulator::Schedule(registrationDelay, &OranE2NodeTerminator::Register, this);
    }
}

void
OranE2NodeTerminator::ScheduleNextSend()
{
    NS_LOG_FUNCTION(this);

    if (m_active)
    {
        m_sendEvent = Simulator::Schedule(Seconds(m_sendIntervalRv->GetValue()),
                                          &OranE2NodeTerminator::DoSendReports,
                                          this);
    }
}

uint64_t
OranE2NodeTerminator::GetE2NodeId() const
{
    NS_LOG_FUNCTION(this);

    return m_e2NodeId;
}

Ptr<OranNearRtRic>
OranE2NodeTerminator::GetNearRtRic() const
{
    NS_LOG_FUNCTION(this);

    return m_nearRtRic;
}

Ptr<Node>
OranE2NodeTerminator::GetNode() const
{
    NS_LOG_FUNCTION(this);

    return m_node;
}

uint32_t
OranE2NodeTerminator::GetNetDeviceIndex() const
{
    NS_LOG_FUNCTION(this);

    return m_netDeviceIndex;
}

} // namespace ns3
