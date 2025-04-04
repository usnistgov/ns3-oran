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

#include "ns3/oran-module.h"

#include <iostream>
#include <sqlite3.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("OranDataRepositoryExample");

/**
 * Short example that demonstrates how the data storage API can be used to
 * store and load information. In this example 3 notional nodes are
 * registered (1 wired node, 1 LTE eNB, and 1 LTE UE). After that the wired
 * node is then deregistered. Following notional positions at notional times
 * are saved for the LTE UE. These positions are then retrieved and reported
 * if they fall in a given time range. After that, notional LTE UE cell
 * information is stored, retrieved, and then reported.
 */
int
main(int argc, char* argv[])
{
    // Make sure we are not using an old DB file
    std::remove("oran-repository.db");

    // In this particular case we are using an SQLITE backend.
    Ptr<OranDataRepository> repository = CreateObject<OranDataRepositorySqlite>();

    // Initialize and activate the data store.
    repository->Activate();

    // Register three nodes
    // Register wired node
    uint64_t wiredE2NodeId = repository->RegisterNode(OranNearRtRic::NodeType::WIRED, 1);
    // Registere LTE eNB with
    repository->RegisterNodeLteUe(2, 1);
    // Register LTE UE
    uint64_t lteUeE2NodeId = repository->RegisterNodeLteUe(3, 1);

    // Check if the wired node is registered and report it's status
    if (repository->IsNodeRegistered(wiredE2NodeId))
    {
        std::cout << "E2 Node with ID = " << wiredE2NodeId << " is registered." << std::endl;
    }
    else
    {
        std::cout << "E2 Node with ID = " << wiredE2NodeId << " is NOT registered." << std::endl;
    }

    // Deregister the wired node.
    repository->DeregisterNode(wiredE2NodeId);

    // Check if the wired node is registered and report it's status
    if (repository->IsNodeRegistered(wiredE2NodeId))
    {
        std::cout << "E2 Node with ID = " << wiredE2NodeId << " is registered." << std::endl;
    }
    else
    {
        std::cout << "E2 Node with ID = " << wiredE2NodeId << " is NOT registered." << std::endl;
    }

    // Store some reported locations for the LTE UE.
    repository->SavePosition(lteUeE2NodeId, Vector(0, 0, 1.5), Seconds(0));
    repository->SavePosition(lteUeE2NodeId, Vector(10, 10, 1.5), Seconds(10));
    repository->SavePosition(lteUeE2NodeId, Vector(100, 100, 1.5), Seconds(100));

    // Get all positions for the LTE UE that fall within 0 s and 15 s of time.
    std::map<Time, Vector> lteUeNodePositions =
        repository->GetNodePositions(lteUeE2NodeId, Seconds(0), Seconds(15));

    // Report the retrieved positions.
    for (auto it = lteUeNodePositions.begin(); it != lteUeNodePositions.end(); it++)
    {
        std::cout << "E2 Node with ID = " << lteUeE2NodeId << " located at (" << it->second.x
                  << ", " << it->second.y << ", " << it->second.z << ")"
                  << " at time t = " << it->first.As(Time::S) << std::endl;
    }

    // Save information about the cell that the UE is currently connected.
    repository->SaveLteUeCellInfo(lteUeE2NodeId, 1, 1, Seconds(10));

    uint16_t rnti;
    uint16_t cellId;
    // Get the cell information that was stored for the UE and report it if it
    // was found.
    bool found;
    std::tie(found, cellId, rnti) = repository->GetLteUeCellInfo(lteUeE2NodeId);
    if (found)
    {
        std::cout << "LTE UE with E2 Node ID = " << lteUeE2NodeId
                  << " and RNTI = " << (uint32_t)rnti
                  << " is connected to cell with Cell ID = " << (uint32_t)cellId << std::endl;
    }

    return (0);
}
