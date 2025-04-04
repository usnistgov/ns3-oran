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

#ifndef ORAN_E2_NODE_TERMINATOR_CONTAINER_H
#define ORAN_E2_NODE_TERMINATOR_CONTAINER_H

#include "oran-e2-node-terminator.h"

#include <vector>

namespace ns3
{

/**
 * @ingroup oran
 *
 * A Container for E2 Node Terminator. This Container allows the Activation and
 * Deactivation of all Terminators in it at once.
 */
class OranE2NodeTerminatorContainer
{
  public:
    /**
     * Declaration of container iterator.
     */
    typedef std::vector<Ptr<OranE2NodeTerminator>>::const_iterator Iterator;
    /**
     * Constructor of the ORanE2NodeTerminatorContainer.
     */
    OranE2NodeTerminatorContainer() = default;
    /**
     * Destructor of the OranE2NodeTerminatorContainer.
     */
    virtual ~OranE2NodeTerminatorContainer() = default;
    /**
     * Activate all of the E2 Node Terminators in the container.
     */
    void Activate();
    /**
     * Get an iterator referencing the first element.
     *
     * @return The iterator.
     */
    Iterator Begin() const;
    /**
     * Deactivate all of the E2 Node Terminators in the container.
     */
    void Deactivate();
    /**
     * Get an iterator referencing the last element.
     *
     * @return The iterator.
     */
    Iterator End() const;
    /**
     * Get the number of elements in the container.
     *
     * @return The number of elements.
     */
    uint32_t GetN() const;
    /**
     * Get an element from the container.
     *
     * @param i The index of the element to return.
     *
     * @return The element.
     */
    Ptr<OranE2NodeTerminator> Get(uint32_t i) const;
    /**
     * Add the elements of another container to this container.
     *
     * @param other The other container.
     */
    void Add(OranE2NodeTerminatorContainer other);
    /**
     * Add an element to this container.
     *
     * @param e2NodeTerminator The element to add.
     */
    void Add(Ptr<OranE2NodeTerminator> e2NodeTerminator);

  private:
    /**
     * The underlying container.
     */
    std::vector<Ptr<OranE2NodeTerminator>> m_e2NodeTerminators;
}; // class OranE2NodeTerminatorContainer

} // namespace ns3

#endif // ORAN_E2_NODE_TERMINATOR_CONTAINER_H
