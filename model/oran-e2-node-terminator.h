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

#ifndef ORAN_E2_NODE_TERMINATOR_H
#define ORAN_E2_NODE_TERMINATOR_H

#include "oran-command.h"
#include "oran-near-rt-ric.h"
#include "oran-report.h"

#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/random-variable-stream.h"

namespace ns3
{

class OranReporter;
class OranNearRtRicE2Terminator;

/**
 * @ingroup oran
 *
 * Base class for the E2 Terminators installed in simulation nodes.
 *
 * This class provides the implementation of common methods for all
 * types of terminators (like activation deactivation, adding Reporters, etc.),
 * but cannot be instantiated as it lacks the implementation of the
 * GetNodeType and ReceiveCommand methods.
 */
class OranE2NodeTerminator : public Object
{
  public:
    /**
     * Gets the TypeId of the OranE2NodeTerminator class.
     *
     * @returns The TypeId.
     */
    static TypeId GetTypeId();
    /**
     * Creates an instance of the OranE2NodeTerminator class.
     */
    OranE2NodeTerminator();
    /**
     * The destructor of the OranE2NodeTerminator class.
     */
    ~OranE2NodeTerminator() override;
    /**
     * Activate the E2 Terminator. This method will activate
     * all the Reporters associated with this Terminator and will start
     * a periodic registration.
     */
    virtual void Activate();
    /**
     * Add a Reporter to the Terminator.
     *
     * @param reporter The Reporter to add.
     */
    virtual void AddReporter(Ptr<OranReporter> reporter);
    /**
     * Attach this Terminator to a node.
     *
     * @param node The node to attach to.
     * @param netDeviceIndex The index of the net device.
     */
    virtual void Attach(Ptr<Node> node, uint32_t netDeviceIndex = 0);
    /**
     * Deactivate the Reporter. This method will deactivate
     * all the Reporters associated with this Terminator
     */
    virtual void Deactivate();
    /**
     * Get the E2 Node type that this Terminator is attached to. This
     * method will also stop the periodic registration and report
     * transmission.
     *
     * @returns the E2 Node type.
     */
    virtual OranNearRtRic::NodeType GetNodeType() const = 0;
    /**
     * Indicate if this Terminator is active.
     *
     * @returns True, if the Terminator is active; otherwise, false.
     */
    virtual bool IsActive() const;
    /**
     * Store a Report generated by a Reporter.
     *
     * @param report The Report to store.
     */
    virtual void StoreReport(Ptr<OranReport> report);
    /**
     * Receive and process a Command.
     *
     * @param command The Command to receive.
     */
    virtual void ReceiveCommand(Ptr<OranCommand> command) = 0;
    /**
     * Receive a deregistration response. This cancels the event for sending
     * another registration request.
     *
     * @param e2NodeId A valid E2 Node ID or 0 to indicate a failed deregistration request.
     */
    virtual void ReceiveDeregistrationResponse(uint64_t e2NodeId);
    /**
     * Receive a registration response. This schedules periodic sending
     * of registration requests.
     *
     * @param e2NodeId A valid E2 Node ID or 0 to indicate a failed registration request.
     */
    virtual void ReceiveRegistrationResponse(uint64_t e2NodeId);

  protected:
    /**
     * Cancel the next registration request event. This will stop the
     * periodic registration.
     */
    virtual void CancelNextRegistration();
    /**
     * Cancel the next send event.
     */
    virtual void CancelNextSend();
    /**
     * Deregister the node.
     */
    virtual void Deregister();
    /**
     * Dispose of the object.
     */
    void DoDispose() override;
    /**
     * Send the Reports.
     */
    virtual void DoSendReports();
    /**
     * Register the node.
     */
    virtual void Register();
    /**
     * Schedule the next send event.
     */
    virtual void ScheduleNextSend();

    /**
     * Indicate if the Reporter is active.
     */
    bool m_active;

  private:
    /**
     * The E2 Node ID.
     */
    uint64_t m_e2NodeId;
    /**
     * The name of the E2 Node Terminator.
     */
    std::string m_name;
    /**
     * The node that this terminator is attached to.
     */
    Ptr<Node> m_node;
    /**
     * The index of the net device that this terminator is attached to.
     */
    uint32_t m_netDeviceIndex;
    /**
     * The Near RT-RIC.
     */
    Ptr<OranNearRtRic> m_nearRtRic;
    /**
     * The collection of Reports to send.
     */
    std::vector<Ptr<OranReport>> m_reports;
    /**
     * The collection of Reporters.
     */
    std::vector<Ptr<OranReporter>> m_reporters;
    /**
     * The event with the next registration request.
     */
    EventId m_registrationEvent;
    /**
     * The random variable used to periodically send registration requests.
     */
    Ptr<RandomVariableStream> m_registrationIntervalRv;
    /**
     * The event with the next Report transmission.
     */
    EventId m_sendEvent;
    /**
     * The random variable used to send Reports to the Near-RT RIC.
     */
    Ptr<RandomVariableStream> m_sendIntervalRv;
    /**
     * The random variable used to to determine the transmission delay of a report.
     */
    Ptr<RandomVariableStream> m_transmissionDelayRv;

  public:
    /**
     * Get the E2 Node ID of the terminator.
     *
     * @return The E2 Node ID.
     */
    uint64_t GetE2NodeId() const;
    /**
     * Get the Near-RT RIC.
     *
     * @return The Near-RT RIC.
     */
    Ptr<OranNearRtRic> GetNearRtRic() const;
    /**
     * Get the node that this terminator is attached to.
     *
     * @return The node.
     */
    Ptr<Node> GetNode() const;
    /**
     * Get the index of the network device.
     *
     * @return The index.
     */
    uint32_t GetNetDeviceIndex() const;
}; // class OranE2NodeTerminator

} // namespace ns3

#endif /* ORAN_E2_NODE_TERMINATOR_H */
