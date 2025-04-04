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

#include "oran-report-trigger-lte-ue-handover.h"

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

NS_LOG_COMPONENT_DEFINE("OranReportTriggerLteUeHandover");

NS_OBJECT_ENSURE_REGISTERED(OranReportTriggerLteUeHandover);

TypeId
OranReportTriggerLteUeHandover::GetTypeId()
{
    static TypeId tid = TypeId("ns3::OranReportTriggerLteUeHandover")
                            .SetParent<OranReportTrigger>()
                            .AddConstructor<OranReportTriggerLteUeHandover>();

    return tid;
}

OranReportTriggerLteUeHandover::OranReportTriggerLteUeHandover()
    : OranReportTrigger()
{
    NS_LOG_FUNCTION(this);
}

OranReportTriggerLteUeHandover::~OranReportTriggerLteUeHandover()
{
    NS_LOG_FUNCTION(this);
}

void
OranReportTriggerLteUeHandover::Activate(Ptr<OranReporter> reporter)
{
    NS_LOG_FUNCTION(this << reporter);

    if (!m_active)
    {
        Ptr<LteUeNetDevice> lteUeNetDev = nullptr;
        Ptr<Node> node = reporter->GetTerminator()->GetNode();

        for (uint32_t idx = 0; lteUeNetDev == nullptr && idx < node->GetNDevices(); idx++)
        {
            lteUeNetDev = node->GetDevice(idx)->GetObject<LteUeNetDevice>();
        }

        NS_ABORT_MSG_IF(lteUeNetDev == nullptr, "Unable to find appropriate network device");

        lteUeNetDev->GetRrc()->TraceConnectWithoutContext(
            "HandoverEndOk",
            MakeCallback(&OranReportTriggerLteUeHandover::HandoverCompleteSink, this));

        lteUeNetDev->GetRrc()->TraceConnectWithoutContext(
            "ConnectionEstablished",
            MakeCallback(&OranReportTriggerLteUeHandover::ConnectionEstablishedSink, this));
    }

    OranReportTrigger::Activate(reporter);
}

void
OranReportTriggerLteUeHandover::Deactivate()
{
    NS_LOG_FUNCTION(this);

    if (m_active)
    {
        DisconnectSink();
    }

    OranReportTrigger::Deactivate();
}

void
OranReportTriggerLteUeHandover::DoDispose()
{
    NS_LOG_FUNCTION(this);

    if (m_active)
    {
        DisconnectSink();
    }

    OranReportTrigger::DoDispose();
}

void
OranReportTriggerLteUeHandover::HandoverCompleteSink(uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
    NS_LOG_FUNCTION(this << imsi << (uint32_t)cellId << (uint32_t)rnti);

    NS_LOG_LOGIC("Handover triggering report");

    TriggerReport();
}

void
OranReportTriggerLteUeHandover::ConnectionEstablishedSink(uint64_t imsi,
                                                          uint16_t cellId,
                                                          uint16_t rnti)
{
    NS_LOG_FUNCTION(this << imsi << (uint32_t)cellId << (uint32_t)rnti);

    NS_LOG_LOGIC("Connection established triggering report");

    TriggerReport();
}

void
OranReportTriggerLteUeHandover::DisconnectSink()
{
    NS_LOG_FUNCTION(this);

    Ptr<LteUeNetDevice> lteUeNetDev = nullptr;
    Ptr<Node> node = m_reporter->GetTerminator()->GetNode();

    for (uint32_t idx = 0; lteUeNetDev == nullptr && idx < node->GetNDevices(); idx++)
    {
        lteUeNetDev = node->GetDevice(idx)->GetObject<LteUeNetDevice>();
    }

    NS_ABORT_MSG_IF(lteUeNetDev == nullptr, "Unable to find appropriate network device");

    lteUeNetDev->GetRrc()->TraceDisconnectWithoutContext(
        "HandoverEndOk",
        MakeCallback(&OranReportTriggerLteUeHandover::HandoverCompleteSink, this));

    lteUeNetDev->GetRrc()->TraceDisconnectWithoutContext(
        "ConnectionEstablished",
        MakeCallback(&OranReportTriggerLteUeHandover::ConnectionEstablishedSink, this));
}

} // namespace ns3
