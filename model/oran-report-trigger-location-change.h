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

#ifndef ORAN_REPORT_TRIGGER_LOCATION_CHANGE_H
#define ORAN_REPORT_TRIGGER_LOCATION_CHANGE_H

#include "oran-report-trigger.h"

#include "ns3/mobility-model.h"
#include "ns3/object.h"
#include "ns3/ptr.h"

#include <string>

namespace ns3
{

class OranReporter;

/**
 * @ingroup oran
 *
 * A class that triggers reports based on position changes
 * UE.
 */
class OranReportTriggerLocationChange : public OranReportTrigger
{
  public:
    /**
     * Get the TypeId of the OranReportTriggerLocationChange class.
     *
     * @return The TypeId.
     */
    static TypeId GetTypeId();
    /**
     * Constructor of the OranReportTriggerLocationChange class.
     */
    OranReportTriggerLocationChange();
    /**
     * Destructor of the OranReportTriggerLocationChange class.
     */
    ~OranReportTriggerLocationChange() override;
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
     * The callback for receiving the HandoverEndOk event from an LTE UE.
     * @param mobility
     */
    virtual void CourseChangedSink(Ptr<const MobilityModel> mobility);

  private:
    /**
     * Disconnects the callback from the LTE UE.
     */
    void DisconnectSink();
}; // class OranReportTriggerLocationChange

} // namespace ns3

#endif /* ORAN_REPORT_TRIGGER_LOCATION_CHANGE_H */
