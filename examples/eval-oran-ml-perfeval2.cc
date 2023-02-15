/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Jaume Nin <jaume.nin@cttc.cat>
 *          Manuel Requena <manuel.requena@cttc.es>
 */
#ifdef NS3_TORCHLIB

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/oran-module.h"
#include <math.h>

#include <torch/script.h>

using namespace ns3;
using namespace psc;

std::map<uint16_t, uint64_t> total_tx_pkts;
std::map<uint16_t, uint64_t> total_rx_pkts;
std::map<uint16_t, uint64_t> total_tx_bytes;
std::map<uint16_t, uint64_t> total_rx_bytes;

uint64_t txpkts1, rxpkts1;
uint64_t txpkts2, rxpkts2;
uint64_t txpkts3, rxpkts3;
uint64_t txpkts4, rxpkts4;
double oldLossPkts21, oldLossBytes21, oldLossPkts31, oldLossBytes31;
double oldLossPkts22, oldLossBytes22, oldLossPkts32, oldLossBytes32;
double oldLossPkts23, oldLossBytes23, oldLossPkts33, oldLossBytes33;
double oldLossPkts24, oldLossBytes24, oldLossPkts34, oldLossBytes34;
uint16_t enbUe2;
uint16_t enbUe3;

uint32_t oldRbsUe21, oldRbsUe31, oldRbsCell11, oldRbsCell21;
uint32_t oldRbsUe22, oldRbsUe32, oldRbsCell12, oldRbsCell22;
uint32_t oldRbsUe23, oldRbsUe33, oldRbsCell13, oldRbsCell23;
uint32_t oldRbsUe24, oldRbsUe34, oldRbsCell14, oldRbsCell24;
uint32_t oldRbsUe25, oldRbsUe35, oldRbsCell15, oldRbsCell25;

torch::jit::script::Module model;

void
NotifyHandoverEndOkEnb (std::string context, uint64_t imsi, uint16_t cellid, uint16_t rnti)
{
  std::ofstream hoOutFile ("handover_trace.txt", std::ios_base::app);
  hoOutFile << Simulator::Now ().GetSeconds () << " " << imsi << " " << cellid << " " << rnti << std::endl;
}

