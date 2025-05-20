#ifndef UAV_APPLICATION_H
#define UAV_APPLICATION_H

#include "ns3/application.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/vector.h"
#include "ns3/animation-interface.h"
#include "ns3/ipv4-interface-container.h" 

namespace ns3 {

class RuApplication;  // Forward declare

class UavApplication : public Application {
public:
  static TypeId GetTypeId(void);
  UavApplication();
  virtual ~UavApplication();

  void SetVelocity(Vector velocity);
  void StartMovement();

  // Provide RU node and app containers for selection
  void SetRuApps(const std::vector<Ptr<RuApplication>>& ruApps);
  void SetRuNodes(const std::vector<Ptr<Node>>& ruNodes);
  //Add a pointer to AnimationInterface and a setter method
  void SetAnimationInterface(ns3::AnimationInterface* anim) {
    m_anim = anim;
  }
  
  // Setter for RU interfaces container (needed to get RU IP addresses)
    void SetRuInterfaces(ns3::Ipv4InterfaceContainer ruInterfaces) {
      m_ruInterfaces = ruInterfaces;
    }

private:
  virtual void StartApplication() override;
  virtual void StopApplication() override;

  void MoveUav();
  void LogPosition();

  Vector m_position;  // UAV Position (x, y, z)
  Vector m_velocity;  // UAV Velocity (vx, vy, vz)
  EventId m_movementEvent;  // Movement event
  EventId m_loggingEvent;   // Logging event

  void CheckAndSelectBestRu();
  double CalculateSignalStrength(Vector uavPos, Vector ruPos, double txPowerDbm, 
                                  uint32_t txAntennas, uint32_t rxAntennas);
  void SendGpsDataToActiveRu();

  
  EventId m_checkEvent;
  std::vector<Ptr<RuApplication>> m_ruApps;
  std::vector<Ptr<Node>> m_ruNodes;

  EventId m_gpsSendEvent;

  ns3::Ipv4InterfaceContainer m_ruInterfaces; // Container holding RU IP addresses

  ns3::AnimationInterface* m_anim = nullptr;  // Pointer to shared AnimationInterface
    
  uint32_t m_currentActiveRuIndex;
};

} // namespace ns3

#endif
