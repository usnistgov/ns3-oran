#ifndef ORAN_LM_LTE_2_LTE_TORCH_HANDOVER
#define ORAN_LM_LTE_2_LTE_TORCH_HANDOVER

#include <vector>
#include <torch/script.h>

#include <ns3/vector.h>

#include "oran-lm.h"
#include "oran-data-repository.h"

namespace ns3 {

class OranLmLte2LteTorchHandover : public OranLm
{
protected:
  struct UeInfo
    {
      uint64_t nodeId;
      uint16_t cellId;
      uint16_t rnti;
      Vector position;
      double loss;
    };

  struct EnbInfo
    {
      uint64_t nodeId;
      uint16_t cellId;
      Vector position;
    };

public:
  static TypeId GetTypeId (void);
  OranLmLte2LteTorchHandover (void);
  ~OranLmLte2LteTorchHandover (void);
  virtual std::vector<Ptr<OranCommand> > Run (void) override;
  void SetTorchModelPath (std::string torchModelPath);

private:
  torch::jit::script::Module m_model;

  std::vector<OranLmLte2LteTorchHandover::UeInfo> GetUeInfos (Ptr<OranDataRepository> data) const;
  std::vector<OranLmLte2LteTorchHandover::EnbInfo> GetEnbInfos (Ptr<OranDataRepository> data) const;
  std::vector<Ptr<OranCommand> > GetHandoverCommands (
      Ptr<OranDataRepository> data,
      std::vector<OranLmLte2LteTorchHandover::UeInfo> ueInfos,
      std::vector<OranLmLte2LteTorchHandover::EnbInfo> enbInfos);

}; // class OranLmLte2LteTorchHandover

} // namespace ns3

#endif // ORAN_LM_LTE_2_LTE_TORCH_HANDOVER
