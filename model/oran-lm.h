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

#ifndef ORAN_LM_H
#define ORAN_LM_H

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/random-variable-stream.h"

#include <string_view>
#include <vector>

namespace ns3
{

class OranNearRtRic;
class OranCommand;

/**
 * @ingroup oran
 *
 * Base class for the Logic Modules that make up the logic of the Near-RT RIC.
 * This base class provides implementations for common operations, like activation,
 * deactivation, getters and setters, and logging logic traces to the Data Repository.
 *
 * This class cannot be instantiated as it lacks implementation of the Run method.
 */
class OranLm : public Object
{
  public:
    /**
     * Get the TypeId of the OranLm class.
     *
     * @return The TypeId.
     */
    static TypeId GetTypeId();
    /**
     * Create an instance of the OranLm class.
     */
    OranLm();
    /**
     * The destructor of the OranLm class.
     */
    ~OranLm() override;
    /**
     * Activate the Logic Module
     */
    virtual void Activate();
    /**
     * Deactivate the Logic Module.
     *
     * This also clears the vector of stored commands.
     */
    virtual void Deactivate();
    /**
     * Check if the Logic Module is active.
     *
     * @return True, if this Logic Module is active; otherwise, false.
     */
    bool IsActive() const;
    /**
     * Get the name of this Logic Module.
     *
     * @return The Name of this Logic Module.
     */
    std::string GetName() const;
    /**
     * Set the name of this Logic Module.
     *
     * @param name The name of this Logic Module.
     */
    void SetName(std::string_view name);
    /**
     * Prompts this Logical Module to execute its logic and generate any
     * necessary commands.
     *
     * @param cycle The cycle to run for.
     */
    void Run(Time cycle);
    /**
     * Cancels the current run.
     */
    void CancelRun();
    /**
     * Indicates if the LM is running.
     *
     * @return true, if the LM is running; otherwise, false.
     */
    bool IsRunning() const;

  protected:
    /**
     * Dispose of the object.
     */
    void DoDispose() override;
    /**
     * Log a string to the Data Repository
     *
     * @param msg The string to log to the Data Repository
     */
    void LogLogicToRepository(const std::string& msg) const;
    /**
     * Finish running the logic module.
     */
    virtual void FinishRun();
    /**
     * Generates the commands to provide to the Near-RT RIC.
     *
     * @return The generated commands.
     */
    virtual std::vector<Ptr<OranCommand>> Run() = 0;

    /**
     * Pointer to the Near-RT RIC.
     */
    Ptr<OranNearRtRic> m_nearRtRic;
    /**
     * A name to differentiate this Logic Module from others
     */
    std::string m_name;
    /**
     * Flag to indicate if logic should be logged to the Data Repository
     */
    bool m_verbose;
    /**
     * Flag to keep track of the active status
     */
    bool m_active{false};

  private:
    /**
     * The finish run event.
     */
    EventId m_finishRunEvent;
    /**
     * The random variable used to determine the delay (in seconds) it takes to
     * generate commands.
     */
    Ptr<RandomVariableStream> m_processingDelayRv;
    /**
     * The current cycle.
     */
    Time m_cycle;
    /**
     * Commands that were generated.
     */
    std::vector<Ptr<OranCommand>> m_commands;
}; // class OranLm

} // namespace ns3

#endif /* ORAN_LM_H */
