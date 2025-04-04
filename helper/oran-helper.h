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

#ifndef ORAN_HELPER_H
#define ORAN_HELPER_H

#include "ns3/node-container.h"
#include "ns3/nstime.h"
#include "ns3/object-factory.h"
#include "ns3/object.h"
#include "ns3/oran-e2-node-terminator-container.h"
#include "ns3/oran-near-rt-ric.h"

namespace ns3
{

/**
 * @ingroup oran
 *
 * Helper to facilitate the configuration and deployment of O-RAN modules
 */
class OranHelper : public Object
{
  public:
    /**
     * Gets the TypeId of the OranHelper.
     *
     * @return The TypeId.
     */
    static TypeId GetTypeId();
    /**
     * Creates an instance of the OranHelper class.
     */
    OranHelper();
    /**
     * The destructor of the OranHelper class.
     */
    ~OranHelper() override;
    /**
     * Adds an additional Logic Module to the RIC. This method takes a string
     * with the class name of the LM to deploy, and pairs of attribute name and
     * value to configure the LM. This method does not instantiate the LM.
     * Instead, it configures an ObjectFactory with the received parameters, and
     * stores this factory in the m_lmFactories vector so the CreateNearRtRic
     * method (which should be invoked later) instantiates the LM and associates
     * it with the Near-RT RIC.
     *
     * @param tid the string representation of the ns3::TypeId associated with the model to set.
     * @param n0 the name of the attribute to set.
     * @param v0 the value of the attribute to set.
     * @param n1 the name of the attribute to set.
     * @param v1 the value of the attribute to set.
     * @param n2 the name of the attribute to set.
     * @param v2 the value of the attribute to set.
     * @param n3 the name of the attribute to set.
     * @param v3 the value of the attribute to set.
     * @param n4 the name of the attribute to set.
     * @param v4 the value of the attribute to set.
     * @param n5 the name of the attribute to set.
     * @param v5 the value of the attribute to set.
     * @param n6 the name of the attribute to set.
     * @param v6 the value of the attribute to set.
     * @param n7 the name of the attribute to set.
     * @param v7 the value of the attribute to set.
     */
    void AddLogicModule(std::string tid,
                        std::string n0 = "",
                        const AttributeValue& v0 = EmptyAttributeValue(),
                        std::string n1 = "",
                        const AttributeValue& v1 = EmptyAttributeValue(),
                        std::string n2 = "",
                        const AttributeValue& v2 = EmptyAttributeValue(),
                        std::string n3 = "",
                        const AttributeValue& v3 = EmptyAttributeValue(),
                        std::string n4 = "",
                        const AttributeValue& v4 = EmptyAttributeValue(),
                        std::string n5 = "",
                        const AttributeValue& v5 = EmptyAttributeValue(),
                        std::string n6 = "",
                        const AttributeValue& v6 = EmptyAttributeValue(),
                        std::string n7 = "",
                        const AttributeValue& v7 = EmptyAttributeValue());
    /**
     * Adds an additional Reporter to the Node E2 Terminator. This method takes
     * a string with the class name of the Reporter to deploy, and pairs of
     * attribute name and value to configure the Reporter. This method does not
     * instantiate the Reporter. Instead, it configures an ObjectFactory with the
     * received parameters, and stores this factory in the m_reporterFactories
     * vector so the DeployTerminators method (which should be invoked later)
     * instantiates one Reporter of this type with this configuration for each
     * Terminator deployed.
     *
     * @param tid the string representation of the ns3::TypeId associated with the model to set.
     * @param n0 the name of the attribute to set.
     * @param v0 the value of the attribute to set.
     * @param n1 the name of the attribute to set.
     * @param v1 the value of the attribute to set.
     * @param n2 the name of the attribute to set.
     * @param v2 the value of the attribute to set.
     * @param n3 the name of the attribute to set.
     * @param v3 the value of the attribute to set.
     * @param n4 the name of the attribute to set.
     * @param v4 the value of the attribute to set.
     * @param n5 the name of the attribute to set.
     * @param v5 the value of the attribute to set.
     * @param n6 the name of the attribute to set.
     * @param v6 the value of the attribute to set.
     * @param n7 the name of the attribute to set.
     * @param v7 the value of the attribute to set.
     */
    void AddReporter(std::string tid,
                     std::string n0 = "",
                     const AttributeValue& v0 = EmptyAttributeValue(),
                     std::string n1 = "",
                     const AttributeValue& v1 = EmptyAttributeValue(),
                     std::string n2 = "",
                     const AttributeValue& v2 = EmptyAttributeValue(),
                     std::string n3 = "",
                     const AttributeValue& v3 = EmptyAttributeValue(),
                     std::string n4 = "",
                     const AttributeValue& v4 = EmptyAttributeValue(),
                     std::string n5 = "",
                     const AttributeValue& v5 = EmptyAttributeValue(),
                     std::string n6 = "",
                     const AttributeValue& v6 = EmptyAttributeValue(),
                     std::string n7 = "",
                     const AttributeValue& v7 = EmptyAttributeValue());
    /**
     * Adds an LM query trigger to the Near-RT RIC. This method takes a string
     * with the class name of the trigger to deploy, the name of the query
     * trigger, and pairs of attribute name and value to configure the trigger.
     * This method does not instantiate the trigger. Instead, it configures an
     * ObjectFactory with the received parameters, and stores this factory in
     * the m_queryTriggerFactories vector so the AddQueryTriggers method (which
     * should be invoked later) instantiates one trigger of this type with this
     * configuration.
     *
     * @param name The name of the query trigger.
     * @param tid the string representation of the ns3::TypeId associated with the model to set.
     * @param n0 the name of the attribute to set.
     * @param v0 the value of the attribute to set.
     * @param n1 the name of the attribute to set.
     * @param v1 the value of the attribute to set.
     * @param n2 the name of the attribute to set.
     * @param v2 the value of the attribute to set.
     * @param n3 the name of the attribute to set.
     * @param v3 the value of the attribute to set.
     * @param n4 the name of the attribute to set.
     * @param v4 the value of the attribute to set.
     * @param n5 the name of the attribute to set.
     * @param v5 the value of the attribute to set.
     * @param n6 the name of the attribute to set.
     * @param v6 the value of the attribute to set.
     * @param n7 the name of the attribute to set.
     * @param v7 the value of the attribute to set.
     */
    void AddQueryTrigger(std::string name,
                         std::string tid,
                         std::string n0 = "",
                         const AttributeValue& v0 = EmptyAttributeValue(),
                         std::string n1 = "",
                         const AttributeValue& v1 = EmptyAttributeValue(),
                         std::string n2 = "",
                         const AttributeValue& v2 = EmptyAttributeValue(),
                         std::string n3 = "",
                         const AttributeValue& v3 = EmptyAttributeValue(),
                         std::string n4 = "",
                         const AttributeValue& v4 = EmptyAttributeValue(),
                         std::string n5 = "",
                         const AttributeValue& v5 = EmptyAttributeValue(),
                         std::string n6 = "",
                         const AttributeValue& v6 = EmptyAttributeValue(),
                         std::string n7 = "",
                         const AttributeValue& v7 = EmptyAttributeValue());
    /**
     * Configures the Conflict Mitigation module to deploy in the RIC. This
     * method takes an string with the class name of the CMM to deploy, and pairs
     * of attribute name and value to configure the CMM. This method does not
     * instantiate the CMM. Instead, it configures an ObjectFactory with the
     * received parameters so the CreateNearRtRic method (which should be invoked
     * later) instantiates the CMM and associates it with the Near-RT RIC.
     *
     * @param tid the string representation of the ns3::TypeId associated with the model to set.
     * @param n0 the name of the attribute to set.
     * @param v0 the value of the attribute to set.
     * @param n1 the name of the attribute to set.
     * @param v1 the value of the attribute to set.
     * @param n2 the name of the attribute to set.
     * @param v2 the value of the attribute to set.
     * @param n3 the name of the attribute to set.
     * @param v3 the value of the attribute to set.
     * @param n4 the name of the attribute to set.
     * @param v4 the value of the attribute to set.
     * @param n5 the name of the attribute to set.
     * @param v5 the value of the attribute to set.
     * @param n6 the name of the attribute to set.
     * @param v6 the value of the attribute to set.
     * @param n7 the name of the attribute to set.
     * @param v7 the value of the attribute to set.
     */
    void SetConflictMitigationModule(std::string tid,
                                     std::string n0 = "",
                                     const AttributeValue& v0 = EmptyAttributeValue(),
                                     std::string n1 = "",
                                     const AttributeValue& v1 = EmptyAttributeValue(),
                                     std::string n2 = "",
                                     const AttributeValue& v2 = EmptyAttributeValue(),
                                     std::string n3 = "",
                                     const AttributeValue& v3 = EmptyAttributeValue(),
                                     std::string n4 = "",
                                     const AttributeValue& v4 = EmptyAttributeValue(),
                                     std::string n5 = "",
                                     const AttributeValue& v5 = EmptyAttributeValue(),
                                     std::string n6 = "",
                                     const AttributeValue& v6 = EmptyAttributeValue(),
                                     std::string n7 = "",
                                     const AttributeValue& v7 = EmptyAttributeValue());
    /**
     * Configures the Data Repository module to deploy in the RIC. This method
     * takes an string with the class name of the DR to deploy, and pairs of
     * attribute name and value to configure the DR. This method does not
     * instantiate the DR. Instead, it configures an ObjectFactory with the
     * received parameters so the CreateNearRtRic method (which should be invoked
     * later) instantiates the DR and associates it with the Near-RT RIC.
     *
     * @param tid the string representation of the ns3::TypeId associated with the model to set.
     * @param n0 the name of the attribute to set.
     * @param v0 the value of the attribute to set.
     * @param n1 the name of the attribute to set.
     * @param v1 the value of the attribute to set.
     * @param n2 the name of the attribute to set.
     * @param v2 the value of the attribute to set.
     * @param n3 the name of the attribute to set.
     * @param v3 the value of the attribute to set.
     * @param n4 the name of the attribute to set.
     * @param v4 the value of the attribute to set.
     * @param n5 the name of the attribute to set.
     * @param v5 the value of the attribute to set.
     * @param n6 the name of the attribute to set.
     * @param v6 the value of the attribute to set.
     * @param n7 the name of the attribute to set.
     * @param v7 the value of the attribute to set.
     */
    void SetDataRepository(std::string tid,
                           std::string n0 = "",
                           const AttributeValue& v0 = EmptyAttributeValue(),
                           std::string n1 = "",
                           const AttributeValue& v1 = EmptyAttributeValue(),
                           std::string n2 = "",
                           const AttributeValue& v2 = EmptyAttributeValue(),
                           std::string n3 = "",
                           const AttributeValue& v3 = EmptyAttributeValue(),
                           std::string n4 = "",
                           const AttributeValue& v4 = EmptyAttributeValue(),
                           std::string n5 = "",
                           const AttributeValue& v5 = EmptyAttributeValue(),
                           std::string n6 = "",
                           const AttributeValue& v6 = EmptyAttributeValue(),
                           std::string n7 = "",
                           const AttributeValue& v7 = EmptyAttributeValue());
    /**
     * Configures the Default Logic Module to deploy in the RIC. This method takes
     * an string with the class name of the LM to deploy as default, and pairs of
     * attribute name and value to configure the LM. This method does not
     * instantiate the default LM. Instead, it configures an ObjectFactory with the
     * received parameters so the CreateNearRtRic method (which should be invoked
     * later) instantiates the LM and associates it with the Near-RT RIC.
     *
     * @param tid the string representation of the ns3::TypeId associated with the model to set.
     * @param n0 the name of the attribute to set.
     * @param v0 the value of the attribute to set.
     * @param n1 the name of the attribute to set.
     * @param v1 the value of the attribute to set.
     * @param n2 the name of the attribute to set.
     * @param v2 the value of the attribute to set.
     * @param n3 the name of the attribute to set.
     * @param v3 the value of the attribute to set.
     * @param n4 the name of the attribute to set.
     * @param v4 the value of the attribute to set.
     * @param n5 the name of the attribute to set.
     * @param v5 the value of the attribute to set.
     * @param n6 the name of the attribute to set.
     * @param v6 the value of the attribute to set.
     * @param n7 the name of the attribute to set.
     * @param v7 the value of the attribute to set.
     */
    void SetDefaultLogicModule(std::string tid,
                               std::string n0 = "",
                               const AttributeValue& v0 = EmptyAttributeValue(),
                               std::string n1 = "",
                               const AttributeValue& v1 = EmptyAttributeValue(),
                               std::string n2 = "",
                               const AttributeValue& v2 = EmptyAttributeValue(),
                               std::string n3 = "",
                               const AttributeValue& v3 = EmptyAttributeValue(),
                               std::string n4 = "",
                               const AttributeValue& v4 = EmptyAttributeValue(),
                               std::string n5 = "",
                               const AttributeValue& v5 = EmptyAttributeValue(),
                               std::string n6 = "",
                               const AttributeValue& v6 = EmptyAttributeValue(),
                               std::string n7 = "",
                               const AttributeValue& v7 = EmptyAttributeValue());
    /**
     * Stores the type and configuration of the next E2 Node Terminators to
     * deploy. This method takes an string with the class name of the E2 Node
     * Terminator, and pairs of attribute name and value to configure it. This
     * method does not instantiate the E2 Node Terminator. Instead, it saves the
     * configuration in an ObjectFactory so the next time the method
     * DeployTerminators is invoked, the factory will be used to instantiate the
     * E2 Node Terminators.
     *
     * @param tid the string representation of the ns3::TypeId associated with the model to set.
     * @param n0 the name of the attribute to set.
     * @param v0 the value of the attribute to set.
     * @param n1 the name of the attribute to set.
     * @param v1 the value of the attribute to set.
     * @param n2 the name of the attribute to set.
     * @param v2 the value of the attribute to set.
     * @param n3 the name of the attribute to set.
     * @param v3 the value of the attribute to set.
     * @param n4 the name of the attribute to set.
     * @param v4 the value of the attribute to set.
     * @param n5 the name of the attribute to set.
     * @param v5 the value of the attribute to set.
     * @param n6 the name of the attribute to set.
     * @param v6 the value of the attribute to set.
     * @param n7 the name of the attribute to set.
     * @param v7 the value of the attribute to set.
     */
    void SetE2NodeTerminator(std::string tid,
                             std::string n0 = "",
                             const AttributeValue& v0 = EmptyAttributeValue(),
                             std::string n1 = "",
                             const AttributeValue& v1 = EmptyAttributeValue(),
                             std::string n2 = "",
                             const AttributeValue& v2 = EmptyAttributeValue(),
                             std::string n3 = "",
                             const AttributeValue& v3 = EmptyAttributeValue(),
                             std::string n4 = "",
                             const AttributeValue& v4 = EmptyAttributeValue(),
                             std::string n5 = "",
                             const AttributeValue& v5 = EmptyAttributeValue(),
                             std::string n6 = "",
                             const AttributeValue& v6 = EmptyAttributeValue(),
                             std::string n7 = "",
                             const AttributeValue& v7 = EmptyAttributeValue());
    /**
     * Creates the Near-RT RIC.
     * This method not only instantiates an OranNearRtRic object, but it also
     * instantiates the configured Data Repository, Default Logic Module,
     * Additional Logic Modules, Conflict Mitigation Module, and E2 RIC
     * Terminator. These components can be configured by calling the appropriate
     * Set*Module methods before invoking this method.
     *
     * @return A pointer to the Near-RT RIC created.
     */
    Ptr<OranNearRtRic> CreateNearRtRic();
    /**
     * Activates and Starts a Near-RT RIC.
     *
     * @param nearRtRic The Near-RT RIC to activate and start.
     */
    void ActivateAndStartNearRtRic(Ptr<OranNearRtRic> nearRtRic);
    /**
     * Deactivates and Stops the Near-RT RIC.
     *
     * @param nearRtRic The Near-RT RIC to deactivate and stop.
     */
    void DeactivateAndStopNearRtRic(Ptr<OranNearRtRic> nearRtRic);
    /**
     * Deploys E2 Node Terminators and Reporters in all the nodes in a container.
     * This method will instantiate an E2 Node Terminator (of the type configured
     * by calling SetE2NodeTerminator) and all the Reporters configured earlier (
     * by calling the AddReporter method) into each of the nodes in the provided
     * container.
     *
     * After this method is complete, the list of Reporters to instantiate with the
     * next E2 Node Terminator is empied.
     *
     * @param nearRtRic The Near-RT RIC to associate termintors with.
     * @param nodes The nodes to deploy terminators on.
     * @param netDeviceIndex The index of the net devices to attach to.
     *
     * @return The collection of created terminators.
     */
    OranE2NodeTerminatorContainer DeployTerminators(Ptr<OranNearRtRic> nearRtRic,
                                                    NodeContainer nodes,
                                                    uint32_t netDeviceIndex = 0);
    /**
     * Activates all the E2 Node Terminators in a container.
     *
     * @param e2NodeTerminators The terminators.
     */
    void ActivateE2NodeTerminators(OranE2NodeTerminatorContainer e2NodeTerminators);
    /**
     * Deactivates all the E2 Node Terminators in a container.
     *
     * @param e2NodeTerminators The terminators.
     */
    void DeactivateE2NodeTerminators(OranE2NodeTerminatorContainer e2NodeTerminators);

