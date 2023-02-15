#ifndef ORAN_LM_LTE_2_LTE_ML_HANDOVER
#define ORAN_LM_LTE_2_LTE_ML_HANDOVER

#include <vector>

#include <ns3/vector.h>

#include "oran-lm.h"
#include "oran-data-repository.h"

#include <torch/script.h>

namespace ns3 {

class OranLmLte2LteMlHandover : public OranLm
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
  OranLmLte2LteMlHandover (void);
  ~OranLmLte2LteMlHandover (void);
  virtual std::vector<Ptr<OranCommand> > Run (void) override;

private:
  torch::jit::script::Module m_model;

  std::vector<OranLmLte2LteMlHandover::UeInfo> GetUeInfos (Ptr<OranDataRepository> data) const;
  std::vector<OranLmLte2LteMlHandover::EnbInfo> GetEnbInfos (Ptr<OranDataRepository> data) const;
  std::vector<Ptr<OranCommand> > GetHandoverCommands (
      Ptr<OranDataRepository> data,
      std::vector<OranLmLte2LteMlHandover::UeInfo> ueInfos,
      std::vector<OranLmLte2LteMlHandover::EnbInfo> enbInfos);

}; // class OranLmLte2LteMlHandover

} // namespace ns3

#endif // ORAN_LM_LTE_2_LTE_ML_HANDOVER
