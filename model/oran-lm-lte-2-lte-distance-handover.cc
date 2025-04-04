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

#include "oran-lm-lte-2-lte-distance-handover.h"

#include "oran-command-lte-2-lte-handover.h"
#include "oran-data-repository.h"

#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"

#include <cfloat>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranLmLte2LteDistanceHandover");

NS_OBJECT_ENSURE_REGISTERED(OranLmLte2LteDistanceHandover);

TypeId
OranLmLte2LteDistanceHandover::GetTypeId()
{
    static TypeId tid = TypeId("ns3::OranLmLte2LteDistanceHandover")
                            .SetParent<OranLm>()
                            .AddConstructor<OranLmLte2LteDistanceHandover>();

    return tid;
}

OranLmLte2LteDistanceHandover::OranLmLte2LteDistanceHandover()
    : OranLm()
{
    NS_LOG_FUNCTION(this);

    m_name = "OranLmLte2LteDistanceHandover";
}

OranLmLte2LteDistanceHandover::~OranLmLte2LteDistanceHandover()
{
    NS_LOG_FUNCTION(this);
}

std::vector<Ptr<OranCommand>>
OranLmLte2LteDistanceHandover::Run()
{
    NS_LOG_FUNCTION(this);

    std::vector<Ptr<OranCommand>> commands;

    if (m_active)
    {
        NS_ABORT_MSG_IF(m_nearRtRic == nullptr,
                        "Attempting to run LM (" + m_name + ") with NULL Near-RT RIC");

        Ptr<OranDataRepository> data = m_nearRtRic->Data();
        std::vector<UeInfo> ueInfos = GetUeInfos(data);
        std::vector<EnbInfo> enbInfos = GetEnbInfos(data);
        commands = GetHandoverCommands(data, ueInfos, enbInfos);
    }

    // Return the commands.
    return commands;
}

std::vector<OranLmLte2LteDistanceHandover::UeInfo>
OranLmLte2LteDistanceHandover::GetUeInfos(Ptr<OranDataRepository> data) const
{
    NS_LOG_FUNCTION(this << data);

    std::vector<UeInfo> ueInfos;
    for (auto ueId : data->GetLteUeE2NodeIds())
    {
        UeInfo ueInfo;
        ueInfo.nodeId = ueId;
        // Get the current cell ID and RNTI of the UE and record it.
        bool found;
        std::tie(found, ueInfo.cellId, ueInfo.rnti) = data->GetLteUeCellInfo(ueInfo.nodeId);
        if (found)
        {
            // Get the latest location of the UE.
            std::map<Time, Vector> nodePositions =
                data->GetNodePositions(ueInfo.nodeId, Seconds(0), Simulator::Now());

            if (!nodePositions.empty())
            {
                // We found both the cell and location informtaion for this UE
                // so record it for a later analysis.
                ueInfo.position = nodePositions.rbegin()->second;
                ueInfos.push_back(ueInfo);
            }
            else
            {
                NS_LOG_INFO("Could not find LTE UE location for E2 Node ID = " << ueInfo.nodeId);
            }
        }
        else
        {
            NS_LOG_INFO("Could not find LTE UE cell info for E2 Node ID = " << ueInfo.nodeId);
        }
    }
    return ueInfos;
}

std::vector<OranLmLte2LteDistanceHandover::EnbInfo>
OranLmLte2LteDistanceHandover::GetEnbInfos(Ptr<OranDataRepository> data) const
{
    NS_LOG_FUNCTION(this << data);

    std::vector<EnbInfo> enbInfos;
    for (auto enbId : data->GetLteEnbE2NodeIds())
    {
        EnbInfo enbInfo;
        enbInfo.nodeId = enbId;
        // Get the cell ID of this eNB and record it.
        bool found;
        std::tie(found, enbInfo.cellId) = data->GetLteEnbCellInfo(enbInfo.nodeId);
        if (found)
        {
            // Get all known locations of the eNB.
            std::map<Time, Vector> nodePositions =
                data->GetNodePositions(enbInfo.nodeId, Seconds(0), Simulator::Now());

            if (!nodePositions.empty())
            {
                // We found both the cell and location information for this
                // eNB so record it for a later analysis.
                enbInfo.position = nodePositions.rbegin()->second;
                enbInfos.push_back(enbInfo);
            }
            else
            {
                NS_LOG_INFO("Could not find LTE eNB location for E2 Node ID = " << enbInfo.nodeId);
            }
        }
        else
        {
            NS_LOG_INFO("Could not find LTE eNB cell info for E2 Node ID = " << enbInfo.nodeId);
        }
    }
    return enbInfos;
}

