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

#ifndef ORAN_REPORT_TRIGGER_H
#define ORAN_REPORT_TRIGGER_H

#include "ns3/object.h"
#include "ns3/ptr.h"

#include <string>

namespace ns3
{

class OranReporter;

/**
 * @ingroup oran
 *
 * Base class for triggering the generation of a Report.
 *
 * This class cannot be instantiated.
 */
class OranReportTrigger : public Object
{
  public:
    /**
     * Get the TypeId of the OranReportTrigger class.
     *
     * @return The TypeId.
     */
    static TypeId GetTypeId();
    /**
     * Destructor of the OranReportTrigger class.
     */
    ~OranReportTrigger() override;
    /**
     * Activates this trigger for and links it to the given reporter.
     * @param reporter The reporter to link to.
     */
    virtual void Activate(Ptr<OranReporter> reporter);
    /**
     * Deactivates this trigger and unlinks it from the current reporter.
     */
    virtual void Deactivate();
    /**
     * Notifies the trigger that initial registartion has completed successfully.
     */
    virtual void NotifyRegistrationComplete();

  protected:
    /**
     * Constructor of the OranReportTrigger class.
     */
    OranReportTrigger();
    /**
     * Dispose of the Report.
     */
    void DoDispose() override;
    /**
     * Triggers a report.
     */
    virtual void TriggerReport();

    /**
     * Flag to indicate if the trigger is active.
     */
    bool m_active;
    /**
     * The reporter to trigger reports for.
     */
    Ptr<OranReporter> m_reporter;

  private:
    /**
     * Indicates if an initial report should be triggered upon registration.
     */
    bool m_initialReport;
}; // class OranReportTrigger

} // namespace ns3

#endif /* ORAN_REPORT_TRIGGER_H */
