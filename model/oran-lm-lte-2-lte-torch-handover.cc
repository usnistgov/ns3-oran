#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include <ns3/string.h>
#include <ns3/uinteger.h>

#include "oran-lm-lte-2-lte-torch-handover.h"
#include "oran-command-lte-2-lte-handover.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("OranLmLte2LteTorchHandover");
NS_OBJECT_ENSURE_REGISTERED (OranLmLte2LteTorchHandover);

TypeId
OranLmLte2LteTorchHandover::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::OranLmLte2LteTorchHandover")
    .SetParent<OranLm> ()
    .AddConstructor<OranLmLte2LteTorchHandover> ()
    .AddAttribute ("TorchModelPath",
                   "The file path of the ML model.",
                   StringValue ("saved_trained_classification_pytorch.pt"),
                   MakeStringAccessor (&OranLmLte2LteTorchHandover::SetTorchModelPath),
                   MakeStringChecker ())
  ;

  return tid;
}

OranLmLte2LteTorchHandover::OranLmLte2LteTorchHandover (void)
{
  NS_LOG_FUNCTION (this);

  m_name = "OranLmLte2LteTorchHandover";
}

OranLmLte2LteTorchHandover::~OranLmLte2LteTorchHandover (void)
{
  NS_LOG_FUNCTION (this);
}

std::vector<Ptr<OranCommand> >
OranLmLte2LteTorchHandover::Run (void)
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

void
OranLmLte2LteTorchHandover::SetTorchModelPath (std::string torchModelPath)
{
  try
    {
      m_model = torch::jit::load (torchModelPath);
    }
  catch (const c10::Error& e)
    {
      NS_ABORT_MSG ("Could not load trained ML model.");
    }
}

std::vector<OranLmLte2LteTorchHandover::UeInfo>
OranLmLte2LteTorchHandover::GetUeInfos (Ptr<OranDataRepository> data) const
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

std::vector<OranLmLte2LteTorchHandover::EnbInfo>
OranLmLte2LteTorchHandover::GetEnbInfos (Ptr<OranDataRepository> data) const
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
OranLmLte2LteTorchHandover::GetHandoverCommands (
    Ptr<OranDataRepository> data,
    std::vector<OranLmLte2LteTorchHandover::UeInfo> ueInfos,
    std::vector<OranLmLte2LteTorchHandover::EnbInfo> enbInfos) 
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

  std::vector<torch::jit::IValue> inputs;
  inputs.push_back (torch::from_blob (inputv.data (), {1, 12}).to (torch::kFloat32));
  at::Tensor output = torch::softmax (m_model.forward (inputs).toTensor (), 1);

  int configuration = output.argmax (1).item ().toInt ();
  LogLogicToRepository ("ML Chooses configuration " + std::to_string (configuration));

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

