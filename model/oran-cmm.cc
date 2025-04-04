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

#include "oran-cmm.h"

#include "oran-command.h"
#include "oran-data-repository.h"
#include "oran-near-rt-ric.h"

#include "ns3/abort.h"
#include "ns3/boolean.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranCmm");

NS_OBJECT_ENSURE_REGISTERED(OranCmm);

TypeId
OranCmm::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::OranCmm")
            .SetParent<Object>()
            .AddAttribute("NearRtRic",
                          "The near RT RIC.",
                          PointerValue(nullptr),
                          MakePointerAccessor(&OranCmm::m_nearRtRic),
                          MakePointerChecker<OranNearRtRic>())
            .AddAttribute("Verbose",
                          "Flag to indicate if logic should be logged to the data storage",
                          BooleanValue(false),
                          MakeBooleanAccessor(&OranCmm::m_verbose),
                          MakeBooleanChecker());

    return tid;
}

OranCmm::OranCmm()
    : m_active(false),
      m_name("Cmm")
{
    NS_LOG_FUNCTION(this);
}

OranCmm::~OranCmm()
{
    NS_LOG_FUNCTION(this);
}

void
OranCmm::Activate()
{
    NS_LOG_FUNCTION(this);

    NS_ABORT_MSG_IF(m_nearRtRic == nullptr, "Attempting to activate LM with NULL Near-RT RIC");

    m_active = true;
}

void
OranCmm::Deactivate()
{
    NS_LOG_FUNCTION(this);

    m_active = false;
}

bool
OranCmm::IsActive() const
{
    NS_LOG_FUNCTION(this);

    return m_active;
}

std::string
OranCmm::GetName() const
{
    NS_LOG_FUNCTION(this);

    return m_name;
}

void
OranCmm::SetName(const std::string& name)
{
    NS_LOG_FUNCTION(this << name);

    m_name = name;
}

void
OranCmm::DoDispose()
{
    NS_LOG_FUNCTION(this);

    m_nearRtRic = nullptr;

    Object::DoDispose();
}

void
OranCmm::LogLogicToStorage(const std::string& msg) const
{
    NS_LOG_FUNCTION(this << msg);

    NS_ABORT_MSG_IF(m_nearRtRic == nullptr, "Attempting to log CMM logic with NULL Near-RT RIC");

    if (m_verbose)
    {
        m_nearRtRic->Data()->LogActionCmm(m_name,
                                          std::to_string(Simulator::Now().GetSeconds()) + " -- " +
                                              m_name + " -- " + msg);
    }
}

} // namespace ns3
