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

#include "oran-cmm-single-command-per-node.h"

#include "oran-command-lte-2-lte-handover.h"
#include "oran-command.h"
#include "oran-data-repository.h"
#include "oran-near-rt-ric.h"

#include "ns3/abort.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranCmmSingleCommandPerNode");

NS_OBJECT_ENSURE_REGISTERED(OranCmmSingleCommandPerNode);

TypeId
OranCmmSingleCommandPerNode::GetTypeId()
{
    static TypeId tid = TypeId("ns3::OranCmmSingleCommandPerNode")
                            .SetParent<OranCmm>()
                            .AddConstructor<OranCmmSingleCommandPerNode>();

    return tid;
}

OranCmmSingleCommandPerNode::OranCmmSingleCommandPerNode()
    : OranCmm()
{
    NS_LOG_FUNCTION(this);

    m_name = "CmmSingleCommandPerNode";
}

OranCmmSingleCommandPerNode::~OranCmmSingleCommandPerNode()
{
    NS_LOG_FUNCTION(this);
}

void
OranCmmSingleCommandPerNode::DoDispose()
{
    NS_LOG_FUNCTION(this);

    OranCmm::DoDispose();
}

std::vector<Ptr<OranCommand>>
OranCmmSingleCommandPerNode::Filter(
    std::map<std::tuple<std::string, bool>, std::vector<Ptr<OranCommand>>> inputCommands)
{
    NS_LOG_FUNCTION(this);

    NS_ABORT_MSG_IF(m_nearRtRic == nullptr,
                    "Attempting to run Conflict Mitigation Module with NULL Near-RT RIC");

    std::vector<Ptr<OranCommand>> commands;

    if (m_active)
    {
        std::map<uint64_t, bool> affectedNodes;
        std::map<uint64_t, Ptr<OranCommand>> selectedCommands;
        for (auto commandSet : inputCommands)
        {
            bool defaultLm = std::get<1>(commandSet.first);
            uint64_t affectedNodeId;

            LogLogicToStorage("Checking commands from LM " + std::get<0>(commandSet.first));
            for (auto command : commandSet.second)
            {
                // Get the affected node E2 Node Id depending on the command type
                if (command->GetInstanceTypeId() == OranCommandLte2LteHandover::GetTypeId())
                {
                    uint16_t cellId;
                    bool found;
                    std::tie(found, cellId) =
                        m_nearRtRic->Data()->GetLteEnbCellInfo(command->GetTargetE2NodeId());
                    affectedNodeId = m_nearRtRic->Data()->GetLteUeE2NodeIdFromCellInfo(
                        cellId,
                        (command->GetObject<OranCommandLte2LteHandover>())->GetTargetRnti());

                    LogLogicToStorage("Evaluating LTE-to-LTE Handover command affecting E2 Node " +
                                      std::to_string(affectedNodeId));
                }
                else
                {
                    // Default: Use the target E2 Node Id
                    affectedNodeId = command->GetTargetE2NodeId();

                    LogLogicToStorage("Evaluating commands affecting E2 Node " +
                                      std::to_string(affectedNodeId));
                }

                if (affectedNodes.find(affectedNodeId) != affectedNodes.end())
                {
                    // There is already a command affecting this node. The only way this
                    // new command replaces the old one is if this was issued by the default LM
                    // and the old one was not
                    if (defaultLm && !affectedNodes[affectedNodeId])
                    {
                        affectedNodes[affectedNodeId] = defaultLm;
                        selectedCommands[affectedNodeId] = command;

                        LogLogicToStorage("There was a command for this node, but the new command "
                                          "was issued by the default LM, so new replaces old.");
                    }
                    else
                    {
                        LogLogicToStorage("There was a command for this node, and the new command "
                                          "has lower precedence (old default? " +
                                          std::to_string(affectedNodes[affectedNodeId]) +
                                          "; new default? " + std::to_string(defaultLm) +
                                          "). Ignoring new command.");
                    }
                }
                else
                {
                    // No command affects the node, so we can just save the current one
                    affectedNodes[affectedNodeId] = defaultLm;
                    selectedCommands[affectedNodeId] = command;

                    LogLogicToStorage("There was no command for this node; Saving new command.");
                }
            }
        }
        // Collect the selected commands into the vector this method will output
        for (auto selcom : selectedCommands)
        {
            commands.push_back(selcom.second);
        }
    }
    else
    {
        // This module is not active. Just copy the same set of commands as output
        for (auto commandSet : inputCommands)
        {
            commands.insert(commands.end(), commandSet.second.begin(), commandSet.second.end());
        }
    }

    return commands;
}

} // namespace ns3