void
CheckLoss (Ptr<LteHelper> lteHelper, NetDeviceContainer ueLteDevs, NetDeviceContainer enbLteDevs, NodeContainer ueNodes, NodeContainer enbNodes)
{
  //std::ofstream hoOutFile ("handover_trace.txt", std::ios_base::app);

  float du1c1, du1c2, du2c1, du2c2, du3c1, du3c2, du4c1, du4c2;

  Vector posU1 = ueNodes.Get (0)->GetObject<MobilityModel> ()->GetPosition ();
  Vector posU2 = ueNodes.Get (1)->GetObject<MobilityModel> ()->GetPosition ();
  Vector posU3 = ueNodes.Get (2)->GetObject<MobilityModel> ()->GetPosition ();
  Vector posU4 = ueNodes.Get (3)->GetObject<MobilityModel> ()->GetPosition ();
  
  Vector posC1 = enbNodes.Get (0)->GetObject<MobilityModel> ()->GetPosition ();
  Vector posC2 = enbNodes.Get (1)->GetObject<MobilityModel> ()->GetPosition ();

  du1c1 = sqrt (pow (posU1.x - posC1.x, 2) + pow (posU1.y - posC1.y, 2) );
  du1c2 = sqrt (pow (posU1.x - posC2.x, 2) + pow (posU1.y - posC2.y, 2) );
  du2c1 = sqrt (pow (posU2.x - posC1.x, 2) + pow (posU2.y - posC1.y, 2) );
  du2c2 = sqrt (pow (posU2.x - posC2.x, 2) + pow (posU2.y - posC2.y, 2) );
  du3c1 = sqrt (pow (posU3.x - posC1.x, 2) + pow (posU3.y - posC1.y, 2) );
  du3c2 = sqrt (pow (posU3.x - posC2.x, 2) + pow (posU3.y - posC2.y, 2) );
  du4c1 = sqrt (pow (posU4.x - posC1.x, 2) + pow (posU4.y - posC1.y, 2) );
  du4c2 = sqrt (pow (posU4.x - posC2.x, 2) + pow (posU4.y - posC2.y, 2) );

  float newLossPktsUe1 = txpkts1 > 0 ? (rxpkts1 > txpkts1 ? 0 : (txpkts1 - rxpkts1) * 1.0 / txpkts1) : 0;
  float newLossPktsUe2 = txpkts2 > 0 ? (rxpkts2 > txpkts2 ? 0 : (txpkts2 - rxpkts2) * 1.0 / txpkts2) : 0;
  float newLossPktsUe3 = txpkts3 > 0 ? (rxpkts3 > txpkts3 ? 0 : (txpkts3 - rxpkts3) * 1.0 / txpkts3) : 0;
  float newLossPktsUe4 = txpkts4 > 0 ? (rxpkts4 > txpkts4 ? 0 : (txpkts4 - rxpkts4) * 1.0 / txpkts4) : 0;

  //hoOutFile << Simulator::Now ().GetSeconds () << " UE1 " << enbUe2 << std::endl;
  //hoOutFile << Simulator::Now ().GetSeconds () << " UE2 " << enbUe3 << std::endl;

  // Get the classification result
  //std::vector<torch::jit::IValue> inputs;
  //std::vector<float> inputv = {
    //du1c1, du1c2, newLossPktsUe1, 
    //du2c1, du2c2, newLossPktsUe2, 
    //du3c1, du3c2, newLossPktsUe3, 
    //du4c1, du4c2, newLossPktsUe4
  //};
  //inputs.push_back (torch::from_blob (inputv.data (), {1, 12}).to (torch::kFloat32));
  //at::Tensor output = model.forward(inputs).toTensor();

  std::cout << Simulator::Now ().GetSeconds ()
    << " " << du1c1 << " " << du1c2 << " " << newLossPktsUe1
    << " " << du2c1 << " " << du2c2 << " " << newLossPktsUe2
    << " " << du3c1 << " " << du3c2 << " " << newLossPktsUe3
    << " " << du4c1 << " " << du4c2 << " " << newLossPktsUe4
    << std::endl;

  //int configuration = output.argmax (1).item ().toInt ();
  //std::cout << Simulator::Now ().GetSeconds () << "\tConfiguration: " << configuration << std::endl;
  /*
  switch (configuration)
    {
      case 0:
        // UE 2 in Cell 1
        if (enbUe2 == 1)
          {
            lteHelper->HandoverRequest (Seconds (0), ueLteDevs.Get (1), enbLteDevs.Get (1), enbLteDevs.Get (0));
            enbUe2 = 0;
            hoOutFile << Simulator::Now ().GetSeconds () << " UE1 " << enbUe2 << std::endl;
          }
        // UE 3 in Cell 1
        if (enbUe3 == 1)
          {
            lteHelper->HandoverRequest (Seconds (0), ueLteDevs.Get (2), enbLteDevs.Get (1), enbLteDevs.Get (0));
            enbUe3 = 0;
            hoOutFile << Simulator::Now ().GetSeconds () << " UE2 " << enbUe3 << std::endl;
          }
        break;
      case 1:
        // UE 2 in Cell 1
        if (enbUe2 == 1)
          {
            lteHelper->HandoverRequest (Seconds (0), ueLteDevs.Get (1), enbLteDevs.Get (1), enbLteDevs.Get (0));
            enbUe2 = 0;
            hoOutFile << Simulator::Now ().GetSeconds () << " UE1 " << enbUe2 << std::endl;
          }
        // UE 3 in Cell 2
        if (enbUe3 == 0)
          {
            lteHelper->HandoverRequest (Seconds (0), ueLteDevs.Get (2), enbLteDevs.Get (0), enbLteDevs.Get (1));
            enbUe3 = 1;
            hoOutFile << Simulator::Now ().GetSeconds () << " UE2 " << enbUe3 << std::endl;
          }
        break;
      case 2:
        // UE 2 in Cell 2
        if (enbUe2 == 0)
          {
            lteHelper->HandoverRequest (Seconds (0), ueLteDevs.Get (1), enbLteDevs.Get (0), enbLteDevs.Get (1));
            enbUe2 = 1;
            hoOutFile << Simulator::Now ().GetSeconds () << " UE1 " << enbUe2 << std::endl;
          }
        // UE 3 in Cell 1
        if (enbUe3 == 1)
          {
            lteHelper->HandoverRequest (Seconds (0), ueLteDevs.Get (2), enbLteDevs.Get (1), enbLteDevs.Get (0));
            enbUe3 = 0;
            hoOutFile << Simulator::Now ().GetSeconds () << " UE2 " << enbUe3 << std::endl;
          }
        break;
      case 3:
        // UE 2 in Cell 2
        if (enbUe2 == 0)
          {
            lteHelper->HandoverRequest (Seconds (0), ueLteDevs.Get (1), enbLteDevs.Get (0), enbLteDevs.Get (1));
            enbUe2 = 1;
            hoOutFile << Simulator::Now ().GetSeconds () << " UE1 " << enbUe2 << std::endl;
          }
        // UE 3 in Cell 2
        if (enbUe3 == 0)
          {
            lteHelper->HandoverRequest (Seconds (0), ueLteDevs.Get (2), enbLteDevs.Get (0), enbLteDevs.Get (1));
            enbUe3 = 1;
            hoOutFile << Simulator::Now ().GetSeconds () << " UE2 " << enbUe3 << std::endl;
          }
        break;
    }
  */

  txpkts1 = rxpkts1 = 0;
  txpkts2 = rxpkts2 = 0;
  txpkts3 = rxpkts3 = 0;
  txpkts4 = rxpkts4 = 0;

  Simulator::Schedule (Seconds (1), &CheckLoss, lteHelper, ueLteDevs, enbLteDevs, ueNodes, enbNodes);
}

