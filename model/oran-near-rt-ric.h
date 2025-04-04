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
 * FITNESS FOR A PARTICULAR PURPOSE, NEAR-INFRINGEMENT AND DATA ACCURACY. NIST
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

#ifndef ORAN_NEAR_RT_RIC_H
#define ORAN_NEAR_RT_RIC_H

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/random-variable-stream.h"

#include <map>

namespace ns3
{

class OranLm;
class OranCmm;
class OranCommand;
class OranDataRepository;
class OranNearRtRicE2Terminator;
class OranQueryTrigger;
class OranReport;

/**
 * @defgroup oran O-RAN architecture
 */
/**
 * @ingroup oran
 *
 * Model for the Near-RT RIC
 *
 * The Near-RT RAN Intelligent Controller (RIC). This model houses the
 * implementation of the Data Storage Module, Default Logic Module, optional
 * Additional Logic Modules, the Conflict Mitigation Module, and the E2 Terminator.
 *
 * Communication with the E2 Terminators in the nodes is done over virtual
 * interfactes, with the E2 Terminator in the RIC keeping pointers to all the
 * Terminators of registered nodes, and each Node E2 Terminator having a pointer
 * to the RIC they report to.
 *
 * The Near-RT RIC can be activated or deactivated, and this status is propagated
 * to all the components.
 *
 * The RIC is responsible for periodically querying all the Logic Modules
 * deployed, and sending the Commands that they generate to the target nodes.
 *
 * A default Logic Module must always be deployed. This default LM can be replaced
 * dynamically during the simulation, but it cannot be removed without replacement.
 * Similarly, an instance of the Conflict Mitigation Module must always be present.
 *
 * Additional Logic Modules can be added and removed during the simulation.
 */
class OranNearRtRic : public Object
{
  public:
    /**
     * Enumeration with the types of nodes
     */
    enum NodeType
    {
        WIRED = 0, //!< Wired node, identified by simulation Node ID
        LTEUE,     //!< LTE UE, identified by IMSI
        LTEENB     //!< LTE ENB, identified by cell ID
    };

    /**
     * Enumeration with the results of the operation to add an additional Logic
     * Module.
     */
    enum AddLmResult
    {
        ADDLM_OK = 0,         //!< Operation successful
        ADDLM_ERR_NAME_EXISTS //!< LM Name already exists
    };

    /**
     * Enumeration with the results of the operation to remove an additional
     * Logic Module.
     */
    enum RemoveLmResult
    {
        DELLM_OK = 0,          //!< Operation successful
        DELLM_ERR_NAME_INVALID //!< Provided name does not exist
    };

    /**
     * Enumeration with policy types for handling commands that are received
     * from Logic Modules after the maximum wait time.
     */
    enum LateCommandPolicy
    {
        DROP = 0, //!< Drop the commands
        SAVE      //!< Saves the commands for the next cycle
    };

    /**
     * Get the TypeId of the OranNearRtRic class.
     *
     * @return The TypeId.
     */
    static TypeId GetTypeId();
    /**
     * Constructor of the OranNearRtRic class.
     */
    OranNearRtRic();
    /**
     * Destructor of the OranNearRtRic class.
     */
    ~OranNearRtRic() override;
    /**
     * Activate the RIC.
     *
     * This method will propagate and activate the E2 Terminator, the Data
     * Repository, and the Logic Modules.
     */
    void Activate();
    /**
     * Deactivate the RIC.
     *
     * This method will propagate and deactivate the E2 Terminator, the Data
     * Repository, and the Logic Modules.
     */
    void Deactivate();
    /**
     * Get the active status of the RIC.
     *
     * @return True if the RIC is active.
     */
    bool IsActive() const;
    /**
     * Activate and start periodically querying the LMs.
     */
    void Start();
    /**
     * Deactivate and stop periodically querying the LMs.
     */
    void Stop();

