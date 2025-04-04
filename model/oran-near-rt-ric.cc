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

#include "oran-near-rt-ric.h"

#include "oran-cmm.h"
#include "oran-command.h"
#include "oran-data-repository.h"
#include "oran-lm.h"
#include "oran-near-rt-ric-e2terminator.h"
#include "oran-query-trigger.h"

#include "ns3/abort.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/string.h"

#include <vector>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranNearRtRic");

NS_OBJECT_ENSURE_REGISTERED(OranNearRtRic);

TypeId
OranNearRtRic::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::OranNearRtRic")
            .SetParent<Object>()
            .AddConstructor<OranNearRtRic>()
            .AddAttribute("DefaultLogicModule",
                          "The default logic module.",
                          PointerValue(nullptr),
                          MakePointerAccessor(&OranNearRtRic::m_defaultLm),
                          MakePointerChecker<OranLm>())
            .AddAttribute("E2Terminator",
                          "The E2 Terminator for the Near-RT RIC.",
                          PointerValue(nullptr),
                          MakePointerAccessor(&OranNearRtRic::m_e2Terminator),
                          MakePointerChecker<OranNearRtRicE2Terminator>())
            .AddAttribute("DataRepository",
                          "The data repository.",
                          PointerValue(nullptr),
                          MakePointerAccessor(&OranNearRtRic::m_data),
                          MakePointerChecker<OranDataRepository>())
            .AddAttribute("ConflictMitigationModule",
                          "The Conflict Mitigation Module.",
                          PointerValue(nullptr),
                          MakePointerAccessor(&OranNearRtRic::m_cmm),
                          MakePointerChecker<OranCmm>())
            .AddAttribute("LmQueryInterval",
                          "Interval between periodic queries to the Logic Modules",
                          TimeValue(Seconds(5)),
                          MakeTimeAccessor(&OranNearRtRic::m_lmQueryInterval),
                          MakeTimeChecker(MilliSeconds(10)))
            .AddAttribute("LmQueryMaxWaitTime",
                          "The maximum time to wait for Logic Modules to finish. A value of \"0\" "
                          "indicates no limit.",
                          TimeValue(Seconds(0)),
                          MakeTimeAccessor(&OranNearRtRic::m_lmQueryMaxWaitTime),
                          MakeTimeChecker())
            .AddAttribute(
                "LmQueryLateCommandPolicy",
                "The filter to apply on UL CQIs received",
                EnumValue(OranNearRtRic::DROP),
                MakeEnumAccessor<LateCommandPolicy>(&OranNearRtRic::m_lmQueryLateCommandPolicy),
                MakeEnumChecker(OranNearRtRic::DROP, "DROP", OranNearRtRic::SAVE, "SAVE"))
            .AddAttribute("E2NodeInactivityThreshold",
                          "The amount of time from a node's last registration request before "
                          "becoming inactive.",
                          TimeValue(Seconds(5)),
                          MakeTimeAccessor(&OranNearRtRic::m_e2NodeInactivityThreshold),
                          MakeTimeChecker())
            .AddAttribute(
                "E2NodeInactivityIntervalRv",
                "The random variable used (in seconds) to periodically deregister inactive nodes.",
                StringValue("ns3::ConstantRandomVariable[Constant=5]"),
                MakePointerAccessor(&OranNearRtRic::m_e2NodeInactivityIntervalRv),
                MakePointerChecker<RandomVariableStream>());

    return tid;
}

OranNearRtRic::OranNearRtRic()
    : Object(),
      m_additionalLms(std::map<std::string, Ptr<OranLm>>()),
      m_active(false),
      m_lmQueryEvent(EventId()),
      m_e2NodeInactivityEvent(EventId()),
      m_lmQueryCycle(Seconds(0))
{
    NS_LOG_FUNCTION(this);
}

OranNearRtRic::~OranNearRtRic()
{
    NS_LOG_FUNCTION(this);
}

