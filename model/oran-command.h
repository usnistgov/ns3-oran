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

#ifndef ORAN_COMMAND_H
#define ORAN_COMMAND_H

#include "ns3/object.h"

#include <string>

namespace ns3
{

/**
 * @ingroup oran
 *
 * Base class of a Command. This class provides the implementation of
 * the GetTargetE2NodeId method for all the Commands. This base class can
 * be instantiated, but the Command issued lacks any meaning and will be
 * silently ignored by the E2 Node Terminators.
 */
class OranCommand : public Object
{
  public:
    /**
     * Gets the TypeId of the OranCommand class.
     *
     * @return The TypeId.
     */
    static TypeId GetTypeId();
    /**
     * Creates an instance of the OranCommand class.
     */
    OranCommand();
    /**
     * The destructor of the OranCommand class.
     */
    ~OranCommand() override;
    /**
     * Get a string representation of this command.
     *
     * @return A string representation of this command.
     */
    virtual std::string ToString() const;
    /**
     * Get the target E2 Node ID.
     *
     * @return The target E2 Node Id.
     */
    uint64_t GetTargetE2NodeId() const;

  private:
    /**
     * The target E2 Node Id
     */
    uint64_t m_targetE2NodeId;
}; // class OranCommand

} // namespace ns3

#endif /* ORAN_COMMAND_H */
