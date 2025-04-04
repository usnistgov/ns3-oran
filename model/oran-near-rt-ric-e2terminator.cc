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

#include "oran-near-rt-ric-e2terminator.h"

#include "oran-command.h"
#include "oran-data-repository.h"
#include "oran-e2-node-terminator-lte-enb.h"
#include "oran-e2-node-terminator-lte-ue.h"
#include "oran-e2-node-terminator.h"
#include "oran-near-rt-ric.h"
#include "oran-report-apploss.h"
#include "oran-report-location.h"
#include "oran-report-lte-ue-cell-info.h"
#include "oran-report-lte-ue-rsrp-rsrq.h"
#include "oran-report.h"

#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/lte-enb-net-device.h"
#include "ns3/lte-enb-rrc.h"
#include "ns3/lte-ue-net-device.h"
#include "ns3/lte-ue-rrc.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/string.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranNearRtRicE2Terminator");
NS_OBJECT_ENSURE_REGISTERED(OranNearRtRicE2Terminator);

TypeId
OranNearRtRicE2Terminator::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::OranNearRtRicE2Terminator")
            .SetParent<Object>()
            .AddConstructor<OranNearRtRicE2Terminator>()
            .AddAttribute("NearRtRic",
                          "Pointer to the Near-RT RIC",
                          PointerValue(nullptr),
                          MakePointerAccessor(&OranNearRtRicE2Terminator::m_nearRtRic),
                          MakePointerChecker<OranNearRtRic>())
            .AddAttribute("DataRepository",
                          "Pointer to the Data Repository",
                          PointerValue(nullptr),
                          MakePointerAccessor(&OranNearRtRicE2Terminator::m_data),
                          MakePointerChecker<OranDataRepository>())
            .AddAttribute("TransmissionDelayRv",
                          "The random variable used (in seconds) to calculate the transmission "
                          "delay for a command.",
                          StringValue("ns3::ConstantRandomVariable[Constant=0]"),
                          MakePointerAccessor(&OranNearRtRicE2Terminator::m_transmissionDelayRv),
                          MakePointerChecker<RandomVariableStream>());

    return tid;
}

OranNearRtRicE2Terminator::OranNearRtRicE2Terminator()
    : Object(),
      m_active(false),
      m_nodeTerminators(std::map<uint64_t, Ptr<OranE2NodeTerminator>>())
{
    NS_LOG_FUNCTION(this);
}

OranNearRtRicE2Terminator::~OranNearRtRicE2Terminator()
{
    NS_LOG_FUNCTION(this);
}

void
OranNearRtRicE2Terminator::Activate()
{
    NS_LOG_FUNCTION(this);

    m_active = true;
}

void
OranNearRtRicE2Terminator::Deactivate()
{
    NS_LOG_FUNCTION(this);

    m_active = false;
}

bool
OranNearRtRicE2Terminator::IsActive() const
{
    NS_LOG_FUNCTION(this);

    return m_active;
}

void
OranNearRtRicE2Terminator::ReceiveRegistrationRequest(OranNearRtRic::NodeType type,
                                                      uint64_t id,
                                                      Ptr<OranE2NodeTerminator> terminator)
{
    NS_LOG_FUNCTION(this << type << id << terminator);

    if (m_active)
    {
        NS_ABORT_MSG_IF(
            m_data == nullptr,
            "Attempting to use a null data repository in the Near-RT RIC E2 Terminator");
        NS_ABORT_MSG_IF(terminator == nullptr, "Attempting to register a NULL Node E2 Terminator");

        uint64_t e2NodeId;
        switch (type)
        {
        case OranNearRtRic::NodeType::LTEUE:
            e2NodeId = m_data->RegisterNodeLteUe(id,
                                                 terminator->GetObject<OranE2NodeTerminatorLteUe>()
                                                     ->GetNetDevice()
                                                     ->GetRrc()
                                                     ->GetImsi());
            break;
        case OranNearRtRic::NodeType::LTEENB:
            e2NodeId = m_data->RegisterNodeLteEnb(
                id,
                terminator->GetObject<OranE2NodeTerminatorLteEnb>()->GetNetDevice()->GetCellId());
            break;
        default:
            e2NodeId = m_data->RegisterNode(type, id);
            break;
        }
        m_nodeTerminators[e2NodeId] = terminator;

        Simulator::Schedule(Seconds(m_transmissionDelayRv->GetValue()),
                            &OranE2NodeTerminator::ReceiveRegistrationResponse,
                            terminator,
                            e2NodeId);
    }
}

