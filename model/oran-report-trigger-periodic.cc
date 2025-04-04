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

#include "oran-report-trigger-periodic.h"

#include "oran-reporter.h"

#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/pointer.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/string.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranReportTriggerPeriodic");

NS_OBJECT_ENSURE_REGISTERED(OranReportTriggerPeriodic);

TypeId
OranReportTriggerPeriodic::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::OranReportTriggerPeriodic")
            .SetParent<OranReportTrigger>()
            .AddConstructor<OranReportTriggerPeriodic>()
            .AddAttribute("IntervalRv",
                          "The random variable used to generate the delay (in seconds) between "
                          "periodic reports.",
                          StringValue("ns3::ConstantRandomVariable[Constant=1]"),
                          MakePointerAccessor(&OranReportTriggerPeriodic::m_intervalRv),
                          MakePointerChecker<RandomVariableStream>());

    return tid;
}

OranReportTriggerPeriodic::OranReportTriggerPeriodic()
    : OranReportTrigger(),
      m_triggerEvent(EventId())
{
    NS_LOG_FUNCTION(this);
}

OranReportTriggerPeriodic::~OranReportTriggerPeriodic()
{
    NS_LOG_FUNCTION(this);
}

void
OranReportTriggerPeriodic::Activate(Ptr<OranReporter> reporter)
{
    NS_LOG_FUNCTION(this << reporter);

    if (!m_active)
    {
        ScheduleNextTrigger();
    }

    OranReportTrigger::Activate(reporter);
}

void
OranReportTriggerPeriodic::Deactivate()
{
    NS_LOG_FUNCTION(this);

    if (m_active)
    {
        CancelNextTrigger();
    }

    OranReportTrigger::Deactivate();
}

void
OranReportTriggerPeriodic::DoDispose()
{
    NS_LOG_FUNCTION(this);

    if (m_triggerEvent.IsPending())
    {
        m_triggerEvent.Cancel();
    }

    m_intervalRv = nullptr;

    OranReportTrigger::DoDispose();
}

void
OranReportTriggerPeriodic::TriggerReport()
{
    NS_LOG_FUNCTION(this);

    if (m_active)
    {
        ScheduleNextTrigger();
    }

    OranReportTrigger::TriggerReport();
}

void
OranReportTriggerPeriodic::CancelNextTrigger()
{
    NS_LOG_FUNCTION(this);

    if (m_triggerEvent.IsPending())
    {
        m_triggerEvent.Cancel();
    }
}

void
OranReportTriggerPeriodic::ScheduleNextTrigger()
{
    NS_LOG_FUNCTION(this);

    m_triggerEvent = Simulator::Schedule(Seconds(m_intervalRv->GetValue()),
                                         &OranReportTriggerPeriodic::TriggerReport,
                                         this);
}

} // namespace ns3
