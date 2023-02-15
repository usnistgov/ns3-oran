#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include <ns3/uinteger.h>

#include "oran-lm-lte-2-lte-ml-handover.h"
#include "oran-command-lte-2-lte-handover.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("OranLmLte2LteMlHandover");
NS_OBJECT_ENSURE_REGISTERED (OranLmLte2LteMlHandover);

TypeId
OranLmLte2LteMlHandover::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::OranLmLte2LteMlHandover")
    .SetParent<OranLm> ()
    .AddConstructor<OranLmLte2LteMlHandover> ()
  ;

  return tid;
}

OranLmLte2LteMlHandover::OranLmLte2LteMlHandover (void)
{
  NS_LOG_FUNCTION (this);

  m_name = "OranLmLte2LteMlHandover";
}

OranLmLte2LteMlHandover::~OranLmLte2LteMlHandover (void)
{
  NS_LOG_FUNCTION (this);
}

std::vector<Ptr<OranCommand> >
OranLmLte2LteMlHandover::Run (void)
{
  NS_LOG_FUNCTION (this);

  std::vector<Ptr<OranCommand> > commands;

  if (m_active)
    {
      NS_ABORT_MSG_IF (m_nearRtRic == nullptr, "Attempting to run LM (" + m_name + ") with NULL Near-RT RIC");

      Ptr<OranDataRepository> data = m_nearRtRic->Data ();
      std::vector<UeInfo> ueInfos = GetUeInfos (data);
      std::vector<EnbInfo> enbInfos = GetEnbInfos (data);
      commands = GetHandoverCommands (data, ueInfos, enbInfos);
    }

  return commands;
}

std::vector<OranLmLte2LteMlHandover::UeInfo>
OranLmLte2LteMlHandover::GetUeInfos (Ptr<OranDataRepository> data) const
{
  NS_LOG_FUNCTION (this << data);

  std::vector<UeInfo> ueInfos;
  for (auto ueId : data->GetLteUeE2NodeIds ())
    {
      UeInfo ueInfo;
      ueInfo.nodeId = ueId;
      bool found;
      std::tie (found, ueInfo.cellId, ueInfo.rnti) = data->GetLteUeCellInfo (ueInfo.nodeId);
      if (found)
        {
          std::map<Time, Vector> nodePositions = data->GetNodePositions (ueInfo.nodeId, Seconds (0), Simulator::Now ());
          if (!nodePositions.empty ())
            {
              ueInfo.position = nodePositions.rbegin ()->second;
              ueInfo.loss = data->GetAppLoss (ueInfo.nodeId);
              ueInfos.push_back (ueInfo);
            }
          else
            {
              NS_LOG_INFO ("Could not find LTE UE location for E2 Node ID = " << ueInfo.nodeId);
            }
        }
      else
        {
          NS_LOG_INFO ("Could not find LTE UE cell info for E2 Node ID = " << ueInfo.nodeId);
        }
    }
  return ueInfos;
}

std::vector<OranLmLte2LteMlHandover::EnbInfo>
OranLmLte2LteMlHandover::GetEnbInfos (Ptr<OranDataRepository> data) const
{
  NS_LOG_FUNCTION (this << data);

  std::vector<EnbInfo> enbInfos;
  for (auto enbId : data->GetLteEnbE2NodeIds())
    {
      EnbInfo enbInfo;
      enbInfo.nodeId = enbId;
      bool found;
      std::tie (found, enbInfo.cellId) = data->GetLteEnbCellInfo (enbInfo.nodeId);
      if (found)
        {
          std::map<Time, Vector> nodePositions = data->GetNodePositions (enbInfo.nodeId, Seconds (0), Simulator::Now ());

          if (!nodePositions.empty ())
            {
              enbInfo.position = nodePositions.rbegin ()->second;
              enbInfos.push_back (enbInfo);
            }
          else
            {
              NS_LOG_INFO ("Could not find LTE eNB location for E2 Node ID = " << enbInfo.nodeId);
            }
        }
      else
        {
          NS_LOG_INFO ("Could not find LTE eNB cell info for E2 Node ID = " << enbInfo.nodeId);
        }
    }
  return enbInfos;
}

