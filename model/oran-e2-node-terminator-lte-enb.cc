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

#include "oran-e2-node-terminator-lte-enb.h"

#include "oran-command-lte-2-lte-handover.h"

#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/lte-enb-net-device.h"
#include "ns3/lte-enb-rrc.h"
#include "ns3/node.h"
#include "ns3/pointer.h"
#include "ns3/string.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranE2NodeTerminatorLteEnb");

NS_OBJECT_ENSURE_REGISTERED(OranE2NodeTerminatorLteEnb);

TypeId
OranE2NodeTerminatorLteEnb::GetTypeId()
{
    static TypeId tid = TypeId("ns3::OranE2NodeTerminatorLteEnb")
                            .SetParent<OranE2NodeTerminator>()
                            .AddConstructor<OranE2NodeTerminatorLteEnb>();

    return tid;
}

OranE2NodeTerminatorLteEnb::OranE2NodeTerminatorLteEnb()
    : OranE2NodeTerminator()
{
    NS_LOG_FUNCTION(this);
}

OranE2NodeTerminatorLteEnb::~OranE2NodeTerminatorLteEnb()
{
    NS_LOG_FUNCTION(this);
}

OranNearRtRic::NodeType
OranE2NodeTerminatorLteEnb::GetNodeType() const
{
    NS_LOG_FUNCTION(this);

    return OranNearRtRic::NodeType::LTEENB;
}

void
OranE2NodeTerminatorLteEnb::ReceiveCommand(Ptr<OranCommand> command)
{
    NS_LOG_FUNCTION(this << command);

    if (m_active)
    {
        if (command->GetInstanceTypeId() == OranCommandLte2LteHandover::GetTypeId())
        {
            Ptr<Node> node = GetNode();
            Ptr<OranCommandLte2LteHandover> handoverCommand =
                command->GetObject<OranCommandLte2LteHandover>();

            Ptr<LteEnbRrc> lteEnbRrc = GetNetDevice()->GetRrc();
            lteEnbRrc->SendHandoverRequest(handoverCommand->GetTargetRnti(),
                                           handoverCommand->GetTargetCellId());
        }
    }
}

Ptr<LteEnbNetDevice>
OranE2NodeTerminatorLteEnb::GetNetDevice() const
{
    NS_LOG_FUNCTION(this);

    Ptr<LteEnbNetDevice> lteEnbNetDev =
        GetNode()->GetDevice(GetNetDeviceIndex())->GetObject<LteEnbNetDevice>();

    NS_ABORT_MSG_IF(lteEnbNetDev == nullptr, "Unable to find appropriate network device");

    return lteEnbNetDev;
}

} // namespace ns3
