/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#ifndef ORAN_HELPER_H
#define ORAN_HELPER_H

#include <ns3/attribute.h>
#include <ns3/net-device-container.h>
#include <ns3/object-factory.h>
#include <ns3/oran-near-rt-ric.h>
#include <ns3/ptr.h>

namespace ns3 {

/**
 * \ingroup oran
 * The ORAN helper.
 */
class OranHelper
{
public:
  /**
   * The constructor for the OranHelper class.
   */
  OranHelper (void);
  /**
   * The destructor for the OranHelper class.
   */
  virtual ~OranHelper (void);
  /**
   * Connects LTE eNB net devices to the RIC.
   * \param nearRtRic The near real-time RIC.
   * \param lteUeNetDevs The collection of LTE eNB net devices.
   * \returns The ORAN net devices.
   */
  virtual NetDeviceContainer ConnectLteEnbs (Ptr<OranNearRtRic> nearRtRic, NetDeviceContainer lteEnbNetDevs);
  /**
   * Connects LTE UE net devices to the RIC.
   * \param nearRtRic The near real-time RIC.
   * \param lteUeNetDevs The collection of LTE UE net devices.
   * \returns The ORAN net devices
   */
  virtual NetDeviceContainer ConnectLteUes (Ptr<OranNearRtRic> nearRtRic, NetDeviceContainer lteUeNetDevs);
  /**
   * Configures the E2 interface.
   * \param tid the string representation of the ns3::TypeId associated with the model to set
   * \param n0 the name of the attribute to set
   * \param v0 the value of the attribute to set
   * \param n1 the name of the attribute to set
   * \param v1 the value of the attribute to set
   * \param n2 the name of the attribute to set
   * \param v2 the value of the attribute to set
   * \param n3 the name of the attribute to set
   * \param v3 the value of the attribute to set
   * \param n4 the name of the attribute to set
   * \param v4 the value of the attribute to set
   * \param n5 the name of the attribute to set
   * \param v5 the value of the attribute to set
   * \param n6 the name of the attribute to set
   * \param v6 the value of the attribute to set
   * \param n7 the name of the attribute to set
   * \param v7 the value of the attribute to set
   */
  virtual void SetE2Interface (std::string tid,
      std::string n0 = "", const AttributeValue& v0 = EmptyAttributeValue (),
      std::string n1 = "", const AttributeValue& v1 = EmptyAttributeValue (),
      std::string n2 = "", const AttributeValue& v2 = EmptyAttributeValue (),
      std::string n3 = "", const AttributeValue& v3 = EmptyAttributeValue (),
      std::string n4 = "", const AttributeValue& v4 = EmptyAttributeValue (),
      std::string n5 = "", const AttributeValue& v5 = EmptyAttributeValue (),
      std::string n6 = "", const AttributeValue& v6 = EmptyAttributeValue (),
      std::string n7 = "", const AttributeValue& v7 = EmptyAttributeValue ());
  /**
   * Configures the E2 LTE eNB.
   * \param tid the string representation of the ns3::TypeId associated with the model to set
   * \param n0 the name of the attribute to set
   * \param v0 the value of the attribute to set
   * \param n1 the name of the attribute to set
   * \param v1 the value of the attribute to set
   * \param n2 the name of the attribute to set
   * \param v2 the value of the attribute to set
   * \param n3 the name of the attribute to set
   * \param v3 the value of the attribute to set
   * \param n4 the name of the attribute to set
   * \param v4 the value of the attribute to set
   * \param n5 the name of the attribute to set
   * \param v5 the value of the attribute to set
   * \param n6 the name of the attribute to set
   * \param v6 the value of the attribute to set
   * \param n7 the name of the attribute to set
   * \param v7 the value of the attribute to set
   */
  virtual void SetE2LteEnb (std::string tid,
      std::string n0 = "", const AttributeValue& v0 = EmptyAttributeValue (),
      std::string n1 = "", const AttributeValue& v1 = EmptyAttributeValue (),
      std::string n2 = "", const AttributeValue& v2 = EmptyAttributeValue (),
      std::string n3 = "", const AttributeValue& v3 = EmptyAttributeValue (),
      std::string n4 = "", const AttributeValue& v4 = EmptyAttributeValue (),
      std::string n5 = "", const AttributeValue& v5 = EmptyAttributeValue (),
      std::string n6 = "", const AttributeValue& v6 = EmptyAttributeValue (),
      std::string n7 = "", const AttributeValue& v7 = EmptyAttributeValue ());
  /**
   * Configures the E2 LTE UE.
   * \param tid the string representation of the ns3::TypeId associated with the model to set
   * \param n0 the name of the attribute to set
   * \param v0 the value of the attribute to set
   * \param n1 the name of the attribute to set
   * \param v1 the value of the attribute to set
   * \param n2 the name of the attribute to set
   * \param v2 the value of the attribute to set
   * \param n3 the name of the attribute to set
   * \param v3 the value of the attribute to set
   * \param n4 the name of the attribute to set
   * \param v4 the value of the attribute to set
   * \param n5 the name of the attribute to set
   * \param v5 the value of the attribute to set
   * \param n6 the name of the attribute to set
   * \param v6 the value of the attribute to set
   * \param n7 the name of the attribute to set
   * \param v7 the value of the attribute to set
   */
  virtual void SetE2LteUe (std::string tid,
      std::string n0 = "", const AttributeValue& v0 = EmptyAttributeValue (),
      std::string n1 = "", const AttributeValue& v1 = EmptyAttributeValue (),
      std::string n2 = "", const AttributeValue& v2 = EmptyAttributeValue (),
      std::string n3 = "", const AttributeValue& v3 = EmptyAttributeValue (),
      std::string n4 = "", const AttributeValue& v4 = EmptyAttributeValue (),
      std::string n5 = "", const AttributeValue& v5 = EmptyAttributeValue (),
      std::string n6 = "", const AttributeValue& v6 = EmptyAttributeValue (),
      std::string n7 = "", const AttributeValue& v7 = EmptyAttributeValue ());
private:
  ObjectFactory m_e2InterfaceFactory; //!< The E2 interface object factory.
  ObjectFactory m_e2LteUeFactory; //!< The E2 LTE UE object factory.
  ObjectFactory m_e2LteEnbFactory; //!< The E2 LTE UE object factory.
};

} // namespace ns3

#endif /* ORAN_HELPER_H */

