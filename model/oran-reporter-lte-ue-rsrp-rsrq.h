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

#ifndef ORAN_REPORTER_LTE_UE_RSRP_RSRQ
#define ORAN_REPORTER_LTE_UE_RSRP_RSRQ

#include "oran-report.h"
#include "oran-reporter.h"

#include "ns3/ptr.h"

#include <vector>

namespace ns3
{

class Packet;
class Address;

/**
 * @ingroup oran
 *
 * A Reporter that captures the LTE UE RSRP and RSRQ of the node.
 */
class OranReporterLteUeRsrpRsrq : public OranReporter
{
  public:
    /**
     * Get the TypeId of the OranReporterLteUeRsrpRsrq class.
     *
     * @return The TypeId.
     */
    static TypeId GetTypeId();
    /**
     * Constructor of the OranReporterLteUeRsrpRsrq class.
     */
    OranReporterLteUeRsrpRsrq();
    /**
     * Destructor of the OranReporterLteUeRsrpRsrq class.
     */
    ~OranReporterLteUeRsrpRsrq() override;
    /**
     * Reports the RSRP and RSRQ for an LTE UE.
     *
     * @param rnti The RNTI of the UE.
     * @param cellId The cell ID.
     * @param rsrp The RSRP.
     * @param rsrq The RSRQ.
     * @param isServingCell A flag that indicates if this is the serving cell.
     * @param componentCarrierId The component carrier ID.
     */
    void ReportRsrpRsrq(uint16_t rnti,
                        uint16_t cellId,
                        double rsrp,
                        double rsrq,
                        bool isServingCell,
                        uint8_t componentCarrierId);

  protected:
    /**
     * Returns the genrated OranReportLteUeRsrpRsrq.
     *
     * @return The generated Report.
     */
    std::vector<Ptr<OranReport>> GenerateReports() override;

  private:
    /**
     * The reports.
     */
    std::vector<Ptr<OranReport>> m_reports;
};

} // namespace ns3

#endif // ORAN_REPORTER_LTE_UE_RSRP_RSRQ
