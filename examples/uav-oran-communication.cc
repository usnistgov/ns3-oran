#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/animation-interface.h"
#include "ns3/oran-module.h"
#include "ns3/wifi-module.h"

using namespace ns3;

// Function to calculate path loss (simple free-space model), signal frequency (3.5 GHz), change when we need
// double CalculateSignalStrength(Vector uavPos, Vector ruPos, double txPowerDbm) {
//     double distance = CalculateDistance(uavPos, ruPos);
//     double pathLossDb = 20 * std::log10(distance) + 20 * std::log10(3.5e9) - 147.55; // FSPL
//     double rxPowerDbm = txPowerDbm - pathLossDb;
//     return rxPowerDbm;
// }

int main(int argc, char *argv[]) {
    //define logs
    LogComponentEnable("RuApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UavApplication", LOG_LEVEL_INFO);
    
   // LogComponentEnable("RuApplication", LOG_LEVEL_DEBUG);

    //Create the UAV node
    NodeContainer uavFleet, ruFleet;
    uavFleet.Create(1);
    ruFleet.Create(3);

    InternetStackHelper internet;
    internet.Install(uavFleet);
    internet.Install(ruFleet);

    // Wi-Fi Channel Setup
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211ac);

    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper wifiPhy;
    wifiPhy.SetChannel(wifiChannel.Create());

    WifiMacHelper wifiMac;
    Ssid ssid = Ssid("uav-ru-network");


    // UAV as Station
    wifiMac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer uavDevices = wifi.Install(wifiPhy, wifiMac, uavFleet);

    // RUs as Access Points
    wifiMac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer ruDevices = wifi.Install(wifiPhy, wifiMac, ruFleet);


    // IP Assignment
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer uavInterfaces = ipv4.Assign(uavDevices);
    Ipv4InterfaceContainer ruInterfaces = ipv4.Assign(ruDevices);

    // // LOG RU IPs to confirm correct assignment
    // for (uint32_t i = 0; i < ruInterfaces.GetN(); ++i) {
    //     NS_LOG_INFO("RU " << i << " IP: " << ruInterfaces.GetAddress(i));
    // }

    //Create 6 “marker” nodes for waypoints
    NodeContainer wpNodes;
    wpNodes.Create(6);

    //Define the UAV waypoints
    std::vector<Vector> pts = {
        {  0.0,   0.0,   0.0},  // A
        {100.0,   0.0,   0.0},  // B
        {100.0, 100.0,   0.0},  // C
        {  0.0, 100.0,   0.0},  // D
        {  0.0, 200.0,   0.0},  // E
        {100.0, 200.0,   0.0}   // F
    };

    //Install constant (stationary) mobility on waypoint markers
    MobilityHelper mobWp;
    mobWp.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    // position allocator matching our pts array order
    Ptr<ListPositionAllocator> posAlloc = CreateObject<ListPositionAllocator>();
    for (auto &v : pts) { posAlloc->Add(v); }
    mobWp.SetPositionAllocator(posAlloc);
    mobWp.Install(wpNodes);    

    //Install WaypointMobility, The UAV moves to each waypoint every 2 minutes (120 seconds).
    MobilityHelper mobUav;
    mobUav.SetMobilityModel("ns3::WaypointMobilityModel");
    mobUav.Install(uavFleet);
    Ptr<WaypointMobilityModel> waypointModel = uavFleet.Get(0)->GetObject<WaypointMobilityModel>();
    for (uint32_t i = 0; i < pts.size(); ++i) {
        //each waypoint is spaced by 120 seconds (2 minutes)
        waypointModel->AddWaypoint(Waypoint(Seconds(i * 120.0), pts[i]));
    }

    // Define RU positions
    std::vector<Vector> ruPositions = {
        {150, 0, 0}, 
        {150, 100, 0}, 
        {150, 200, 0}
    };

    // Mobility for RUs (fixed positions)
    MobilityHelper mobRu;
        mobRu.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        Ptr<ListPositionAllocator> ruPos = CreateObject<ListPositionAllocator>();
    for (auto &pos : ruPositions) { ruPos->Add(pos); }
        mobRu.SetPositionAllocator(ruPos);
        mobRu.Install(ruFleet);  
    
    std::vector<Ptr<RuApplication>> ruApps;
    std::vector<Ptr<Node>> ruNodes;
    for (uint32_t i = 0; i < ruFleet.GetN(); ++i) {
            Ptr<RuApplication> ruApp = CreateObject<RuApplication>();
            ruApp->SetRuPosition(ruFleet.Get(i)->GetObject<MobilityModel>()->GetPosition());
            ruFleet.Get(i)->AddApplication(ruApp);
            ruApp->SetStartTime(Seconds(0.0));
            ruApp->SetStopTime(Seconds(600.0));
            ruApps.push_back(ruApp);
            ruNodes.push_back(ruFleet.Get(i));
    }

    //Install the UAV application
    Ptr<UavApplication> uavApp = CreateObject<UavApplication>();
    uavFleet.Get(0)->AddApplication(uavApp);
    uavApp->SetRuApps(ruApps);
    uavApp->SetRuNodes(ruNodes);
    uavApp->SetRuInterfaces(ruInterfaces);
    uavApp->SetStartTime(Seconds(0.0));
    uavApp->SetStopTime(Seconds(600.0));


    //NetAnim setup
    AnimationInterface anim("results/uav-oran-communication.xml");
    anim.SetMobilityPollInterval(Seconds(1));

    // Label the UAV node
    anim.UpdateNodeDescription(uavFleet.Get(0), "UAV");
    //adjust UAV node size
    anim.UpdateNodeSize(uavFleet.Get(0), 6,6);

    // Label each waypoint marker A–F
    static const char *labels[] = {"A","B","C","D","E","F"};
    for (uint32_t i = 0; i < wpNodes.GetN(); ++i) {
        anim.UpdateNodeDescription(wpNodes.Get(i), labels[i]);
        //Increase waypoint node size
        anim.UpdateNodeSize(wpNodes.Get(i), 3,3);
        //distinct color
        anim.UpdateNodeColor(wpNodes.Get(i), 255 - i * 40, i * 40, 255 - i * 30);
    }
    //set ru description,color and size
    for (uint32_t i = 0; i < ruFleet.GetN(); ++i) {
        anim.UpdateNodeDescription(ruFleet.Get(i), "RU" + std::to_string(i+1));
        anim.UpdateNodeColor(ruFleet.Get(i), 200, 100, 50);
        anim.UpdateNodeSize(ruFleet.Get(i), 9,9);
    }

    // Pass pointer to the AnimationInterface object
    uavApp->SetAnimationInterface(&anim);
    


    //Run
    Simulator::Stop(Seconds(600.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}