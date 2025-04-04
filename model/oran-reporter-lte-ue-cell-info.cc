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

#include "oran-reporter-lte-ue-cell-info.h"

#include "oran-report-lte-ue-cell-info.h"

#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/lte-ue-net-device.h"
#include "ns3/lte-ue-rrc.h"
#include "ns3/mobility-model.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranReporterLteUeCellInfo");

NS_OBJECT_ENSURE_REGISTERED(OranReporterLteUeCellInfo);

TypeId
OranReporterLteUeCellInfo::GetTypeId()
{
    static TypeId tid = TypeId("ns3::OranReporterLteUeCellInfo")
                            .SetParent<OranReporter>()
                            .AddConstructor<OranReporterLteUeCellInfo>();

    return tid;
}

OranReporterLteUeCellInfo::OranReporterLteUeCellInfo()
    : OranReporter()
{
    NS_LOG_FUNCTION(this);
}

OranReporterLteUeCellInfo::~OranReporterLteUeCellInfo()
{
    NS_LOG_FUNCTION(this);
}

std::vector<Ptr<OranReport>>
OranReporterLteUeCellInfo::GenerateReports()
{
    NS_LOG_FUNCTION(this);

    std::vector<Ptr<OranReport>> reports;
    if (m_active)
    {
        NS_ABORT_MSG_IF(m_terminator == nullptr,
                        "Attempting to generate reports in reporter with NULL E2 Terminator");

        Ptr<LteUeNetDevice> lteUeNetDev = nullptr;
        Ptr<Node> node = m_terminator->GetNode();
        Ptr<OranReportLteUeCellInfo> cellInfoReport = CreateObject<OranReportLteUeCellInfo>();

        for (uint32_t idx = 0; lteUeNetDev == nullptr && idx < node->GetNDevices(); idx++)
        {
            lteUeNetDev = node->GetDevice(idx)->GetObject<LteUeNetDevice>();
        }

        NS_ABORT_MSG_IF(lteUeNetDev == nullptr, "Unable to find appropriate network device");

        Ptr<LteUeRrc> lteUeRrc = lteUeNetDev->GetRrc();

        cellInfoReport->SetAttribute("ReporterE2NodeId",
                                     UintegerValue(m_terminator->GetE2NodeId()));
        cellInfoReport->SetAttribute("CellId", UintegerValue(lteUeRrc->GetCellId()));
        cellInfoReport->SetAttribute("Rnti", UintegerValue(lteUeRrc->GetRnti()));
        cellInfoReport->SetAttribute("Time", TimeValue(Simulator::Now()));

        reports.push_back(cellInfoReport);
    }

    return reports;
}

} // namespace ns3
