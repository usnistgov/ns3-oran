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

#include "oran-reporter-lte-ue-rsrp-rsrq.h"

#include "oran-report-lte-ue-rsrp-rsrq.h"

#include "ns3/abort.h"
#include "ns3/address.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranReporterLteUeRsrpRsrq");
NS_OBJECT_ENSURE_REGISTERED(OranReporterLteUeRsrpRsrq);

TypeId
OranReporterLteUeRsrpRsrq::GetTypeId()
{
    static TypeId tid = TypeId("ns3::OranReporterLteUeRsrpRsrq")
                            .SetParent<OranReporter>()
                            .AddConstructor<OranReporterLteUeRsrpRsrq>();

    return tid;
}

OranReporterLteUeRsrpRsrq::OranReporterLteUeRsrpRsrq()
{
    NS_LOG_FUNCTION(this);
}

OranReporterLteUeRsrpRsrq::~OranReporterLteUeRsrpRsrq()
{
    NS_LOG_FUNCTION(this);
}

void
OranReporterLteUeRsrpRsrq::ReportRsrpRsrq(uint16_t rnti,
                                          uint16_t cellId,
                                          double rsrp,
                                          double rsrq,
                                          bool isServingCell,
                                          uint8_t componentCarrierId)
{
    NS_LOG_FUNCTION(this << +rnti << +cellId << rsrp << rsrq << isServingCell
                         << componentCarrierId);

    if (m_active)
    {
        NS_ABORT_MSG_IF(m_terminator == nullptr,
                        "Attempting to generate reports in reporter with NULL E2 Terminator");

        Ptr<OranReportLteUeRsrpRsrq> report = CreateObject<OranReportLteUeRsrpRsrq>();
        report->SetAttribute("ReporterE2NodeId", UintegerValue(m_terminator->GetE2NodeId()));
        report->SetAttribute("Time", TimeValue(Simulator::Now()));
        report->SetAttribute("Rnti", UintegerValue(rnti));
        report->SetAttribute("CellId", UintegerValue(cellId));
        report->SetAttribute("Rsrp", DoubleValue(rsrp));
        report->SetAttribute("Rsrq", DoubleValue(rsrq));
        report->SetAttribute("IsServingCell", BooleanValue(isServingCell));
        report->SetAttribute("ComponentCarrierId", UintegerValue(componentCarrierId));

        m_reports.push_back(report);
    }
}

std::vector<Ptr<OranReport>>
OranReporterLteUeRsrpRsrq::GenerateReports()
{
    NS_LOG_FUNCTION(this);

    std::vector<Ptr<OranReport>> reports;

    if (m_active)
    {
        reports = m_reports;
        m_reports.clear();
    }

    return reports;
}

} // namespace ns3