  protected:
    /**
     * Disposes of the OranHelper.
     */
    virtual void DoDispose() override;

  private:
    /**
     * The factory used to create the Conflict Mitigation Module.
     */
    ObjectFactory m_cmmFactory;
    /**
     * The factory used to create the Data Repository.
     */
    ObjectFactory m_dataRepositoryFactory;
    /**
     * The factory used to create the Default Logic Module.
     */
    ObjectFactory m_defaultLmFactory;
    /**
     * The factory used to create the E2 Node Terminators.
     */
    ObjectFactory m_e2NodeTerminatorFactory;
    /**
     * The factories used to create Additional Logic Modules.
     */
    std::vector<ObjectFactory> m_lmFactories;
    /**
     * The object factories used to create Reporters.
     */
    std::vector<ObjectFactory> m_reporterFactories;
    /**
     * The object factories used to create LM query triggers.
     */
    std::map<std::string, ObjectFactory> m_queryTriggerFactories;
    /**
     * Indicates if components should be verbose.
     */
    bool m_verbose;
    /**
     * The interval at which the Near-RT RIC should query the Logic Modules.
     */
    Time m_ricLmQueryInterval;
    /**
     * The inactivity threshold for E2 Nodes.
     */
    Time m_e2NodeInactivityThreshold;
    /**
     * The random variable used to periodically schedule checks for E2 node inactivity.
     */
    Ptr<RandomVariableStream> m_e2NodeInactivityIntervalRv;
    /**
     * The maximum amount of time to wait for a Logic Module to finish running.
     */
    Time m_ricLmQueryMaxWaitTime;
    /**
     * The policy to apply when a late command is received from a Logic Module.
     */
    OranNearRtRic::LateCommandPolicy m_ricLmQueryLateCommandPolicy;
    /**
     * The random variable used (in seconds) to calculate the transmission delay for a command.
     */
    Ptr<RandomVariableStream> m_ricTransmissionDelayRv;
}; // class OranHelper

} // namespace ns3

#endif // ORAN_HELPER_H
