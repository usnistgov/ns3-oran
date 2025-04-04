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

#ifndef ORAN_REPORT_H
#define ORAN_REPORT_H

#include "ns3/nstime.h"
#include "ns3/object.h"

#include <string>

namespace ns3
{

/**
 * @ingroup oran
 *
 * Base class for the Reports sent by the nodes to the Near-RT RIC
 * to report on the state of the network.
 *
 * This class can be instantiated but should not be used, as it does
 * not contain useful information.
 */
class OranReport : public Object
{
  public:
    /**
     * Get the TypeId of the OranReport class.
     *
     * @return The TypeId.
     */
    static TypeId GetTypeId();
    /**
     * Constructor of the OranReport class.
     */
    OranReport();
    /**
     * Destructor of the OranReport class.
     */
    ~OranReport() override;
    /**
     * Get a string representation of this Report.
     *
     * @return A string representation of this Report.
     */
    virtual std::string ToString() const;
    /**
     * Get the E2 Node ID of the reporter.
     *
     * @return The E2 Node ID of the reporter.
     */
    uint64_t GetReporterE2NodeId() const;
    /**
     * Get the Time at which the Report was generated.
     *
     * @return The Time at which the Report was generated.
     */
    Time GetTime() const;

  private:
    /**
     * E2 Node ID of the Reporter.
     */
    uint64_t m_reporterE2NodeId;
    /**
     * Time at which the Report was generated.
     */
    Time m_time;
}; // class OranReport

} // namespace ns3

#endif /* ORAN_REPORT_H */
