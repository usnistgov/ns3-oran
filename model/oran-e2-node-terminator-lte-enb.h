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

#ifndef ORAN_E2_NODE_TERMINATOR_LTE_ENB_H
#define ORAN_E2_NODE_TERMINATOR_LTE_ENB_H

#include "oran-e2-node-terminator.h"

#include "ns3/lte-enb-net-device.h"

namespace ns3
{

/**
 * @ingroup oran
 *
 * E2 Node Terminator for LTE eNBs. This Terminator can process LTE Handover
 * Commands
 */
class OranE2NodeTerminatorLteEnb : public OranE2NodeTerminator
{
  public:
    /**
     * Get the TypeId of the OranE2NodeTerminatorLteEnb class.
     *
     * @return The TypeId.
     */
    static TypeId GetTypeId();
    /**
     * Create an instance of the OranE2NodeTerminatorLteEnb class.
     */
    OranE2NodeTerminatorLteEnb();
    /**
     * The destructor of the OranE2NodeTerminatorLteEnb class.
     */
    ~OranE2NodeTerminatorLteEnb() override;
    /**
     * Get the E2 Node Type. For this Terminator, this method
     * always returns the LTE eNB type.
     *
     * @return the E2 Node Type.
     */
    OranNearRtRic::NodeType GetNodeType() const override;
    /**
     * Receive and process a command. If the Command is an LTE Handover Command
     * it will be processed. All other types of Commands are silently discarded..
     *
     * @param command The received command.
     */
    void ReceiveCommand(Ptr<OranCommand> command) override;
    /**
     * Get the NetDevice of the LTE eNB.
     *
     * @return The net device.
     */
    virtual Ptr<LteEnbNetDevice> GetNetDevice() const;
}; // class OranE2NodeTermiantorLteEnb

} // namespace ns3

#endif /* ORAN_E2_NODE_TERMINATOR_LTE_ENB_H */