void
OranNearRtRic::Activate()
{
    NS_LOG_FUNCTION(this);

    NS_ABORT_MSG_IF(m_e2Terminator == nullptr,
                    "Attempting to activate Near-RT RIC with a NULL E2 Terminator");
    NS_ABORT_MSG_IF(m_defaultLm == nullptr,
                    "Attempting to activate Near-RT RIC with a NULL Default Logic Module");
    NS_ABORT_MSG_IF(m_cmm == nullptr,
                    "Attempting to activate Near-RT RIC with a NULL Conflict Mitigation Module");
    NS_ABORT_MSG_IF(m_data == nullptr,
                    "Attempting to activate Near-RT RIC with a NULL Data Repository");

    if (!m_active)
    {
        NS_LOG_LOGIC("Near-RT RIC activated");

        m_active = true;
        // Activate the E2 Terminator
        m_e2Terminator->Activate();
        // Activate the data repository
        m_data->Activate();
        // Activate the default and additional Logic Modules
        m_defaultLm->Activate();
        for (auto entry : m_additionalLms)
        {
            entry.second->Activate();
        }
        // Activate the conflic mitigation module
        m_cmm->Activate();
    }
}

void
OranNearRtRic::Deactivate()
{
    NS_LOG_FUNCTION(this);

    if (m_active)
    {
        NS_ABORT_MSG_IF(m_e2Terminator == nullptr,
                        "Attempting to deactivate Near-RT RIC with a NULL E2 Terminator");
        NS_ABORT_MSG_IF(m_defaultLm == nullptr,
                        "Attempting to deactivate Near-RT RIC with a NULL Default Logic Module");
        NS_ABORT_MSG_IF(
            m_cmm == nullptr,
            "Attempting to deactivate Near-RT RIC with a NULL Conflict Mitigation Module");
        NS_ABORT_MSG_IF(m_data == nullptr,
                        "Attempting to deactivate Near-RT RIC with a NULL Data Repository");

        NS_LOG_LOGIC("Near-RT RIC deactivated");

        // Deactivate the E2 Terminator
        m_e2Terminator->Deactivate();
        // Deactivate the data repository
        m_data->Deactivate();
        // Deactivate the default and additional Logic Modules
        m_defaultLm->Deactivate();
        for (auto entry : m_additionalLms)
        {
            entry.second->Deactivate();
        }
        // Deactivate the conflic mitigation module
        m_cmm->Deactivate();
    }
    m_active = false;
}

bool
OranNearRtRic::IsActive() const
{
    NS_LOG_FUNCTION(this);

    return m_active;
}

void
OranNearRtRic::Start()
{
    NS_LOG_FUNCTION(this);

    Activate();

    NS_ABORT_MSG_IF(m_lmQueryEvent.IsPending(), "Near-RT RIC has already been started");
    NS_ABORT_MSG_IF(m_e2NodeInactivityEvent.IsPending(), "Near-RT RIC has already been started");

    m_lmQueryEvent = Simulator::Schedule(m_lmQueryInterval, &OranNearRtRic::QueryLms, this);
    m_e2NodeInactivityEvent = Simulator::Schedule(Seconds(m_e2NodeInactivityIntervalRv->GetValue()),
                                                  &OranNearRtRic::CheckForInactivity,
                                                  this);
}

void
OranNearRtRic::Stop()
{
    NS_LOG_FUNCTION(this);

    Deactivate();
    m_lmQueryEvent.Cancel();
    m_e2NodeInactivityEvent.Cancel();
    m_processLmQueryCommandsEvent.Cancel();
}

Ptr<OranNearRtRicE2Terminator>
OranNearRtRic::GetE2Terminator() const
{
    NS_LOG_FUNCTION(this);

    return m_e2Terminator;
}

Ptr<OranLm>
OranNearRtRic::GetDefaultLogicModule() const
{
    NS_LOG_FUNCTION(this);

    return m_defaultLm;
}

void
OranNearRtRic::SetDefaultLogicModule(Ptr<OranLm> newDefaultLm)
{
    NS_LOG_FUNCTION(this << newDefaultLm);

    NS_ABORT_MSG_IF(newDefaultLm == nullptr,
                    "Attempting to set a NULL Default Logic Module in the Near-RT RIC");
    m_defaultLm = newDefaultLm;
}

