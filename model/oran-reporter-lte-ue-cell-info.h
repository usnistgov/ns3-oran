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

#ifndef ORAN_REPORTER_LTE_UE_CELL_INFO_H
#define ORAN_REPORTER_LTE_UE_CELL_INFO_H

#include "oran-report.h"
#include "oran-reporter.h"

#include "ns3/ptr.h"

#include <vector>

namespace ns3
{

/**
 * @ingroup oran
 *
 * Reporter that attaches to an LTE UE and captures the LTE Cell ID
 * of the eNB the UE is attached to.
 */
class OranReporterLteUeCellInfo : public OranReporter
{
  public:
    /**
     * Get the TypeId of the OranReporterLteUeCellInfo class.
     *
     * @return The TypeId.
     */
    static TypeId GetTypeId();
    /**
     * Constructor of the OranReporterLteUeCellInfo class.
     */
    OranReporterLteUeCellInfo();
    /**
     * Destructor of the OranReporterLteUeCellInfo class.
     */
    ~OranReporterLteUeCellInfo() override;

  protected:
    /**
     * Get the Cell ID of the attached LTE cell, and generate an
     * OranReportLteUeCEllInfo.
     *
     * @return The generated Report.
     */
    std::vector<Ptr<OranReport>> GenerateReports() override;
}; // class OranReporterLteUeCellInfo

} // namespace ns3

#endif /* ORAN_REPORTER_LTE_UE_CELL_INFO_H */