std::vector<Ptr<OranCommand>>
OranLmLte2LteDistanceHandover::GetHandoverCommands(
    Ptr<OranDataRepository> data,
    std::vector<OranLmLte2LteDistanceHandover::UeInfo> ueInfos,
    std::vector<OranLmLte2LteDistanceHandover::EnbInfo> enbInfos) const
{
    NS_LOG_FUNCTION(this << data);

    std::vector<Ptr<OranCommand>> commands;

    // Compare the location of each active eNB with the location of each active
    // UE and see if that UE is currently being served by the closet cell. If
    // there is a closer eNB to the UE then the currently serving cell then
    // issue a handover command.
    for (auto ueInfo : ueInfos)
    {
        double min = DBL_MAX;               // The minimum distance recorded.
        uint64_t oldCellNodeId;             // The ID of the cell currently serving the UE.
        uint16_t newCellId = ueInfo.cellId; // The ID of the closest cell.
        for (const auto& enbInfo : enbInfos)
        {
            // Calculate the distance between the UE and eNB.
            double dist = std::sqrt(std::pow(ueInfo.position.x - enbInfo.position.x, 2) +
                                    std::pow(ueInfo.position.y - enbInfo.position.y, 2) +
                                    std::pow(ueInfo.position.z - enbInfo.position.z, 2));

            LogLogicToRepository("Distance from UE with RNTI " + std::to_string(ueInfo.rnti) +
                                 " in CellID " + std::to_string(ueInfo.cellId) +
                                 " to eNB with CellID " + std::to_string(enbInfo.cellId) + " is " +
                                 std::to_string(dist));

            // Check if the distance is shorter than the current minimum
            if (dist < min)
            {
                // Record the new minimum
                min = dist;
                // Record the ID of the cell that produced the new minimum.
                newCellId = enbInfo.cellId;

                LogLogicToRepository("Distance to eNB with CellID " +
                                     std::to_string(enbInfo.cellId) + " is shortest so far");
            }

            // Check if this cell is the currently serving this UE.
            if (ueInfo.cellId == enbInfo.cellId)
            {
                // It is, so indicate record the ID of the cell that is
                // currently serving the UE.
                oldCellNodeId = enbInfo.nodeId;
            }
        }

        // Check if the ID of the closest cell is different from ID of the cell
        // that is currently serving the UE
        if (newCellId != ueInfo.cellId)
        {
            // It is, so issue a handover command.
            Ptr<OranCommandLte2LteHandover> handoverCommand =
                CreateObject<OranCommandLte2LteHandover>();
            // Send the command to the cell currently serving the UE.
            handoverCommand->SetAttribute("TargetE2NodeId", UintegerValue(oldCellNodeId));
            // Use the RNTI that the current cell is using to identify the UE.
            handoverCommand->SetAttribute("TargetRnti", UintegerValue(ueInfo.rnti));
            // Give the current cell the ID of the new cell to handover to.
            handoverCommand->SetAttribute("TargetCellId", UintegerValue(newCellId));
            // Log the command to the storage
            data->LogCommandLm(m_name, handoverCommand);
            // Add the command to send.
            commands.push_back(handoverCommand);

            LogLogicToRepository("Closest eNB (CellID " + std::to_string(newCellId) + ")" +
                                 " is different than the currently attached eNB" + " (CellID " +
                                 std::to_string(ueInfo.cellId) + ")." +
                                 " Issuing handover command.");
        }
    }
    return commands;
}

} // namespace ns3