Ptr<OranLm>
OranNearRtRic::GetAdditionalLogicModule(std::string name) const
{
    NS_LOG_FUNCTION(this << name);

    Ptr<OranLm> ret;

    auto foundLm = m_additionalLms.find(name);
    if (foundLm == m_additionalLms.end())
    {
        ret = nullptr;
    }
    else
    {
        ret = foundLm->second;
    }

    return ret;
}

OranNearRtRic::AddLmResult
OranNearRtRic::AddLogicModule(Ptr<OranLm> newLm)
{
    NS_LOG_FUNCTION(this);

    AddLmResult ret = OranNearRtRic::ADDLM_OK;
    if (m_additionalLms.find(newLm->GetName()) == m_additionalLms.end())
    {
        NS_LOG_LOGIC("Near-RT RIC adding the \"" << newLm->GetName() << "\" Logic Module");
        m_additionalLms[newLm->GetName()] = newLm;
    }
    else
    {
        NS_LOG_LOGIC("Near-RT RIC could not add \"" << newLm->GetName()
                                                    << "\" because Logic Module already exists");
        ret = OranNearRtRic::ADDLM_ERR_NAME_EXISTS;
    }

    return ret;
}

OranNearRtRic::RemoveLmResult
OranNearRtRic::RemoveLogicModule(std::string name)
{
    NS_LOG_FUNCTION(this);

    RemoveLmResult ret = OranNearRtRic::DELLM_OK;
    if (m_additionalLms.find(name) != m_additionalLms.end())
    {
        NS_LOG_LOGIC("Near-RT RIC removing the \"" << name << "\" Logic Module");
        m_additionalLms.erase(name);
    }
    else
    {
        NS_LOG_LOGIC("Near-RT RIC could not remove \""
                     << name << "\" because the Logic Module was not found");
        ret = OranNearRtRic::DELLM_ERR_NAME_INVALID;
    }

    return ret;
}

bool
OranNearRtRic::AddQueryTrigger(std::string name, Ptr<OranQueryTrigger> trigger)
{
    NS_LOG_FUNCTION(this << name << trigger);

    bool success = true;

    if (m_queryTriggers.find(name) == m_queryTriggers.end())
    {
        NS_LOG_LOGIC("Near-RT RIC adding the \"" << name << "\" query trigger");
        m_queryTriggers[name] = trigger;
    }
    else
    {
        NS_LOG_LOGIC("Near-RT RIC could not add \""
                     << name << "\" becasue the query trigger already exists");
        success = false;
    }

    return success;
}

bool
OranNearRtRic::RemoveQueryTrigger(std::string name)
{
    NS_LOG_FUNCTION(this << name);

    bool success = true;
    auto it = m_queryTriggers.find(name);

    if (it != m_queryTriggers.end())
    {
        NS_LOG_LOGIC("Near-RT RIC removing the \"" << name << "\" query trigger");
        m_queryTriggers.erase(it);
    }
    else
    {
        NS_LOG_LOGIC("Near-RT RIC could not remove \""
                     << name << "\" becasue the query trigger was not found");
        success = false;
    }

    return success;
}

Ptr<OranDataRepository>
OranNearRtRic::Data() const
{
    NS_LOG_FUNCTION(this);

    return m_data;
}

Ptr<OranCmm>
OranNearRtRic::GetCmm() const
{
    NS_LOG_FUNCTION(this);

    return m_cmm;
}

void
OranNearRtRic::SetCmm(Ptr<OranCmm> newCmm)
{
    NS_LOG_FUNCTION(this << newCmm);

    NS_ABORT_MSG_IF(newCmm == nullptr,
                    "Attempting to set a NULL Conflict Mitigation Module in the Near-RT RIC");
    m_cmm = newCmm;
}

