// ru-application.cc
#include "ru-application.h"
#include "ns3/log.h"
#include "ns3/socket.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/packet.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RuApplication");
NS_OBJECT_ENSURE_REGISTERED(RuApplication);

TypeId RuApplication::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::RuApplication")
        .SetParent<Application>()
        .SetGroupName("Oran")
        .AddConstructor<RuApplication>();
    return tid;
}

RuApplication::RuApplication() : m_port(500),m_receivedPackets(0), m_gpsReceivedPackets(0), m_active(false) {}

RuApplication::~RuApplication() {}

void RuApplication::StartApplication() {
    NS_LOG_INFO("RU Application Started");

    m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    m_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port));
    m_socket->SetRecvCallback(MakeCallback(&RuApplication::HandleReceive, this));
}

void RuApplication::HandleReceive(Ptr<Socket> socket) {
    Ptr<Packet> packet = socket->Recv();
    m_receivedPackets++;

    // Check the packet size
    uint32_t packetSize = packet->GetSize();
    if (packetSize == 0) {
        NS_LOG_WARN("Received an empty packet");
        return;
    }

    // Allocate buffer with null termination
    std::vector<uint8_t> buffer(packetSize + 1);
    packet->CopyData(buffer.data(), packetSize);
    buffer[packetSize] = '\0'; // Null-terminate the buffer

    std::string receivedData(reinterpret_cast<char *>(buffer.data()));

    NS_LOG_INFO("RU Received Packet: " << receivedData);

    // Check if the packet contains GPS data (simple keyword check)
    //When RU receives a UDP packet, it reads the packet data into receivedData as a string.
    //It checks if this string contains the phrase "GPS Data Sent".
    if (receivedData.find("GPS Data Sent") != std::string::npos) {
        m_gpsReceivedPackets++;
        NS_LOG_INFO("[RU] GPS Packets Received: " << m_gpsReceivedPackets);
    }
}

void RuApplication::SetRuPosition(Vector pos) {
    m_position = pos;
}

Vector RuApplication::GetRuPosition() const {
    return m_position;
}

void RuApplication::SetActive(bool active) {
    m_active = active;
    if (active) {
        NS_LOG_INFO("RU Activated");
    } else {
        NS_LOG_INFO("RU Deactivated");
    }
}

void RuApplication::StopApplication() {
    NS_LOG_INFO("RU Application Stopped");
    if (m_socket) {
        m_socket->Close();
    }
}

} // namespace ns3
