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

#include "oran-report-trigger-location-change.h"

#include "oran-reporter.h"

#include "ns3/log.h"
#include "ns3/lte-ue-net-device.h"
#include "ns3/lte-ue-rrc.h"
#include "ns3/nstime.h"
#include "ns3/pointer.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/string.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranReportTriggerLocationChange");

NS_OBJECT_ENSURE_REGISTERED(OranReportTriggerLocationChange);

TypeId
OranReportTriggerLocationChange::GetTypeId()
{
    static TypeId tid = TypeId("ns3::OranReportTriggerLocationChange")
                            .SetParent<OranReportTrigger>()
                            .AddConstructor<OranReportTriggerLocationChange>();

    return tid;
}

OranReportTriggerLocationChange::OranReportTriggerLocationChange()
    : OranReportTrigger()
{
    NS_LOG_FUNCTION(this);
}

OranReportTriggerLocationChange::~OranReportTriggerLocationChange()
{
    NS_LOG_FUNCTION(this);
}

void
OranReportTriggerLocationChange::Activate(Ptr<OranReporter> reporter)
{
    NS_LOG_FUNCTION(this << reporter);

    if (!m_active)
    {
        Ptr<MobilityModel> mobility =
            reporter->GetTerminator()->GetNode()->GetObject<MobilityModel>();

        NS_ABORT_MSG_IF(mobility == nullptr, "Unable to find mobility model");

        mobility->TraceConnectWithoutContext(
            "CourseChange",
            MakeCallback(&OranReportTriggerLocationChange::CourseChangedSink, this));
    }

    OranReportTrigger::Activate(reporter);
}

void
OranReportTriggerLocationChange::Deactivate()
{
    NS_LOG_FUNCTION(this);

    if (m_active)
    {
        DisconnectSink();
    }

    OranReportTrigger::Deactivate();
}

void
OranReportTriggerLocationChange::DoDispose()
{
    NS_LOG_FUNCTION(this);

    if (m_active)
    {
        DisconnectSink();
    }

    OranReportTrigger::DoDispose();
}

void
OranReportTriggerLocationChange::CourseChangedSink(Ptr<const MobilityModel> mobility)
{
    NS_LOG_FUNCTION(this << mobility);

    NS_LOG_LOGIC("Location change triggering report");

    TriggerReport();
}

void
OranReportTriggerLocationChange::DisconnectSink()
{
    NS_LOG_FUNCTION(this);

    Ptr<MobilityModel> mobility =
        m_reporter->GetTerminator()->GetNode()->GetObject<MobilityModel>();

    NS_ABORT_MSG_IF(mobility == nullptr, "Unable to find mobility model");

    mobility->TraceDisconnectWithoutContext(
        "CourseChange",
        MakeCallback(&OranReportTriggerLocationChange::CourseChangedSink, this));
}

} // namespace ns3
