#ifndef ORAN_LM_LTE_2_LTE_ONNX_HANDOVER
#define ORAN_LM_LTE_2_LTE_ONNX_HANDOVER

#include <vector>

#include <ns3/vector.h>

#include "oran-lm.h"
#include "oran-data-repository.h"
#include <onnxruntime_cxx_api.h>

namespace ns3 {

class OranLmLte2LteOnnxHandover : public OranLm
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
  OranLmLte2LteOnnxHandover (void);
  ~OranLmLte2LteOnnxHandover (void);
  virtual std::vector<Ptr<OranCommand> > Run (void) override;
  void SetOnnxModelPath (std::string onnxModelPath);

private:
  Ort::Env m_env;
  Ort::Session m_session {nullptr};
  Ort::MemoryInfo m_memoryInfo {Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU)};
  Ort::AllocatorWithDefaultOptions m_allocator;

  std::vector<OranLmLte2LteOnnxHandover::UeInfo> GetUeInfos (Ptr<OranDataRepository> data) const;
  std::vector<OranLmLte2LteOnnxHandover::EnbInfo> GetEnbInfos (Ptr<OranDataRepository> data) const;
  std::vector<Ptr<OranCommand> > GetHandoverCommands (
      Ptr<OranDataRepository> data,
      std::vector<OranLmLte2LteOnnxHandover::UeInfo> ueInfos,
      std::vector<OranLmLte2LteOnnxHandover::EnbInfo> enbInfos);

}; // class OranLmLte2LteOnnxHandover

} // namespace ns3

#endif // ORAN_LM_LTE_2_LTE_ONNX_HANDOVER
