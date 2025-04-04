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

#include "oran-reporter-apploss.h"

#include "oran-report-apploss.h"

#include "ns3/abort.h"
#include "ns3/address.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranReporterAppLoss");
NS_OBJECT_ENSURE_REGISTERED(OranReporterAppLoss);

TypeId
OranReporterAppLoss::GetTypeId()
{
    static TypeId tid = TypeId("ns3::OranReporterAppLoss")
                            .SetParent<OranReporter>()
                            .AddConstructor<OranReporterAppLoss>();

    return tid;
}

OranReporterAppLoss::OranReporterAppLoss()
{
    NS_LOG_FUNCTION(this);

    m_tx = 0;
    m_rx = 0;
}

OranReporterAppLoss::~OranReporterAppLoss()
{
    NS_LOG_FUNCTION(this);
}

void
OranReporterAppLoss::AddTx(Ptr<const Packet> p)
{
    NS_LOG_FUNCTION(this << p);

    m_tx++;
}

void
OranReporterAppLoss::AddRx(Ptr<const Packet> p, const Address& from)
{
    NS_LOG_FUNCTION(this << p << from);

    m_rx++;
}

std::vector<Ptr<OranReport>>
OranReporterAppLoss::GenerateReports()
{
    NS_LOG_FUNCTION(this);

    std::vector<Ptr<OranReport>> reports;

    if (m_active)
    {
        NS_ABORT_MSG_IF(m_terminator == nullptr,
                        "Attempting to generate reports in reporter with NULL E2 Terminator");

        double loss = 0;
        if (m_rx <= m_tx && m_tx > 0)
        {
            // loss = 1 - (m_rx * 1.0 / m_tx);
            loss = static_cast<double>(m_tx - m_rx) / static_cast<double>(m_tx);
        }

        Ptr<OranReportAppLoss> lossReport = CreateObject<OranReportAppLoss>();
        lossReport->SetAttribute("ReporterE2NodeId", UintegerValue(m_terminator->GetE2NodeId()));
        lossReport->SetAttribute("Time", TimeValue(Simulator::Now()));
        lossReport->SetAttribute("Loss", DoubleValue(loss));

        reports.push_back(lossReport);
        m_tx = 0;
        m_rx = 0;
    }

    return reports;
}

} // namespace ns3