    /* Getters and Setters for the modules in the Near-RT RIC */
    /**
     * Get the E2 Terminator.
     *
     * @return The E2 Terminator in the Near-RT RIC.
     */
    Ptr<OranNearRtRicE2Terminator> GetE2Terminator() const;
    /**
     * Get the Default Logic Module.
     *
     * @return The Default Logic Module.
     */
    Ptr<OranLm> GetDefaultLogicModule() const;
    /**
     * Set the Default Logic Module.
     *
     * @param newDefaultLm The Logic Module to set as default.
     */
    void SetDefaultLogicModule(Ptr<OranLm> newDefaultLm);
    /**
     * Get an additional Logic Modules by name. If there is no additional Logic
     * Module with that name, return a null pointer.
     *
     * @param name The name of the additional Logic Module we want to retrieve.
     *
     * @return A pointer to the additional Logic Module with the name provided.
     */
    Ptr<OranLm> GetAdditionalLogicModule(std::string name) const;
    /**
     * Add an additional logic module.
     *
     * @param newLm The new logic module.
     *
     * @return The result of adding the Logic Module
     */
    OranNearRtRic::AddLmResult AddLogicModule(Ptr<OranLm> newLm);
    /**
     * Remove an additional logic module.
     *
     * @param name A string with the name of the logic module to remove
     *
     * @return The result of removing the Logic Module
     */
    OranNearRtRic::RemoveLmResult RemoveLogicModule(std::string name);
    /**
     * Add an LM query trigger.
     *
     * @param name The name of the trigger.
     * @param trigger The trigger to add.
     *
     * @return True, if the trigger was added successfully; otherwise, false.
     */
    bool AddQueryTrigger(std::string name, Ptr<OranQueryTrigger> trigger);
    /**
     * Removes an LM query trigger.
     *
     * @param name The name of the trigger.
     *
     * @return True, if the trigger was removed successfully; otherwise, false.
     */
    bool RemoveQueryTrigger(std::string name);
    /**
     * Get a pointer to the Data Repository instance.
     *
     * @return A pointer to the Data Repository instance.
     */
    Ptr<OranDataRepository> Data() const;
    /**
     * Get the Conflict Mitigation Module.
     *
     * @return The Conflict Mitigation Module.
     */
    Ptr<OranCmm> GetCmm() const;
    /**
     * Set the Conflict Mitigation Module.
     *
     * @param newCmm The Conflict Mitigation Module to use.
     */
    void SetCmm(Ptr<OranCmm> newCmm);
    /**
     * Notifies the Near-RT RIC that a Logic Module has finished running.
     *
     * @param lmQueryCycle The lmQueryCycle that the commands were generated for.
     * @param commands The commands that were generated.
     * @param lm The Logic Module that generated the commands.
     */
    void NotifyLmFinished(Time lmQueryCycle,
                          std::vector<Ptr<OranCommand>> commands,
                          Ptr<OranLm> lm);
    /**
     * Notifies the Near-RT RIC that a report was received.
     *
     * @param report The report that was received.
     */
    void NotifyReportReceived(Ptr<OranReport> report);

  protected:
    /**
     * Dispose of the object.
     */
    virtual void DoDispose() override;

  private:
    /**
     * Function that tells all logic modules to do their calculations, and
     * collects all the commands generated to pass them to the E2 Terminator.
     */
    void QueryLms();
    /**
     * Function that checks for node inactivity.
     */
    void CheckForInactivity();
    /**
     * Processes the commands received for this LM query cycle.
     */
    void ProcessLmQueryCommands();

    /**
     * The E2 Terminator.
     */
    Ptr<OranNearRtRicE2Terminator> m_e2Terminator;
    /**
     * The Data Repository implementation.
     */
    Ptr<OranDataRepository> m_data;
    /**
     * The default Logic Module.
     */
    Ptr<OranLm> m_defaultLm;
    /**
     * The vector of additional Logic Modules, indexed by their names.
     */
    std::map<std::string, Ptr<OranLm>> m_additionalLms;
    /**
     * The Conflict Mitigation Module.
     */
    Ptr<OranCmm> m_cmm;
    /**
     * Flag to keep track of active status.
     */
    bool m_active;
    /**
     * Interval between periodic queries to the Logic Modules.
     */
    Time m_lmQueryInterval;
    /**
     * Event for the scheduling of LM querying.
     */
    EventId m_lmQueryEvent;
    /**
     * The event for scheduling to check for node inactivity.
     */
    EventId m_e2NodeInactivityEvent;
    /**
     * The amount of time that must pass from a node's last registration before determining a node
     * is inactive.
     */
    Time m_e2NodeInactivityThreshold;
    /**
     * The random variable used to periodically schedule checks for node inactivity.
     */
    Ptr<RandomVariableStream> m_e2NodeInactivityIntervalRv;
    /**
     * The current LM query cycle.
     */
    Time m_lmQueryCycle;
    /**
     * The maximum amount of time to wait for a Logic Module to finish running.
     */
    Time m_lmQueryMaxWaitTime;
    /**
     * The event for scheduling when to process LM query commands.
     */
    EventId m_processLmQueryCommandsEvent;
    /**
     * The policy to apply when a late command is received from a Logic Module.
     */
    LateCommandPolicy m_lmQueryLateCommandPolicy;
    /**
     * The collection of LM commands to send at the end of the query cycle.
     */
    std::map<std::tuple<std::string, bool>, std::vector<Ptr<OranCommand>>> m_lmQueryCommands;
    /**
     * The vector of LM query triggers, indexed by their names.
     */
    std::map<std::string, Ptr<OranQueryTrigger>> m_queryTriggers;
}; // class OranNearRtRic

} // namespace ns3

#endif /* ORAN_NEAR_RT_RIC_H */