void
OranNearRtRicE2Terminator::ReceiveDeregistrationRequest(uint64_t e2NodeId)
{
    NS_LOG_FUNCTION(this << e2NodeId);

    if (m_active)
    {
        NS_ABORT_MSG_IF(
            m_data == nullptr,
            "Attempting to use a null data repository in the Near-RT RIC E2 Terminator");

        uint64_t deregisteredE2NodeId = m_data->DeregisterNode(e2NodeId);

        Simulator::Schedule(Seconds(m_transmissionDelayRv->GetValue()),
                            &OranE2NodeTerminator::ReceiveDeregistrationResponse,
                            m_nodeTerminators[e2NodeId],
                            deregisteredE2NodeId);
    }
}

void
OranNearRtRicE2Terminator::ReceiveReport(Ptr<OranReport> report)
{
    NS_LOG_FUNCTION(this << report->ToString());

    if (m_active)
    {
        NS_ABORT_MSG_IF(
            m_data == nullptr,
            "Attempting to use a null data repository in the Near-RT RIC E2 Terminator");

        if (report->GetInstanceTypeId() == TypeId::LookupByName("ns3::OranReportLocation"))
        {
            Ptr<OranReportLocation> posRpt = report->GetObject<OranReportLocation>();
            m_data->SavePosition(posRpt->GetReporterE2NodeId(),
                                 posRpt->GetLocation(),
                                 posRpt->GetTime());
        }
        else if (report->GetInstanceTypeId() ==
                 TypeId::LookupByName("ns3::OranReportLteUeCellInfo"))
        {
            Ptr<OranReportLteUeCellInfo> lteUeCellInfoRpt =
                report->GetObject<OranReportLteUeCellInfo>();
            m_data->SaveLteUeCellInfo(lteUeCellInfoRpt->GetReporterE2NodeId(),
                                      lteUeCellInfoRpt->GetCellId(),
                                      lteUeCellInfoRpt->GetRnti(),
                                      lteUeCellInfoRpt->GetTime());
        }
        else if (report->GetInstanceTypeId() == TypeId::LookupByName("ns3::OranReportAppLoss"))
        {
            Ptr<OranReportAppLoss> appLossRpt = report->GetObject<OranReportAppLoss>();
            m_data->SaveAppLoss(appLossRpt->GetReporterE2NodeId(),
                                appLossRpt->GetLoss(),
                                appLossRpt->GetTime());
        }
        else if (report->GetInstanceTypeId() ==
                 TypeId::LookupByName("ns3::OranReportLteUeRsrpRsrq"))
        {
            Ptr<OranReportLteUeRsrpRsrq> rsrpRsrqRpt = report->GetObject<OranReportLteUeRsrpRsrq>();
            m_data->SaveLteUeRsrpRsrq(rsrpRsrqRpt->GetReporterE2NodeId(),
                                      rsrpRsrqRpt->GetTime(),
                                      rsrpRsrqRpt->GetRnti(),
                                      rsrpRsrqRpt->GetCellId(),
                                      rsrpRsrqRpt->GetRsrp(),
                                      rsrpRsrqRpt->GetRsrq(),
                                      rsrpRsrqRpt->GetIsServingCell(),
                                      rsrpRsrqRpt->GetComponentCarrierId());
        }

        m_nearRtRic->NotifyReportReceived(report);
    }
}

void
OranNearRtRicE2Terminator::SendCommand(Ptr<OranCommand> command)
{
    NS_LOG_FUNCTION(this << command->ToString());

    if (m_active)
    {
        NS_ABORT_MSG_IF(
            m_data == nullptr,
            "Attempting to use a null data repository in the Near-RT RIC E2 Terminator");

        m_data->LogCommandE2Terminator(command);

        Simulator::Schedule(Seconds(m_transmissionDelayRv->GetValue()),
                            &OranE2NodeTerminator::ReceiveCommand,
                            m_nodeTerminators[command->GetTargetE2NodeId()],
                            command);
    }
}

void
OranNearRtRicE2Terminator::ProcessCommands(std::vector<Ptr<OranCommand>> commands)
{
    NS_LOG_FUNCTION(this);

    if (m_active)
    {
        for (const auto& cmd : commands)
        {
            SendCommand(cmd);
        }
    }
}

void
OranNearRtRicE2Terminator::DoDispose()
{
    NS_LOG_FUNCTION(this);

    m_nearRtRic = nullptr;
    m_data = nullptr;
    m_nodeTerminators.clear();
    m_transmissionDelayRv = nullptr;

    Object::DoDispose();
}

} // namespace ns3
