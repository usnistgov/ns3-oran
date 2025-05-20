// ru-application.h
#ifndef RU_APPLICATION_H
#define RU_APPLICATION_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/ipv4-address.h"
#include "ns3/vector.h"  // Include Vector for RU position

namespace ns3 {

class RuApplication : public Application {
public:

    void SetUavIp(Ipv4Address ip) { m_uavIp = ip; }
    void SetUavPort(uint16_t port) { m_uavPort = port; }

    static TypeId GetTypeId(void);
    RuApplication();
    virtual ~RuApplication();

    void SetListeningPort(uint16_t port); // Set the port for receiving data

    // Set and get RU position
    void SetRuPosition(Vector pos);
    Vector GetRuPosition() const;

    void SetActive(bool active); // Set RU active status

protected:
    virtual void StartApplication() override;
    virtual void StopApplication() override;

private:

    Ipv4Address m_uavIp;       // UAV IP to send RU IP info
    uint16_t m_uavPort;        // UAV port to send RU IP info
    void SendIpToUav();        // Method to send RU IP to UAV    

    void HandleReceive(Ptr<Socket> socket); // Receive packet handler

    Ptr<Socket> m_socket;          // Receiving socket
    uint16_t m_port = 500;               // Port to listen for packets
    uint32_t m_receivedPackets = 0;    // Count of received packets
    uint32_t m_gpsReceivedPackets = 0; // Count of received GPS packets
    bool m_active = false;                 // RU active state
    Vector m_position;             // Position of the RU
};

} // namespace ns3

#endif
