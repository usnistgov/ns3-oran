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

#ifndef ORAN_REPORTER_H
#define ORAN_REPORTER_H

#include "oran-e2-node-terminator.h"

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"

#include <string>

namespace ns3
{

class OranReportTrigger;

/**
 * @ingroup oran
 *
 * Base class for Reporters that attach to simulation nodes and collect
 * specific statistics.
 *
 * This class cannot be instantiated as it does not have an implementatio
 * of the GenerateReports method.
 */
class OranReporter : public Object
{
  public:
    /**
     * Get the TypeId of the OranReporter class.
     *
     * @return The TypeId.
     */
    static TypeId GetTypeId();
    /**
     * Constructor of the OranReporter class.
     */
    OranReporter();
    /**
     * Destructor of the OranReporter class.
     */
    ~OranReporter() override;
    /**
     * Activate the reporter and start periodically generating Reports
     * for the E2 Node Terminator.
     */
    virtual void Activate();
    /**
     * Deactivate the reporter and stop generating Reports.
     */
    virtual void Deactivate();
    /**
     * Indicate if the reporter is active.
     *
     * @return True, if the reporter is active; otherwise, false.
     */
    virtual bool IsActive() const;
    /**
     * Get the E2 Node Terminator.
     * @return The E2 Node Terminator.
     */
    Ptr<OranE2NodeTerminator> GetTerminator() const;
    /**
     * Invoke GenerateReports and send the Reports to the E2 Terminator.
     */
    virtual void PerformReport();
    /**
     * Notifies the reporter that initial registartion has completed successfully.
     */
    virtual void NotifyRegistrationComplete();

  protected:
    /**
     * Dispose of the Reporter.
     */
    void DoDispose() override;
    /**
     * Collect values and metrics from the models, and generate reports
     * with them.
     *
     * @return The collection of Reports.
     */
    virtual std::vector<Ptr<OranReport>> GenerateReports() = 0;

    /**
     * Flag to indicate if the Reporter is active.
     */
    bool m_active;
    /**
     * The E2 Node Terminator.
     */
    Ptr<OranE2NodeTerminator> m_terminator;

  private:
    /**
     * The trigger that generates reports.
     */
    Ptr<OranReportTrigger> m_trigger;
}; // class OranReporter

} // namespace ns3

#endif /* ORAN_REPORTER_H */