void
TracePositions (NodeContainer nodes) 
{
  std::ofstream posOutFile ("positions_trace.txt", std::ios_base::app);
  
  posOutFile << Simulator::Now ().GetSeconds ();
  for (uint32_t i = 0; i < nodes.GetN (); i++) 
    {
      Vector pos = nodes.Get (i)->GetObject<MobilityModel>()->GetPosition ();
      posOutFile << " " << pos.x << " " << pos.y;
    }
  posOutFile << std::endl;

  Simulator::Schedule (Seconds (1), &TracePositions, nodes);
}

// Function that will save the traces of RX'd packets 
void
RxTrace (Ptr<const Packet> p, const Address &from, const Address &to)
{
  //std::ofstream rxOutFile ("rx_trace.txt", std::ios_base::app);
  /*
  std::ofstream rxOutFile ("traffic_trace.txt", std::ios_base::app);

  uint16_t ueId = (InetSocketAddress::ConvertFrom (to).GetPort () / 1000) - 1;

  rxOutFile << Simulator::Now ().GetSeconds () << " UE " << ueId << " RX " << p->GetSize () << " Bytes from " << InetSocketAddress::ConvertFrom (from).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (from).GetPort () << " to " << InetSocketAddress::ConvertFrom (to).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (to).GetPort () << std::endl;
  */
  uint16_t ueId = (InetSocketAddress::ConvertFrom (to).GetPort () / 1000) - 1;

  std::ofstream rxOutFile ("traffic_trace.txt", std::ios_base::app);
  rxOutFile << Simulator::Now ().GetSeconds () << " " << ueId << " RX " << p->GetSize () << std::endl;

  total_rx_pkts [ueId] ++;
  total_rx_bytes [ueId] += p->GetSize ();

  switch (ueId)
    {
      case 0:
        rxpkts1 ++;
        break;
      case 1:
        rxpkts2 ++;
        break;
      case 2:
        rxpkts3 ++;
        break;
      case 3:
        rxpkts4 ++;
        break;
    }
}


// Function that will save the traces of TX'd packets 
void
TxTrace (Ptr<const Packet> p, const Address &from, const Address &to)
{
  //std::ofstream txOutFile ("tx_trace.txt", std::ios_base::app);
  /*
  std::ofstream txOutFile ("traffic_trace.txt", std::ios_base::app);

  uint16_t ueId = (InetSocketAddress::ConvertFrom (to).GetPort () / 1000) - 1;

  txOutFile << Simulator::Now ().GetSeconds () << " UE " << ueId << " TX " << p->GetSize () << " Bytes from " << InetSocketAddress::ConvertFrom (from).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (from).GetPort () << " to " << InetSocketAddress::ConvertFrom (to).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (to).GetPort () << std::endl;
  */
  uint16_t ueId = (InetSocketAddress::ConvertFrom (to).GetPort () / 1000) - 1;

  std::ofstream rxOutFile ("traffic_trace.txt", std::ios_base::app);
  rxOutFile << Simulator::Now ().GetSeconds () << " " << ueId << " TX " << p->GetSize () << std::endl;

  total_tx_pkts [ueId] ++;
  total_tx_bytes [ueId] += p->GetSize ();

  switch (ueId)
    {
      case 0:
        txpkts1 ++;
        break;
      case 1:
        txpkts2 ++;
        break;
      case 2:
        txpkts3 ++;
        break;
      case 3:
        txpkts4 ++;
        break;
    }

}


