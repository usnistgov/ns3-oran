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

#include "oran-lm.h"

#include "oran-data-repository.h"
#include "oran-near-rt-ric.h"

#include "ns3/abort.h"
#include "ns3/boolean.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/string.h"

#include <string>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranLm");

NS_OBJECT_ENSURE_REGISTERED(OranLm);

TypeId
OranLm::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::OranLm")
            .SetParent<Object>()
            .AddAttribute("NearRtRic",
                          "The near RT RIC.",
                          PointerValue(nullptr),
                          MakePointerAccessor(&OranLm::m_nearRtRic),
                          MakePointerChecker<OranNearRtRic>())
            .AddAttribute("Verbose",
                          "Flag to indicate if logic should be logged to the data storage.",
                          BooleanValue(false),
                          MakeBooleanAccessor(&OranLm::m_verbose),
                          MakeBooleanChecker())
            .AddAttribute("ProcessingDelayRv",
                          "The random variable used to determine the delay (in seconds) to run.",
                          StringValue("ns3::ConstantRandomVariable[Constant=0]"),
                          MakePointerAccessor(&OranLm::m_processingDelayRv),
                          MakePointerChecker<RandomVariableStream>());

    return tid;
}

OranLm::OranLm()
    : Object()
{
    NS_LOG_FUNCTION(this);
}

OranLm::~OranLm()
{
    NS_LOG_FUNCTION(this);
}

void
OranLm::Activate()
{
    NS_LOG_FUNCTION(this);

    NS_ABORT_MSG_IF(m_nearRtRic == nullptr, "Attempting to activate LM with NULL Near-RT RIC");

    NS_LOG_LOGIC("\"" << m_name << "\" Logic Module activated");

    m_active = true;
}

void
OranLm::Deactivate()
{
    NS_LOG_FUNCTION(this);

    NS_LOG_LOGIC("\"" << m_name << "\" Logic Module deactivated");

    m_active = false;

    if (IsRunning())
    {
        CancelRun();
    }
}

bool
OranLm::IsActive() const
{
    NS_LOG_FUNCTION(this);

    return m_active;
}

std::string
OranLm::GetName() const
{
    NS_LOG_FUNCTION(this);

    return m_name;
}

void
OranLm::SetName(std::string_view name)
{
    NS_LOG_FUNCTION(this << name);

    m_name = name;
}

void
OranLm::Run(Time cycle)
{
    NS_LOG_FUNCTION(this << cycle);

    if (m_active)
    {
        NS_ABORT_MSG_IF(IsRunning(), "Attempting to run LM that is already running");

        NS_LOG_LOGIC("\"" << m_name << "\" Logic Module starting to run");

        double delay = m_processingDelayRv->GetValue();

        delay = delay < 0.0 ? 0.0 : delay;

        m_cycle = cycle;
        m_commands = Run();

        m_finishRunEvent = Simulator::Schedule(Seconds(delay), &OranLm::FinishRun, this);
    }
}

void
OranLm::CancelRun()
{
    NS_LOG_FUNCTION(this);

    if (m_active && IsRunning())
    {
        m_finishRunEvent.Cancel();

        std::string msg = "Run canceld for cycle " + std::to_string(m_cycle.GetTimeStep()) +
                          " with " + std::to_string(m_commands.size()) + " command(s) lost";

        if (!m_commands.empty())
        {
            msg += " {";

            for (auto command : m_commands)
            {
                msg += command->ToString() + ",";
            }

            msg.pop_back();
            msg += "}";

            LogLogicToRepository(msg);
        }

        m_commands.clear();
    }
}

bool
OranLm::IsRunning() const
{
    NS_LOG_FUNCTION(this);

    return m_finishRunEvent.IsPending();
}

void
OranLm::DoDispose()
{
    NS_LOG_FUNCTION(this);

    m_nearRtRic = nullptr;
    m_processingDelayRv = nullptr;

    m_finishRunEvent.Cancel();

    Object::DoDispose();
}

void
OranLm::LogLogicToRepository(const std::string& msg) const
{
    NS_LOG_FUNCTION(this << msg);

    NS_ABORT_MSG_IF(m_nearRtRic == nullptr, "Attempting to log LM logic with NULL Near-RT RIC");

    if (m_verbose)
    {
        m_nearRtRic->Data()->LogActionLm(m_name,
                                         std::to_string(Simulator::Now().GetSeconds()) + " -- " +
                                             m_name + " -- " + msg);
    }
}

void
OranLm::FinishRun()
{
    NS_LOG_FUNCTION(this);

    if (m_active)
    {
        NS_ABORT_MSG_IF(m_nearRtRic == nullptr, "Attempting to run LM logic with NULL Near-RT RIC");

        NS_LOG_LOGIC("\"" << m_name << "\" Logic Module finished running");

        m_nearRtRic->NotifyLmFinished(m_cycle, m_commands, GetObject<OranLm>());

        m_commands.clear();
    }
}

} // namespace ns3
