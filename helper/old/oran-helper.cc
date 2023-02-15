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

#include <ns3/node.h>
#include <ns3/pointer.h>
#include <ns3/lte-ue-rrc.h>

#include <ns3/oran-e2-interface.h>
#include <ns3/oran-e2-device.h>
#include <ns3/oran-e2-lte-enb.h>
#include <ns3/oran-e2-lte-ue.h>

#include "oran-helper.h"

namespace ns3 {

OranHelper::OranHelper (void)
{
  m_e2InterfaceFactory.SetTypeId (OranE2Interface::GetTypeId ());
  m_e2LteUeFactory.SetTypeId (OranE2LteUe::GetTypeId ());
  m_e2LteEnbFactory.SetTypeId (OranE2LteEnb::GetTypeId ());
}

OranHelper::~OranHelper (void)
{
}

NetDeviceContainer
OranHelper::ConnectLteUes (Ptr<OranNearRtRic> nearRtRic, NetDeviceContainer lteUeNetDevs)
{
  NetDeviceContainer container;
  for (uint32_t idx = 0; idx < lteUeNetDevs.GetN (); idx++)
    {
      Ptr<LteUeNetDevice> lteUeNetDev = lteUeNetDevs.Get (idx)->GetObject<LteUeNetDevice> ();
      Ptr<OranE2Interface> e2Interface = m_e2InterfaceFactory.Create<OranE2Interface> ();
      Ptr<OranE2LteUe> e2LteUe = m_e2LteUeFactory.Create<OranE2LteUe> ();

      e2LteUe->SetAttribute ("LteUeNetDevice", PointerValue (lteUeNetDev));
      e2LteUe->SetAttribute ("E2Interface", PointerValue (e2Interface));

      e2Interface->SetAttribute ("NearRtRic", PointerValue (nearRtRic));
      e2Interface->SetAttribute ("E2Device", PointerValue (e2LteUe->GetObject<OranE2Device> ()));

      nearRtRic->AddInterface (e2Interface);
      nearRtRic->RegisterLteUe (lteUeNetDev->GetNode ()->GetId (), lteUeNetDev->GetImsi ());

      container.Add (e2LteUe);
    }

  return container;
}

NetDeviceContainer
OranHelper::ConnectLteEnbs (Ptr<OranNearRtRic> nearRtRic, NetDeviceContainer lteEnbNetDevs)
{
  NetDeviceContainer container;
  for (uint32_t idx = 0; idx < lteEnbNetDevs.GetN (); idx++)
    {
      Ptr<LteEnbNetDevice> lteEnbNetDev = lteEnbNetDevs.Get (idx)->GetObject<LteEnbNetDevice> ();
      Ptr<OranE2Interface> e2Interface = m_e2InterfaceFactory.Create<OranE2Interface> ();
      Ptr<OranE2LteEnb> e2LteEnb = m_e2LteEnbFactory.Create<OranE2LteEnb> ();

      e2LteEnb->SetAttribute ("LteEnbNetDevice", PointerValue (lteEnbNetDev));
      e2LteEnb->SetAttribute ("E2Interface", PointerValue (e2Interface));

      e2Interface->SetAttribute ("NearRtRic", PointerValue (nearRtRic));
      e2Interface->SetAttribute ("E2Device", PointerValue (e2LteEnb->GetObject<OranE2Device> ()));

      nearRtRic->AddInterface (e2Interface);
      nearRtRic->RegisterLteEnb (lteEnbNetDev->GetNode ()->GetId (), lteEnbNetDev->GetCellId ());

      container.Add (e2LteEnb);
    }

  return container;
}

void
OranHelper::SetE2Interface (std::string tid,
    std::string n0, const AttributeValue& v0,
    std::string n1, const AttributeValue& v1,
    std::string n2, const AttributeValue& v2,
    std::string n3, const AttributeValue& v3,
    std::string n4, const AttributeValue& v4,
    std::string n5, const AttributeValue& v5,
    std::string n6, const AttributeValue& v6,
    std::string n7, const AttributeValue& v7)
{
  ObjectFactory factory;
  factory.SetTypeId (tid);
  factory.Set (n0, v0);
  factory.Set (n1, v1);
  factory.Set (n2, v2);
  factory.Set (n3, v3);
  factory.Set (n4, v4);
  factory.Set (n5, v5);
  factory.Set (n6, v6);
  factory.Set (n7, v7);
  m_e2InterfaceFactory = factory;
}

void
OranHelper::SetE2LteEnb (std::string tid,
    std::string n0, const AttributeValue& v0,
    std::string n1, const AttributeValue& v1,
    std::string n2, const AttributeValue& v2,
    std::string n3, const AttributeValue& v3,
    std::string n4, const AttributeValue& v4,
    std::string n5, const AttributeValue& v5,
    std::string n6, const AttributeValue& v6,
    std::string n7, const AttributeValue& v7)
{
  ObjectFactory factory;
  factory.SetTypeId (tid);
  factory.Set (n0, v0);
  factory.Set (n1, v1);
  factory.Set (n2, v2);
  factory.Set (n3, v3);
  factory.Set (n4, v4);
  factory.Set (n5, v5);
  factory.Set (n6, v6);
  factory.Set (n7, v7);
  m_e2LteEnbFactory = factory;
}

void
OranHelper::SetE2LteUe (std::string tid,
    std::string n0, const AttributeValue& v0,
    std::string n1, const AttributeValue& v1,
    std::string n2, const AttributeValue& v2,
    std::string n3, const AttributeValue& v3,
    std::string n4, const AttributeValue& v4,
    std::string n5, const AttributeValue& v5,
    std::string n6, const AttributeValue& v6,
    std::string n7, const AttributeValue& v7)
{
  ObjectFactory factory;
  factory.SetTypeId (tid);
  factory.Set (n0, v0);
  factory.Set (n1, v1);
  factory.Set (n2, v2);
  factory.Set (n3, v3);
  factory.Set (n4, v4);
  factory.Set (n5, v5);
  factory.Set (n6, v6);
  factory.Set (n7, v7);
  m_e2LteUeFactory = factory;
}

} // namespace ns3