std::vector<Ptr<OranCommand> >
OranLmLte2LteMlHandover::GetHandoverCommands (
    Ptr<OranDataRepository> data,
    std::vector<OranLmLte2LteMlHandover::UeInfo> ueInfos,
    std::vector<OranLmLte2LteMlHandover::EnbInfo> enbInfos) 
{
  NS_LOG_FUNCTION (this << data);

  std::vector<Ptr<OranCommand> > commands;

  std::map<uint16_t, float> distanceEnb1;
  std::map<uint16_t, float> distanceEnb2;
  std::map<uint16_t, float> loss;

  for (auto ueInfo : ueInfos)
    {
      for (auto enbInfo : enbInfos) 
        {
          float d = std::sqrt (
              std::pow (ueInfo.position.x - enbInfo.position.x, 2) +
              std::pow (ueInfo.position.y - enbInfo.position.y, 2)
              //+ std::pow (ueInfo.position.z - enbInfo.position.z, 2)
              );
          if (enbInfo.cellId == 1)
            {
              distanceEnb1 [ueInfo.nodeId] = d;
            }
          else
            {
              distanceEnb2 [ueInfo.nodeId] = d;
            }
        }
      loss[ueInfo.nodeId] = ueInfo.loss;
    }

  std::vector<float> inputv = {
    distanceEnb1[1], distanceEnb2[1], loss[1], 
    distanceEnb1[2], distanceEnb2[2], loss[2], 
    distanceEnb1[3], distanceEnb2[3], loss[3], 
    distanceEnb1[4], distanceEnb2[4], loss[4]
  };
  LogLogicToRepository ("ML input tensor: (" + 
      std::to_string (inputv.at(0)) + ", " +
      std::to_string (inputv.at(1)) + ", " + 
      std::to_string (inputv.at(2)) + ", " +
      std::to_string (inputv.at(3)) + ", " +
      std::to_string (inputv.at(4)) + ", " + 
      std::to_string (inputv.at(5)) + ", " +
      std::to_string (inputv.at(6)) + ", " +
      std::to_string (inputv.at(7)) + ", " + 
      std::to_string (inputv.at(8)) + ", " +
      std::to_string (inputv.at(9)) + ", " +
      std::to_string (inputv.at(10)) + ", " + 
      std::to_string (inputv.at(11)) + ", " +
      ")"
      );

  const auto inputShape = session.GetInputTypeInfo(0UL).GetTensorTypeAndShapeInfo().GetShape();
  const auto inputTensor = Ort::Value::CreateTensor<float>(
      memoryInfo,
      inputv.data(), inputv.size(),
      inputShape.data(), inputShape.size()
  );

  const auto inputName = session.GetInputNameAllocated(0UL, allocator);
  std::array<const char *, 1> inputNames{inputName.get()};

  const auto outputName = session.GetOutputNameAllocated(0UL, allocator);
  std::array<const char *, 1> outputNames{outputName.get()};
  const auto output = session.Run(
      Ort::RunOptions{},
      inputNames.data(), &inputTensor, 1UL,
      outputNames.data(), 1
  );

  // We get 4 floats back from the network
  // each with the fitting amount for each
  // possible class.
  // We select the class from the index
  // with the highest 'fitting' value
  const auto outputData = output[0].GetTensorData<float>();
  const auto count = output[0].GetTensorTypeAndShapeInfo().GetElementCount();
  auto maxValue = *outputData;
  auto maxIndex = 0UL;

  // ONNX gives us a C Style array back
  // TODO EPB: Maybe provide an output tensor with std::array
  for (auto i = 0UL; i < count; i++) {
      if (*(outputData+i) > maxValue) {
            maxValue = *(outputData+i);
            maxIndex = i;
      }
  }

  int configuration = static_cast<int>(maxIndex);
  LogLogicToRepository ("ML Chooses configuration " + std::to_string (configuration));

  //std::cout << Simulator::Now ().GetSeconds () << " CONFIG " << configuration << std::endl;

  for (auto ueInfo : ueInfos)
    {
      if (ueInfo.nodeId == 2)
        {
          if (ueInfo.cellId == 1 && (configuration == 2 || configuration == 3))
            {
              // Handover to cellId 2
              Ptr<OranCommandLte2LteHandover> handoverCommand = CreateObject<OranCommandLte2LteHandover> ();
              handoverCommand->SetAttribute ("TargetE2NodeId", UintegerValue (5));
              handoverCommand->SetAttribute ("TargetRnti", UintegerValue (ueInfo.rnti));
              handoverCommand->SetAttribute ("TargetCellId", UintegerValue (2));
              data->LogCommandLm (m_name, handoverCommand);
              commands.push_back (handoverCommand);

              LogLogicToRepository ("Moving UE 2 to Cell ID 2");
            }
          else
            {
              if (ueInfo.cellId == 2 && (configuration == 0 || configuration == 1))
                {
                  // Handover to cellId 1
                  Ptr<OranCommandLte2LteHandover> handoverCommand = CreateObject<OranCommandLte2LteHandover> ();
                  handoverCommand->SetAttribute ("TargetE2NodeId", UintegerValue (6));
                  handoverCommand->SetAttribute ("TargetRnti", UintegerValue (ueInfo.rnti));
                  handoverCommand->SetAttribute ("TargetCellId", UintegerValue (1));
                  data->LogCommandLm (m_name, handoverCommand);
                  commands.push_back (handoverCommand);

                  LogLogicToRepository ("Moving UE 2 to Cell ID 1");
                }
            }
        }
      else
        {
          if (ueInfo.nodeId == 3)
            {
              if (ueInfo.cellId == 1 && (configuration == 1 || configuration == 3))
                {
                  // Handover to cellId 2
                  Ptr<OranCommandLte2LteHandover> handoverCommand = CreateObject<OranCommandLte2LteHandover> ();
                  handoverCommand->SetAttribute ("TargetE2NodeId", UintegerValue (5));
                  handoverCommand->SetAttribute ("TargetRnti", UintegerValue (ueInfo.rnti));
                  handoverCommand->SetAttribute ("TargetCellId", UintegerValue (2));
                  data->LogCommandLm (m_name, handoverCommand);
                  commands.push_back (handoverCommand);

                  LogLogicToRepository ("Moving UE 3 to Cell ID 2");
                }
              else
                {
                  if (ueInfo.cellId == 2 && (configuration == 0 || configuration == 2))
                    {
                      // Handover to cellId 1
                      Ptr<OranCommandLte2LteHandover> handoverCommand = CreateObject<OranCommandLte2LteHandover> ();
                      handoverCommand->SetAttribute ("TargetE2NodeId", UintegerValue (6));
                      handoverCommand->SetAttribute ("TargetRnti", UintegerValue (ueInfo.rnti));
                      handoverCommand->SetAttribute ("TargetCellId", UintegerValue (1));
                      data->LogCommandLm (m_name, handoverCommand);
                      commands.push_back (handoverCommand);

                      LogLogicToRepository ("Moving UE 3 to Cell ID 1");
                    }
                }
            }
        }
    }
 
  return commands;
}

} // namespace ns3

