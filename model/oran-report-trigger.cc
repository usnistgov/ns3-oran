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

#include "oran-report-trigger.h"

#include "oran-reporter.h"

#include "ns3/boolean.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/string.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranReportTrigger");

NS_OBJECT_ENSURE_REGISTERED(OranReportTrigger);

TypeId
OranReportTrigger::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::OranReportTrigger")
            .SetParent<Object>()
            .AddAttribute("Reporter",
                          "The reporter.",
                          PointerValue(nullptr),
                          MakePointerAccessor(&OranReportTrigger::m_reporter),
                          MakePointerChecker<OranReporter>())
            .AddAttribute("InitialReport",
                          "Indicates if an initial report should be generated upon registration.",
                          BooleanValue(false),
                          MakeBooleanAccessor(&OranReportTrigger::m_initialReport),
                          MakeBooleanChecker());

    return tid;
}

OranReportTrigger::OranReportTrigger()
    : Object(),
      m_active(false)
{
    NS_LOG_FUNCTION(this);
}

OranReportTrigger::~OranReportTrigger()
{
    NS_LOG_FUNCTION(this);
}

void
OranReportTrigger::Activate(Ptr<OranReporter> reporter)
{
    NS_LOG_FUNCTION(this << reporter);

    if (!m_active)
    {
        NS_ABORT_MSG_IF(reporter == nullptr, "Attempting to link to a NULL reporter.");

        m_active = true;
        m_reporter = reporter;
        m_reporter->SetAttribute("Trigger", PointerValue(GetObject<OranReportTrigger>()));
    }
}

void
OranReportTrigger::Deactivate()
{
    NS_LOG_FUNCTION(this);

    if (m_active)
    {
        NS_ABORT_MSG_IF(m_reporter == nullptr, "Attempting to unlink from a NULL reporter.");

        m_reporter->SetAttribute("Trigger", PointerValue(nullptr));
        m_reporter = nullptr;
        m_active = false;
    }
}

void
OranReportTrigger::NotifyRegistrationComplete()
{
    if (m_active && m_initialReport)
    {
        TriggerReport();
    }
}

void
OranReportTrigger::DoDispose()
{
    NS_LOG_FUNCTION(this);

    m_reporter = nullptr;

    Object::DoDispose();
}

void
OranReportTrigger::TriggerReport()
{
    NS_LOG_FUNCTION(this);

    NS_ABORT_MSG_IF(m_reporter == nullptr, "Attempting to trigger a report with a NULL reporter.");

    m_reporter->PerformReport();
}

} // namespace ns3