// Function that will save the traces of allocated RBs in the DL 
/*
void
DlRbTrace (uint16_t cellId, FfMacSchedSapUser::SchedDlConfigIndParameters ind)
{
  std::ofstream dlrbsOutFile ("dlrbs_trace.txt", std::ios_base::app);

  uint16_t acumRbsCell = 0;
  uint32_t imsi;

  if (ind.m_buildDataList.size () > 0)
    {
      dlrbsOutFile << Simulator::Now ().GetSeconds () << " CELLID " << cellId;
      for (unsigned int i = 0; i < ind.m_buildDataList.size (); i++)
        {
          uint16_t numallocatedrbgs = 0;
          uint64_t bitmapCopy = ind.m_buildDataList.at (i).m_dci.m_rbBitmap;
          while (bitmapCopy)
            {
              numallocatedrbgs += bitmapCopy & 1;
              bitmapCopy >>= 1;
            }
          acumRbsCell += numallocatedrbgs;

          imsi = LteIdMapperHelper::GetInstance ()->Rnti2Imsi (cellId, ind.m_buildDataList.at (i).m_rnti);

          dlrbsOutFile << " RNTI " << ind.m_buildDataList.at (i).m_rnti;
          dlrbsOutFile << " IMSI " << imsi;
          dlrbsOutFile << " MCS " << (uint16_t) ind.m_buildDataList.at (i).m_dci.m_mcs.at (0);
          dlrbsOutFile << " RBS " << numallocatedrbgs;

          if (imsi == 2) 
            {
              oldRbsUe21 += numallocatedrbgs;
            }
          else
            {
              if (imsi == 3)
                {
                  oldRbsUe31 += numallocatedrbgs;
                }
            }
        }
      dlrbsOutFile << std::endl;
    }

  if (cellId == 1) 
    {
      oldRbsCell11 += acumRbsCell;
    }
  else 
    {
      oldRbsCell21 += acumRbsCell;
    }
}
*/

// Function that will save the traces of allocated RBs in the UL 
void
UlRbTrace (const uint16_t rnti, const uint8_t rbs)
{
  std::ofstream ulrbsOutFile ("ulrbs_trace.txt", std::ios_base::app);

  ulrbsOutFile << Simulator::Now ().GetSeconds () << " RNTI " << rnti << " RBs " << (uint16_t)rbs << std::endl;
}


// Function that will save the DL buffer status as seen by the scheduler
void
DlBufferStatusTrace (const uint16_t rnti, const uint8_t lcid, 
    const uint32_t txQueueSize, const uint16_t txQueueHolDelay,
    const uint32_t rtxQueueSize, const uint16_t rtxQueueHolDelay) 
{
  std::ofstream dlBufferOutFile ("dlbuffer_trace.txt", std::ios_base::app);

  dlBufferOutFile << Simulator::Now ().GetSeconds () << " BUFFERSTATUS" <<
    " RNTI " << rnti << " LCID " << ((uint16_t)lcid) << 
    " TXSIZE " << txQueueSize << " TXHOLDELAY " << txQueueHolDelay <<
    " RTXSIZE " << rtxQueueSize << " RTXHOLDELAY " << rtxQueueHolDelay << std::endl;
}


// Function that will save the UL buffer status as seen by the scheduler
void
UlBufferStatusTrace (const uint16_t rnti, const uint32_t queueSize)
{
  std::ofstream ulBufferOutFile ("ulbuffer_trace.txt", std::ios_base::app);

  ulBufferOutFile << Simulator::Now ().GetSeconds () << " ULBUFFERSTATUS" << 
    " RNTI " << rnti << " QUEUESIZE " << queueSize << std::endl;
}





NS_LOG_COMPONENT_DEFINE ("Scenario1");

