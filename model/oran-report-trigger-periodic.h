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

#ifndef ORAN_REPORT_TRIGGER_PERIODIC_H
#define ORAN_REPORT_TRIGGER_PERIODIC_H

#include "oran-report-trigger.h"

#include "ns3/event-id.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"

#include <string>

namespace ns3
{

class OranReporter;

/**
 * @ingroup oran
 *
 * A class that periodically triggers reports based on a random variable.
 */
class OranReportTriggerPeriodic : public OranReportTrigger
{
  public:
    /**
     * Get the TypeId of the OranReportTriggerPeriodic class.
     *
     * @return The TypeId.
     */
    static TypeId GetTypeId();
    /**
     * Constructor of the OranReportTriggerPeriodic class.
     */
    OranReportTriggerPeriodic();
    /**
     * Destructor of the OranReportTriggerPeriodic class.
     */
    ~OranReportTriggerPeriodic() override;
    /**
     * Activates this trigger for the given reporter.
     * @param reporter The reporter to link to.
     */
    void Activate(Ptr<OranReporter> reporter) override;
    /**
     * Deactivates this trigger and unlinks it from the current reporter.
     */
    void Deactivate() override;

  protected:
    /**
     * Dispose of the Report.
     */
    void DoDispose() override;
    /**
     * Triggers a report.
     */
    void TriggerReport() override;
    /**
     * Cancel the next trigger event.
     */
    virtual void CancelNextTrigger();
    /**
     * Schedule the next trigger event.
     */
    virtual void ScheduleNextTrigger();

  private:
    /**
     * The next trigger event.
     */
    EventId m_triggerEvent;
    /**
     * The random variable used to generate the delay (in seconds) between
     * triggered reports.
     */
    Ptr<RandomVariableStream> m_intervalRv;
}; // class OranReportTriggerPeriodic

} // namespace ns3

#endif /* ORAN_REPORT_TRIGGER_PERIODIC_H */
