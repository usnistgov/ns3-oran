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

#include "oran-lm-noop.h"

#include "oran-command.h"
#include "oran-near-rt-ric.h"

#include "ns3/abort.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranLmNoop");

NS_OBJECT_ENSURE_REGISTERED(OranLmNoop);

TypeId
OranLmNoop::GetTypeId()
{
    static TypeId tid = TypeId("ns3::OranLmNoop").SetParent<OranLm>().AddConstructor<OranLmNoop>();

    return tid;
}

OranLmNoop::OranLmNoop()
    : OranLm()
{
    NS_LOG_FUNCTION(this);

    m_name = "OranLmNoop";
}

OranLmNoop::~OranLmNoop()
{
    NS_LOG_FUNCTION(this);
}

std::vector<Ptr<OranCommand>>
OranLmNoop::Run()
{
    NS_LOG_FUNCTION(this);

    // Do nothing and return an empty vector of commands
    // As we do nothing, there is no need to check if the LM
    // is active or not.
    // We check if the pointer to the Near-RT RIC has been set,
    // though, as not having that one should be a configuration
    // problem (and may be a symptom of other issues)
    NS_ABORT_MSG_IF(m_nearRtRic == nullptr,
                    "Attempting to run LM (" + m_name + ") with NULL Near-RT RIC");

    LogLogicToRepository("No action taken");
    return {};
}

} // namespace ns3
