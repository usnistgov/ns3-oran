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

#ifndef ORAN_NEAR_RT_RIC_E2TERMINATOR
#define ORAN_NEAR_RT_RIC_E2TERMINATOR

#include "oran-command.h"
#include "oran-data-repository.h"
#include "oran-report.h"

#include "ns3/object.h"
#include "ns3/random-variable-stream.h"

#include <map>
#include <vector>

namespace ns3
{

class OranNearRtRic;
class OranE2NodeTerminator;

/**
 * @ingroup oran
 *
 * The E2 Terminator of the Near-RT RIC.
 */
class OranNearRtRicE2Terminator : public Object
{
  public:
    /**
     * Get the TypeId of the OranNearRtRicE2 class.
     *
     * @return The TypeId.
     */
    static TypeId GetTypeId();
    /**
     * Constructor of the OranNearRtRicE2Terminator class.
     */
    OranNearRtRicE2Terminator();
    /**
     * Destructor of the OranNearRtRicE2Terminator class.
     */
    ~OranNearRtRicE2Terminator() override;
    /**
     * Activate the E2 Terminator.
     */
    virtual void Activate();
    /**
     * Deactivate the E2 Terminator.
     */
    virtual void Deactivate();
    /**
     * Get the active status of this E2 Terminator.
     *
     * @return True if this E2 Terminator is active; otherwise, false.
     */
    bool IsActive() const;
    /**
     * Receive a registration request. This method stores the Node type and ID
     * in the Data Repository (which returns the assigned E2 Node ID), adds the
     * node E2 Terminator to the map (so that it knows where to send future
     * commands), and 'sends' the registration response.
     *
     * @param type The type of node
     * @param id The unique ID for the node
     * @param terminator the Node E2 Terminator
     */
    void ReceiveRegistrationRequest(OranNearRtRic::NodeType type,
                                    uint64_t id,
                                    Ptr<OranE2NodeTerminator> terminator);
    /**
     * Receive a deregistration request. This method logs the deregistration
     * in the data repository, and removes the node E2 Terminator from the map.
     *
     * @param e2NodeId The E2NodeId to deregister.
     */
    void ReceiveDeregistrationRequest(uint64_t e2NodeId);
    /**
     * Receive a Report and log the report in the Data Repository.
     *
     * @param report The Report from the Reporter.
     */
    void ReceiveReport(Ptr<OranReport> report);
    /**
     * Send a Command to an E2 Node Terminator. The Command will be transmitted
     * directly to the target Terminator using the map of registered Terminators
     * with the Target E2 Node indicated in the Command.
     *
     * @param command The Command to send.
     */
    void SendCommand(Ptr<OranCommand> command);
    /**
     * Send multiple commands to the corresponding E2 Node Terminators.
     *
     * @param commands A vector with the Commands to send.
     */
    void ProcessCommands(std::vector<Ptr<OranCommand>> commands);

  protected:
    /**
     * Dispose of the object.
     */
    void DoDispose() override;

  private:
    /**
     * Flag to keep track of active status
     */
    bool m_active;
    /**
     * Pointer to the Near-RT RIC.
     */
    Ptr<OranNearRtRic> m_nearRtRic;
    /**
     * Pointer to the Data Repository.
     */
    Ptr<OranDataRepository> m_data;
    /**
     * Map with the active Node E2 Terminators, indexed by E2 Node IDs
     */
    std::map<uint64_t, Ptr<OranE2NodeTerminator>> m_nodeTerminators;
    /**
     * The random variable used to to determine the transmission delay of a command.
     */
    Ptr<RandomVariableStream> m_transmissionDelayRv;
}; // class  OranNearRtRicE2Terminator

} // namespace ns3

#endif // ORAN_NEAR_RT_RIC_E2TERMINATOR