int
main (int argc, char *argv[])
{
  bool verbose = false;
  bool useMl = true;
  bool useOran = true;
  bool printLoss = false;
  uint32_t startConfig = 1;
  double lmQueryInterval = 1;
  double txDelay = 0;
  Time simTime = Seconds (100);
  std::string dbFileName = "oran-repository.db";

  CommandLine cmd;
  cmd.AddValue ("verbose", "Enable printing SQL queries results", verbose);
  cmd.AddValue ("use-ml", "Indicates whether ML should be used or not (must use ORAN)", useMl);
  cmd.AddValue ("use-oran", "Indicates whether ORAN should be used or not", useOran);
  cmd.AddValue ("start-config", "The starting configuration", startConfig);
  cmd.AddValue ("sim-time", "The duration for which traffic should flow", simTime);
  cmd.AddValue ("lm-query-interval", "The LM query interval", lmQueryInterval);
  cmd.AddValue ("tx-delay", "The E2 termiantor's transmission delay", txDelay);
  cmd.AddValue ("print-loss", "Run \"CheckLoss\" and print statistics to std output", printLoss);
  cmd.Parse (argc, argv);

  try 
    {
      //model = torch::jit::load ("saved_pytorch_short_trained2.pt");
      model = torch::jit::load ("saved_trained_classification_pytorch.pt");
    }
  catch (const c10::Error& e)
    {
      std::cerr << "Error loading the trained model" << std::endl;
      return -1;
    }

  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (1000 * 1024));
  Config::SetDefault ("ns3::LteUePhy::EnableRlfDetection", BooleanValue (false));

  // Configure the LTE parameters (pathloss, bandwidth, scheduler)
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::Cost231PropagationLossModel"));
  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (25));
  lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (25));
  //lteHelper->SetAttribute ("AllocationType", StringValue ("AllocationType2"));
  lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");
  lteHelper->SetSchedulerAttribute ("HarqEnabled", BooleanValue (true));
  //lteHelper->SetHandoverAlgorithmType ("ns3::A3RsrpHandoverAlgorithm");
  lteHelper->SetHandoverAlgorithmType ("ns3::NoOpHandoverAlgorithm");

  // Deploy the EPC 
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  // Create a single remote host
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // IP configuration 
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (65000));
  p2ph.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (0)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  //Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  // Create eNB and UE
  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create (2);
  ueNodes.Create (4);

  // Install Mobility Model for eNB (Constant Position at (0, 0, 0)
  Ptr<ListPositionAllocator> positionAllocEnbs = CreateObject<ListPositionAllocator> ();
  positionAllocEnbs->Add (Vector (0, 0, 0));
  positionAllocEnbs->Add (Vector (265, 0, 0));
  MobilityHelper mobilityEnbs;
  mobilityEnbs.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobilityEnbs.SetPositionAllocator(positionAllocEnbs);
  mobilityEnbs.Install(enbNodes);

  // Install Mobility Model for UE (Constant Positions)
  Ptr<ListPositionAllocator> positionAllocUes = CreateObject<ListPositionAllocator> ();
  positionAllocUes->Add (Vector (100, 0, 0));
  positionAllocUes->Add (Vector (190, 0, 0));
  positionAllocUes->Add (Vector (210, 0, 0));
  positionAllocUes->Add (Vector (300, 0, 0));
  MobilityHelper mobilityUes;
  /*
  mobilityUes.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobilityUes.SetPositionAllocator(positionAllocUes);
  mobilityUes.Install(ueNodes.Get (0));
  mobilityUes.Install(ueNodes.Get (1));
  mobilityUes.Install(ueNodes.Get (2));
  mobilityUes.Install(ueNodes.Get (3));
  */

  // Mobility Model for UE 0
  Ptr<RandomVariableStream> speedRvs = CreateObjectWithAttributes <UniformRandomVariable> ("Min", DoubleValue (1), "Max", DoubleValue (2.5));
  Ptr<RandomVariableStream> pauseRvs = CreateObjectWithAttributes <UniformRandomVariable> ("Min", DoubleValue (1), "Max", DoubleValue (6));
  mobilityUes.SetMobilityModel("ns3::RandomDirection2dMobilityModel", "Bounds", RectangleValue (Rectangle (-180, -10, -120, 120)), "Speed", PointerValue (speedRvs), "Pause", PointerValue (pauseRvs));
  mobilityUes.Install (ueNodes.Get (0));

  // Mobility Model for UE 1
  mobilityUes.SetMobilityModel("ns3::RandomDirection2dMobilityModel", "Bounds", RectangleValue (Rectangle (60, 190, -60, 80)), "Speed", PointerValue (speedRvs), "Pause", PointerValue (pauseRvs));
  mobilityUes.Install (ueNodes.Get (1));

  // Mobility Model for UE 2
  mobilityUes.SetMobilityModel("ns3::RandomDirection2dMobilityModel", "Bounds", RectangleValue (Rectangle (120, 180, -120, 110)), "Speed", PointerValue (speedRvs), "Pause", PointerValue (pauseRvs));
  mobilityUes.Install (ueNodes.Get (2));

  // Mobility Model for UE 3
  mobilityUes.SetMobilityModel("ns3::RandomDirection2dMobilityModel", "Bounds", RectangleValue (Rectangle (230, 390, -180, 120)), "Speed", PointerValue (speedRvs), "Pause", PointerValue (pauseRvs));
  mobilityUes.Install (ueNodes.Get (3));


  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

  // Connect the RB utilization traces to the eNB MAC
  //DynamicCast<LteEnbNetDevice> (enbLteDevs.Get (0))->GetMac ()->TraceConnectWithoutContext ("DlAllocation", MakeCallback (&DlRbTrace));
  //DynamicCast<LteEnbNetDevice> (enbLteDevs.Get (1))->GetMac ()->TraceConnectWithoutContext ("DlAllocation", MakeCallback (&DlRbTrace));
  //DynamicCast<LteEnbNetDevice> (enbLteDevs.Get (0))->GetMac ()->TraceConnectWithoutContext ("UlRbsScheduled", MakeCallback (&UlRbTrace));

  //DynamicCast<RrFfMacScheduler> (DynamicCast<ComponentCarrierEnb> (DynamicCast<LteEnbNetDevice> (enbLteDevs.Get (0))->GetCcMap ().begin ()->second)->GetFfMacScheduler ())->TraceConnectWithoutContext ("DlBufferStatus", MakeCallback (&DlBufferStatusTrace));
  //DynamicCast<RrFfMacScheduler> (DynamicCast<ComponentCarrierEnb> (DynamicCast<LteEnbNetDevice> (enbLteDevs.Get (0))->GetCcMap ().begin ()->second)->GetFfMacScheduler ())->TraceConnectWithoutContext ("UlBufferStatus", MakeCallback (&UlBufferStatusTrace));

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  switch (startConfig)
    {
      case 0:
        lteHelper->Attach (ueLteDevs.Get(0), enbLteDevs.Get(0));
        lteHelper->Attach (ueLteDevs.Get(1), enbLteDevs.Get(0));
        lteHelper->Attach (ueLteDevs.Get(2), enbLteDevs.Get(0));
        lteHelper->Attach (ueLteDevs.Get(3), enbLteDevs.Get(1));
        break;
      case 1:
        lteHelper->Attach (ueLteDevs.Get(0), enbLteDevs.Get(0));
        lteHelper->Attach (ueLteDevs.Get(1), enbLteDevs.Get(0));
        lteHelper->Attach (ueLteDevs.Get(2), enbLteDevs.Get(1));
        lteHelper->Attach (ueLteDevs.Get(3), enbLteDevs.Get(1));
        break;
      case 2:
        lteHelper->Attach (ueLteDevs.Get(0), enbLteDevs.Get(0));
        lteHelper->Attach (ueLteDevs.Get(1), enbLteDevs.Get(1));
        lteHelper->Attach (ueLteDevs.Get(2), enbLteDevs.Get(0));
        lteHelper->Attach (ueLteDevs.Get(3), enbLteDevs.Get(1));
        break;
      case 3:
        lteHelper->Attach (ueLteDevs.Get(0), enbLteDevs.Get(0));
        lteHelper->Attach (ueLteDevs.Get(1), enbLteDevs.Get(1));
        lteHelper->Attach (ueLteDevs.Get(2), enbLteDevs.Get(1));
        lteHelper->Attach (ueLteDevs.Get(3), enbLteDevs.Get(1));
        break;
      default:
        NS_ABORT_MSG ("Starting configuration " << startConfig << " not supported.");
    }

  enbUe2 = 0;
  enbUe3 = 1;

  /*
  for (uint16_t i = 0; i < ueNodes.GetN (); i++)
    {
      lteHelper->Attach (ueLteDevs.Get(i), enbLteDevs.Get(0));
      // side effect: the default EPS bearer will be activated
    }
  */

  lteHelper->AddX2Interface (enbNodes);

  // Install and start applications on UEs and remote host
  // The applications will change the demand every 10 seconds by reducing the 
  // inter-packet interval
  uint16_t basePort = 1000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  for (uint16_t i = 0; i < ueNodes.GetN (); i++)
    {
      uint16_t port = basePort * (i + 1);

      PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
      serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get (i)));
      // Enable the tracing of RX packets
      serverApps.Get (i)->TraceConnectWithoutContext("RxWithAddresses", MakeCallback (&RxTrace));

      Ptr <PscVideoStreaming> streamingServer = CreateObject <PscVideoStreaming> ();
      clientApps.Add (streamingServer);
      // Attributes
      streamingServer->SetAttribute ("ReceiverAddress", AddressValue (ueIpIface.GetAddress (i)));
      streamingServer->SetAttribute ("ReceiverPort", UintegerValue (port));
      streamingServer->SetAttribute ("Distribution", StringValue ("1080p-bright"));
      streamingServer->SetAttribute ("BoostLengthPacketCount", UintegerValue (0));
      streamingServer->SetAttribute ("BoostPercentile", DoubleValue (50));
      streamingServer->SetAttribute ("MaxUdpDatagramSize", UintegerValue (1500));

      remoteHost->AddApplication (streamingServer);
      streamingServer->TraceConnectWithoutContext ("TxWithAddresses", MakeCallback (&TxTrace));
    }

  // Servers start listening at 5 seconds
  serverApps.Start (Seconds (1));
  // Clients start transmitting at 10 seconds
  clientApps.Start (Seconds (2));
  // Clients stop transmitting at 1010 seconds
  //clientApps.Stop (Seconds (1010));
  clientApps.Stop (simTime + Seconds (10));
  // Servers stop listening at 1015 seconds
  //serverApps.Stop (Seconds (1015));
  serverApps.Stop (simTime + Seconds (15));

  // ORAN BEGIN
  if (useOran == true)
    {
      if (! dbFileName.empty ())
        {
          std::remove (dbFileName.c_str ());
        }

      Ptr<OranLm> defaultLm = nullptr;
      Ptr<OranDataRepository> dataRepository = CreateObject<OranDataRepositorySqlite> ();
      Ptr<OranCmm> cmm = CreateObject<OranCmmHandover> ();
      Ptr<OranNearRtRic> nearRtRic = CreateObject<OranNearRtRic> ();
      Ptr<OranNearRtRicE2Terminator> nearRtRicE2Terminator = CreateObject<OranNearRtRicE2Terminator> ();
 
      if (useMl == true)
        {
          defaultLm = CreateObject<OranLmLte2LteMlHandover> ();
        }
      else
        {
          //defaultLm = CreateObject<OranLmNoop> ();
          defaultLm = CreateObject<OranLmLte2LteDistanceHandover> ();
        }

    dataRepository->SetAttribute ("DatabaseFile", StringValue (dbFileName));
    defaultLm->SetAttribute ("Verbose", BooleanValue (verbose));
    defaultLm->SetAttribute ("NearRtRic", PointerValue (nearRtRic));
  
    cmm->SetAttribute ("NearRtRic", PointerValue (nearRtRic));
  
    nearRtRicE2Terminator->SetAttribute ("NearRtRic", PointerValue (nearRtRic));
    nearRtRicE2Terminator->SetAttribute ("DataRepository", PointerValue (dataRepository));
    nearRtRicE2Terminator->SetAttribute ("TransmissionDelayRv", StringValue ("ns3::ConstantRandomVariable[Constant=" + std::to_string (txDelay) + "]"));
  
    nearRtRic->SetAttribute ("DefaultLogicModule", PointerValue (defaultLm));
    nearRtRic->SetAttribute ("E2Terminator", PointerValue (nearRtRicE2Terminator));
    nearRtRic->SetAttribute ("DataRepository", PointerValue (dataRepository));
    nearRtRic->SetAttribute ("LmQueryInterval", TimeValue (Seconds (lmQueryInterval)));
    nearRtRic->SetAttribute ("ConflictMitigationModule", PointerValue (cmm));
  
    Simulator::Schedule (Seconds (1), &OranNearRtRic::Start, nearRtRic);
  
    for (uint32_t idx = 0; idx < ueNodes.GetN (); idx++) 
      {
        Ptr<OranReporterLocation> locationReporter = CreateObject<OranReporterLocation> ();
        Ptr<OranReporterLteUeCellInfo> lteUeCellInfoReporter = CreateObject<OranReporterLteUeCellInfo> ();
        Ptr<OranReporterAppLoss> appLossReporter = CreateObject<OranReporterAppLoss> ();
        Ptr<OranE2NodeTerminatorLteUe> lteUeTerminator = CreateObject<OranE2NodeTerminatorLteUe> ();
  
        locationReporter->SetAttribute ("Terminator", PointerValue (lteUeTerminator));
  
        lteUeCellInfoReporter->SetAttribute ("Terminator", PointerValue (lteUeTerminator));
  
        appLossReporter->SetAttribute ("Terminator", PointerValue (lteUeTerminator));
        clientApps.Get (idx)->TraceConnectWithoutContext ("Tx", MakeCallback (&ns3::OranReporterAppLoss::AddTx, appLossReporter));
        serverApps.Get (idx)->TraceConnectWithoutContext ("Rx", MakeCallback (&ns3::OranReporterAppLoss::AddRx, appLossReporter));
  
        lteUeTerminator->SetAttribute ("NearRtRic", PointerValue (nearRtRic));
        lteUeTerminator->SetAttribute ("RegistrationIntervalRv", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
        lteUeTerminator->SetAttribute ("SendIntervalRv", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  
        lteUeTerminator->AddReporter (locationReporter);
        lteUeTerminator->AddReporter (lteUeCellInfoReporter);
        lteUeTerminator->AddReporter (appLossReporter);
        lteUeTerminator->SetAttribute ("TransmissionDelayRv", StringValue ("ns3::ConstantRandomVariable[Constant=" + std::to_string (txDelay) + "]"));
  
        lteUeTerminator->Attach (ueNodes.Get (idx));
  
        //Simulator::Schedule (Seconds (1.999), &OranE2NodeTerminatorLteUe::Activate, lteUeTerminator);
        Simulator::Schedule (Seconds (1), &OranE2NodeTerminatorLteUe::Activate, lteUeTerminator);
      }
  
    for (uint32_t idx = 0; idx < enbNodes.GetN (); idx++)
      {
        Ptr<OranReporterLocation> locationReporter = CreateObject<OranReporterLocation> ();
        Ptr<OranE2NodeTerminatorLteEnb> lteEnbTerminator = CreateObject<OranE2NodeTerminatorLteEnb> ();
  
        locationReporter->SetAttribute ("Terminator", PointerValue (lteEnbTerminator));
  
        lteEnbTerminator->SetAttribute ("NearRtRic", PointerValue (nearRtRic));
        lteEnbTerminator->SetAttribute ("RegistrationIntervalRv", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
        lteEnbTerminator->SetAttribute ("SendIntervalRv", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  
        lteEnbTerminator->AddReporter (locationReporter);
        lteEnbTerminator->Attach (enbNodes.Get (idx));
        lteEnbTerminator->SetAttribute ("TransmissionDelayRv", StringValue ("ns3::ConstantRandomVariable[Constant=" + std::to_string (txDelay) + "]"));
        //Simulator::Schedule (Seconds (1.999), &OranE2NodeTerminatorLteEnb::Activate, lteEnbTerminator);
        Simulator::Schedule (Seconds (1), &OranE2NodeTerminatorLteEnb::Activate, lteEnbTerminator);
      }
  }
  // ORAN END


  // If we want detailed MAC and PHY traces, uncomment the next line
  // (slows down simulation significantly)
  //lteHelper->EnableTraces ();

  // Simulation ends at 1020 seconds
  //Simulator::Stop (Seconds(1020));
  Simulator::Stop (simTime + Seconds (20));

  // Erase the trace files if they exist

  //std::ofstream posOutFile ("positions_trace.txt", std::ios_base::trunc);
  //posOutFile.close ();
  std::ofstream trafficOutFile ("traffic_trace.txt", std::ios_base::trunc);
  trafficOutFile.close ();
  //std::ofstream rxOutFile ("rx_trace.txt", std::ios_base::trunc);
  //rxOutFile.close ();
  //std::ofstream txOutFile ("tx_trace.txt", std::ios_base::trunc);
  //txOutFile.close ();
  //std::ofstream dlrbsOutFile ("dlrbs_trace.txt", std::ios_base::trunc);
  //dlrbsOutFile.close ();
  //std::ofstream ulrbsOutFile ("ulrbs_trace.txt", std::ios_base::trunc);
  //ulrbsOutFile.close ();
  //std::ofstream dlBufferOutFile ("dlbuffer_trace.txt", std::ios_base::trunc);
  //dlBufferOutFile.close ();
  //std::ofstream ulBufferOutFile ("ulbuffer_trace.txt", std::ios_base::trunc);
  //ulBufferOutFile.close ();
  std::ofstream hoOutFile ("handover_trace.txt", std::ios_base::trunc);
  hoOutFile.close ();

  //Simulator::Schedule (Seconds (1), &TracePositions, ueNodes);

  txpkts1 = 0;
  rxpkts1 = 0;

  txpkts2 = 0;
  rxpkts2 = 0;

  txpkts3 = 0;
  rxpkts3 = 0;

  txpkts4 = 0;
  rxpkts4 = 0;

  oldLossPkts21 = 0;
  oldLossPkts22 = 0;
  oldLossPkts23 = 0;
  oldLossPkts24 = 0;
  oldLossBytes21 = 0;
  oldLossBytes22 = 0;
  oldLossBytes23 = 0;
  oldLossBytes24 = 0;
  oldLossPkts31 = 0;
  oldLossPkts32 = 0;
  oldLossPkts33 = 0;
  oldLossPkts34 = 0;
  oldLossBytes31 = 0;
  oldLossBytes32 = 0;
  oldLossBytes33 = 0;
  oldLossBytes34 = 0;

  oldRbsUe21 = 0;
  oldRbsUe22 = 0;
  oldRbsUe23 = 0;
  oldRbsUe24 = 0;
  oldRbsUe25 = 0;
  oldRbsUe31 = 0;
  oldRbsUe32 = 0;
  oldRbsUe33 = 0;
  oldRbsUe34 = 0;
  oldRbsUe35 = 0;
  oldRbsCell11 = 0;
  oldRbsCell12 = 0;
  oldRbsCell13 = 0;
  oldRbsCell14 = 0;
  oldRbsCell15 = 0;
  oldRbsCell21 = 0;
  oldRbsCell22 = 0;
  oldRbsCell23 = 0;
  oldRbsCell24 = 0;
  oldRbsCell25 = 0;

  if (printLoss)
    {
      Simulator::Schedule (Seconds (11), &CheckLoss, lteHelper, ueLteDevs, enbLteDevs, ueNodes, enbNodes);
    }

  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverEndOk", MakeCallback (&NotifyHandoverEndOkEnb));

  // Run the simulation
  Simulator::Run ();

  Simulator::Destroy ();

  if (verbose == true)
    {
      for (uint16_t i = 0; i < ueNodes.GetN () ; i++) 
        {
          //std::cout << "IMSI" << i;
          std::cout << (i + 1);
          std::cout << "\tTX_PKTS\t" << total_tx_pkts[i] << "\tTX_BYTES\t" << total_tx_bytes[i];
          std::cout << "\tRX_PKTS\t" << total_rx_pkts[i] << "\tRX_BYTES\t" << total_rx_bytes[i];
          std::cout << "\tLOSS_PKTS\t" << (1 - total_rx_pkts[i] * 1.0 / total_tx_pkts[i]);
          std::cout << "\tLOSS_BYTES\t" << (1 - total_rx_bytes[i] * 1.0 / total_tx_bytes[i]);
          std::cout << std::endl;
        }
    }

  return 0;
}

#else
int
main (int argc, char *argv[]) 
{
  return 0;
}

#endif