void
OranNearRtRic::NotifyLmFinished(Time cycle, std::vector<Ptr<OranCommand>> commands, Ptr<OranLm> lm)
{
    NS_LOG_FUNCTION(this << cycle << commands << lm);

    // Indicate whether or not this is the default.
    bool isDefaultLm = (lm == m_defaultLm);
    // Create the key used to identify the LM.
    std::tuple<std::string, bool> key = std::make_tuple(lm->GetName(), isDefaultLm);

    if (m_lmQueryCommands.find(key) != m_lmQueryCommands.end())
    {
        m_lmQueryCommands[key] = std::vector<Ptr<OranCommand>>();
    }

    // Check if the issued commands belong to this cycle.
    if (cycle == m_lmQueryCycle)
    {
        // Check if commands still have yet to be processed.
        if (m_processLmQueryCommandsEvent.IsPending() || m_lmQueryMaxWaitTime == Seconds(0))
        {
            NS_LOG_LOGIC("Near-RT RIC received command(s) from \""
                         << lm->GetName() << "\" for cycle " << cycle.GetTimeStep());

            // Collect the commands generated by the LM
            m_lmQueryCommands[key].insert(m_lmQueryCommands[key].end(),
                                          commands.begin(),
                                          commands.end());
        }
        else
        {
            NS_LOG_WARN("Near-RT RIC received late command(s) for cycle "
                        << m_lmQueryCycle.GetTimeStep() << " from \"" << lm->GetName() << "\"");

            // Check what to do with late commands.
            switch (m_lmQueryLateCommandPolicy)
            {
            case DROP:
                NS_LOG_LOGIC("Dropping command(s) due to late command policy");
                break;
            case SAVE:
                NS_LOG_LOGIC("Saving command(s) for this cycle due to late command policy");
                m_lmQueryCommands[key].insert(m_lmQueryCommands[key].end(),
                                              commands.begin(),
                                              commands.end());
                break;
            default:
                NS_ABORT_MSG("Unsupported late command policy in Near-RT RIC");
                break;
            }
        }
    }
    else
    {
        NS_ABORT_MSG("Near-RT RIC received command for unexpected cycle");
    }

    // Only process LM query commands from here once all LMs have returned
    // commands AND it is either the case that all commands have been received
    // before the maiximum wait time has been exceeded or there is no maximum
    // wait time.
    if (m_lmQueryCommands.size() == m_additionalLms.size() + 1 &&
        (m_processLmQueryCommandsEvent.IsPending() || m_lmQueryMaxWaitTime == Seconds(0)))
    {
        ProcessLmQueryCommands();
    }
}

void
OranNearRtRic::NotifyReportReceived(Ptr<OranReport> report)
{
    NS_LOG_FUNCTION(this << report);

    NS_LOG_LOGIC("Near-RT RIC received a report");

    bool queryLms = false;

    for (auto qtrigger : m_queryTriggers)
    {
        if (qtrigger.second->QueryLms(report))
        {
            queryLms = true;
            NS_LOG_LOGIC("Near-RT RIC's \""
                         << qtrigger.first
                         << "\" query trigger has indicated that LMs should be queried");
        }
    }

    if (queryLms)
    {
        if (m_lmQueryEvent.IsPending())
        {
            m_lmQueryEvent.Cancel();
        }

        NS_LOG_LOGIC("Near-RT RIC LM query triggered based on received report");

        QueryLms();
    }
}

void
OranNearRtRic::DoDispose()
{
    NS_LOG_FUNCTION(this);

    m_e2Terminator = nullptr;
    m_data = nullptr;
    m_defaultLm = nullptr;

    m_additionalLms.clear();

    m_lmQueryEvent.Cancel();
    m_e2NodeInactivityEvent.Cancel();
    m_processLmQueryCommandsEvent.Cancel();

    m_cmm = nullptr;

    m_lmQueryCommands.clear();

    Object::DoDispose();
}

