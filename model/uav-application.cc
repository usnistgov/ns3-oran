// uav-application.cc
#include "uav-application.h"
#include "ru-application.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/mobility-model.h"
#include "ns3/animation-interface.h"
#include <cmath>
#include "ns3/udp-socket-factory.h" // NEWLY ADDED
#include "ns3/packet.h"             // NEWLY ADDED

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("UavApplication");
NS_OBJECT_ENSURE_REGISTERED(UavApplication);

UavApplication::UavApplication()
    : m_currentActiveRuIndex(UINT32_MAX) // none selected initially
{
}

TypeId UavApplication::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::UavApplication")
        .SetParent<Application>()
        .SetGroupName("Tutorial")
        .AddConstructor<UavApplication>();
    return tid;
}

UavApplication::~UavApplication() {}

void UavApplication::SetRuApps(const std::vector<Ptr<RuApplication>>& ruApps) {
    m_ruApps = ruApps;
}

void UavApplication::SetRuNodes(const std::vector<Ptr<Node>>& ruNodes) {
    m_ruNodes = ruNodes;
}

//Sets a periodic timer (60 second) and to call CheckAndSelectBestRu
void UavApplication::StartApplication() {
    NS_LOG_INFO("UAV Application started");
    m_checkEvent = Simulator::Schedule(Seconds(60.0), &UavApplication::CheckAndSelectBestRu, this);
    //m_gpsSendEvent = Simulator::Schedule(Seconds(60.0), &UavApplication::SendGpsData, this);
}


void UavApplication::LogPosition() {
    Ptr<Node> node = GetNode();
    Ptr<MobilityModel> mobility = node->GetObject<MobilityModel>();
    Vector pos = mobility->GetPosition();
    NS_LOG_INFO("Time: " << Simulator::Now().GetMinutes() << " min, Position: "
                << pos.x << ":" << pos.y << ":" << pos.z);
    m_loggingEvent = Simulator::Schedule(Minutes(1), &UavApplication::LogPosition, this);
}

//Function to calculate signal strength received from RU, 

double UavApplication::CalculateSignalStrength(Vector uavPos, Vector ruPos, double txPowerDbm, 
    uint32_t txAntennas, uint32_t rxAntennas) {
double distance = CalculateDistance(uavPos, ruPos);
if (distance < 1.0) distance = 1.0; // avoid log(0)

// Free-Space Path Loss (FSPL)
double pathLossDb = 20 * std::log10(distance) + 20 * std::log10(3.5e9) - 147.55;

//ruPos and uavPos are 3D position vectors (x, y, z).Calculate how far the RU is relative to the UAV
Vector direction = ruPos - uavPos;
//angle is measured counterclockwise from the x-axis
double angle = std::atan2(direction.y, direction.x); // Direction angle (2D)
// Beamforming Gain Calculation
double beamformingGain = std::abs(std::cos(angle)); // Simplified Gain Calculation (cosine gain)

// MIMO Gain Calculation
double mimoGainDb = 10 * std::log10(txAntennas * rxAntennas); // MIMO Gain in dB

// Total Received Power Calculation
double rxPowerDbm = txPowerDbm - pathLossDb + 10 * std::log10(beamformingGain) + mimoGainDb;
return rxPowerDbm;
}

