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

#include "oran-reporter-location.h"

#include "oran-report-location.h"

#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranReporterLocation");

NS_OBJECT_ENSURE_REGISTERED(OranReporterLocation);

TypeId
OranReporterLocation::GetTypeId()
{
    static TypeId tid = TypeId("ns3::OranReporterLocation")
                            .SetParent<OranReporter>()
                            .AddConstructor<OranReporterLocation>();

    return tid;
}

OranReporterLocation::OranReporterLocation()
    : OranReporter()
{
    NS_LOG_FUNCTION(this);
}

OranReporterLocation::~OranReporterLocation()
{
    NS_LOG_FUNCTION(this);
}

std::vector<Ptr<OranReport>>
OranReporterLocation::GenerateReports()
{
    NS_LOG_FUNCTION(this);

    std::vector<Ptr<OranReport>> reports;
    if (m_active)
    {
        NS_ABORT_MSG_IF(m_terminator == nullptr,
                        "Attempting to generate reports in reporter with NULL E2 Terminator");
        Ptr<MobilityModel> mobility = m_terminator->GetNode()->GetObject<MobilityModel>();

        Ptr<OranReportLocation> locationReport = CreateObject<OranReportLocation>();
        locationReport->SetAttribute("ReporterE2NodeId",
                                     UintegerValue(m_terminator->GetE2NodeId()));
        locationReport->SetAttribute("Location", VectorValue(mobility->GetPosition()));
        locationReport->SetAttribute("Time", TimeValue(Simulator::Now()));

        reports.push_back(locationReport);
    }
    return reports;
}

} // namespace ns3