void
OranNearRtRic::QueryLms()
{
    NS_LOG_FUNCTION(this);

    if (m_active)
    {
        NS_ABORT_MSG_IF(m_e2Terminator == nullptr,
                        "Attempting to query LMs in a Near-RT RIC with a NULL E2 Terminator");
        NS_ABORT_MSG_IF(m_defaultLm == nullptr,
                        "Attempting to query LMs in Near-RT RIC with a NULL Default Logic Module");
        NS_ABORT_MSG_IF(
            m_cmm == nullptr,
            "Attempting to query LMs in Near-RT RIC with a NULL Conflict Mitigation Module");
        NS_ABORT_MSG_IF(m_data == nullptr,
                        "Attempting to query LMs in  Near-RT RIC with a NULL Data Repository");

        // Move to next cycle.
        m_lmQueryCycle = Simulator::Now();

        NS_LOG_LOGIC("Near-RT RIC querying LMs and signaling for them to run for cycle "
                     << m_lmQueryCycle.GetTimeStep());

        // Mark E2 nodes that have not recently sent a registration request as
        // in active.
        CheckForInactivity();

        if (m_lmQueryMaxWaitTime > Seconds(0))
        {
            m_processLmQueryCommandsEvent =
                Simulator::Schedule(m_lmQueryMaxWaitTime,
                                    &OranNearRtRic::ProcessLmQueryCommands,
                                    this);
        }

        // Check if LM is still running.
        if (m_defaultLm->IsRunning())
        {
            // Cancel the current process.
            m_defaultLm->CancelRun();
            NS_LOG_WARN("Near-RT RIC canceled run for \""
                        << m_defaultLm->GetName()
                        << "\" because it had not finished running by next query cycle");
        }

        // Signal default LM to run
        m_defaultLm->Run(m_lmQueryCycle);

        // Signal all additional LMs to run.
        for (auto lm : m_additionalLms)
        {
            // Check if LM is still running.
            if (lm.second->IsRunning())
            {
                // Cancel the current process.
                lm.second->CancelRun();
                NS_LOG_WARN("Near-RT RIC canceled run for \""
                            << lm.second->GetName()
                            << "\" because it had not finished running by next query cycle");
            }
            lm.second->Run(m_lmQueryCycle);
        }

        m_lmQueryEvent = Simulator::Schedule(m_lmQueryInterval, &OranNearRtRic::QueryLms, this);
    }
}

void
OranNearRtRic::CheckForInactivity()
{
    NS_LOG_FUNCTION(this);

    if (m_active)
    {
        NS_ABORT_MSG_IF(
            m_data == nullptr,
            "Attempting to check for inactivity in Near-RT RIC with a NULL Data Repository");
        NS_ABORT_MSG_IF(
            m_e2Terminator == nullptr,
            "Attempting to check for inactivity in Near-RT RIC with a NULL E2 Terminator");

        NS_LOG_LOGIC("Near-RT RIC checking for E2 Node inactivity");

        std::vector<std::tuple<uint64_t, Time>> lastRegistrations =
            m_data->GetLastRegistrationRequests();

        for (auto lastreg : lastRegistrations)
        {
            uint64_t e2NodeId;
            Time registrationTime;

            std::tie(e2NodeId, registrationTime) = lastreg;

            Time inactivityPeriod = Simulator::Now() - registrationTime;

            if (inactivityPeriod > m_e2NodeInactivityThreshold)
            {
                NS_LOG_LOGIC("Near-RT RIC deactivating E2 Node with ID "
                             << e2NodeId << " that has not registered in "
                             << inactivityPeriod.GetSeconds() << " s");
                m_e2Terminator->ReceiveDeregistrationRequest(e2NodeId);
            }
        }

        if (m_e2NodeInactivityEvent.IsPending())
        {
            m_e2NodeInactivityEvent.Cancel();
        }

        m_e2NodeInactivityEvent =
            Simulator::Schedule(Seconds(m_e2NodeInactivityIntervalRv->GetValue()),
                                &OranNearRtRic::CheckForInactivity,
                                this);
    }
}

void
OranNearRtRic::ProcessLmQueryCommands()
{
    NS_LOG_FUNCTION(this);

    if (m_active)
    {
        NS_LOG_LOGIC("Near-RT RIC processing commands");

        if (m_processLmQueryCommandsEvent.IsPending())
        {
            m_processLmQueryCommandsEvent.Cancel();
        }

        // Pass to the E2 Terminator the set of commands resulting
        // from the Conflict Mitigation Module filtering the complete
        // set of commands generated
        m_e2Terminator->ProcessCommands(m_cmm->Filter(m_lmQueryCommands));

        m_lmQueryCommands.clear();
    }
}

} // namespace ns3