void UavApplication::CheckAndSelectBestRu() {
    Ptr<MobilityModel> uavMob = GetNode()->GetObject<MobilityModel>();
    Vector uavPos = uavMob->GetPosition();
    //txAntennas = 8; RU antennas (transmitter) 
    //rxAntennas = 2; // UAV antennas (receiver)
    double txPowerDbm = 30.0; // example Tx power
    uint32_t txAntennas = 8;
    uint32_t rxAntennas = 2;
    double bestIndex = -1; //initializes the variable that will store the index of the RU (Radio Unit) with the strongest signal
    double bestSignal = -1e2;//initializes the variable that will store the strongest signal value found so far 100dBm

    for (uint32_t i = 0; i < m_ruApps.size(); ++i) {
        Ptr<MobilityModel> ruMob = m_ruNodes[i]->GetObject<MobilityModel>();
        Vector ruPos = ruMob->GetPosition();
        double signal = CalculateSignalStrength(uavPos, ruPos, txPowerDbm, txAntennas, rxAntennas);
        NS_LOG_INFO("Signal to RU" << i+1 << " = " << signal << " dBm");

        if (signal > bestSignal) {
            bestSignal = signal;
            bestIndex = i;
        }
    }

    if (bestIndex != -1 && bestIndex != m_currentActiveRuIndex) {
        NS_LOG_INFO("Switching active RU" << bestIndex << " to RU" << bestIndex+1);

        // Deactivate previous RU
        if (m_currentActiveRuIndex != UINT32_MAX) {
            m_ruApps[m_currentActiveRuIndex]->SetActive(false);
            if (m_anim) {
                m_anim->UpdateNodeColor(m_ruNodes[m_currentActiveRuIndex], 200, 100, 50);
                m_anim->UpdateNodeSize(m_ruNodes[m_currentActiveRuIndex], 9, 9);
                m_anim->UpdateNodeDescription(m_ruNodes[m_currentActiveRuIndex], "RU" + std::to_string(m_currentActiveRuIndex + 1));
            }
        }

            // Activate new RU
            m_ruApps[bestIndex]->SetActive(true);
            if (m_anim) {
                m_anim->UpdateNodeColor(m_ruNodes[bestIndex], 0, 255, 0);
                m_anim->UpdateNodeSize(m_ruNodes[bestIndex], 12, 12);
                m_anim->UpdateNodeDescription(m_ruNodes[bestIndex], "RU" + std::to_string(bestIndex + 1) + " (Active)");
            }

        m_currentActiveRuIndex = bestIndex;

        // Immediately send GPS data to the active RU
        SendGpsDataToActiveRu();

    }

    // Schedule next check 60 seconds
    m_checkEvent = Simulator::Schedule(Seconds(60.0), &UavApplication::CheckAndSelectBestRu, this);
}

// void UavApplication::SendGpsData() {
//     Ptr<MobilityModel> uavMob = GetNode()->GetObject<MobilityModel>();
//     Vector uavPos = uavMob->GetPosition();

//     double lat = 37.7749 + (uavPos.y * 0.000009); // Approximate latitude change
//     double lon = -122.4194 + (uavPos.x * 0.000011); // Approximate longitude change

//     NS_LOG_INFO("GPS Data - Latitude: " << lat << ", Longitude: " << lon << ", Altitude: " << uavPos.z);

//     m_gpsSendEvent = Simulator::Schedule(Seconds(60.0), &UavApplication::SendGpsData, this);
// }

void UavApplication::SendGpsDataToActiveRu() {
    if (m_currentActiveRuIndex == UINT32_MAX) {
        NS_LOG_WARN("No active RU selected, cannot send GPS data.");
        return;
    }

    Ptr<MobilityModel> uavMob = GetNode()->GetObject<MobilityModel>();
    Vector uavPos = uavMob->GetPosition();
    double lat = 37.7749 + (uavPos.y * 0.000009);
    double lon = -122.4194 + (uavPos.x * 0.000011);
    double alt = uavPos.z;

    std::string gpsData = "GPS Data Sent - Latitude: " + std::to_string(lat) +
                          ", Longitude: " + std::to_string(lon) +
                          ", Altitude: " + std::to_string(alt);

    NS_LOG_INFO("Sending GPS Data to Active RU: " << gpsData);

    // Create a UDP socket
    Ptr<Socket> socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    // InetSocketAddress remoteAddr = InetSocketAddress(Ipv4Address::GetAny(), 500); // Use RU IP and port

    Ipv4Address ruIp = m_ruInterfaces.GetAddress(m_currentActiveRuIndex);
    InetSocketAddress remoteAddr = InetSocketAddress(ruIp, 500);
    NS_LOG_INFO("Sending GPS Data to IP: " << ruIp << " Port: 500");
    // Ensure the selected RU is the active one
    Ptr<Node> activeRuNode = m_ruNodes[m_currentActiveRuIndex];
    Ptr<Ipv4> ipv4 = activeRuNode->GetObject<Ipv4>();
    Ipv4Address ruAddress = ipv4->GetAddress(1, 0).GetLocal();
    remoteAddr.SetIpv4(ruAddress);

    socket->Connect(remoteAddr);

    // Send the GPS data as a packet
    Ptr<Packet> packet = Create<Packet>((uint8_t *)gpsData.c_str(), gpsData.size());
    socket->Send(packet);

    NS_LOG_INFO("GPS Data Packet Sent to RU " << m_currentActiveRuIndex + 1);
    socket->Close();
}



void UavApplication::StopApplication() {
    if (m_checkEvent.IsPending()) {
        Simulator::Cancel(m_checkEvent);
    }
    if (m_gpsSendEvent.IsPending()) {
        Simulator::Cancel(m_gpsSendEvent);
    }
}

} // namespace ns3