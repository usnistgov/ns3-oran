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

#include "oran-reporter.h"

#include "oran-report-trigger.h"

#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/string.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranReporter");

NS_OBJECT_ENSURE_REGISTERED(OranReporter);

TypeId
OranReporter::GetTypeId()
{
    static TypeId tid = TypeId("ns3::OranReporter")
                            .SetParent<Object>()
                            .AddAttribute("Terminator",
                                          "The E2 Node terminator.",
                                          PointerValue(nullptr),
                                          MakePointerAccessor(&OranReporter::m_terminator),
                                          MakePointerChecker<OranE2NodeTerminator>())
                            .AddAttribute("Trigger",
                                          "The trigger that causes the generation of reports.",
                                          StringValue("ns3::OranReportTriggerPeriodic"),
                                          MakePointerAccessor(&OranReporter::m_trigger),
                                          MakePointerChecker<OranReportTrigger>());

    return tid;
}

OranReporter::OranReporter()
    : Object(),
      m_active(false)
{
    NS_LOG_FUNCTION(this);
}

OranReporter::~OranReporter()
{
    NS_LOG_FUNCTION(this);
}

void
OranReporter::Activate()
{
    NS_LOG_FUNCTION(this);

    if (!m_active && m_terminator != nullptr && m_trigger != nullptr)
    {
        m_active = true;
        m_trigger->Activate(GetObject<OranReporter>());
    }
    else
    {
        Deactivate();
    }
}

void
OranReporter::Deactivate()
{
    NS_LOG_FUNCTION(this);

    if (m_active)
    {
        m_trigger->Deactivate();
        m_active = false;
    }
}

bool
OranReporter::IsActive() const
{
    NS_LOG_FUNCTION(this);

    return m_active;
}

void
OranReporter::PerformReport()
{
    NS_LOG_FUNCTION(this);

    if (m_active)
    {
        if (m_terminator == nullptr)
        {
            // The E2 Node Terminator is not set, so we do not
            // know where to send the report. We switch to deactivated state
            // until we get a pointer
            Deactivate();
        }
        else
        {
            std::vector<Ptr<OranReport>> reports = GenerateReports();

            for (const auto& r : reports)
            {
                m_terminator->StoreReport(r);
            }
        }
    }
}

void
OranReporter::NotifyRegistrationComplete()
{
    NS_LOG_FUNCTION(this);

    NS_ABORT_MSG_IF(m_trigger == nullptr,
                    "Attempting to notify successfull registration to NULL trigger.");

    m_trigger->NotifyRegistrationComplete();
}

void
OranReporter::DoDispose()
{
    NS_LOG_FUNCTION(this);

    m_terminator = nullptr;
    m_trigger = nullptr;

    Object::DoDispose();
}

Ptr<OranE2NodeTerminator>
OranReporter::GetTerminator() const
{
    NS_LOG_FUNCTION(this);

    return m_terminator;
}

} // namespace ns3
