/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * NIST-developed software is provided by NIST as a public service. You may
 * use, copy and distribute copies of the software in any medium, provided that
 * you keep intact this entire notice. You may improve, modify and create
 * derivative works of the software or any portion of the software, and you may
 * copy and distribute such modifications or works. Modified works should carry
 * a notice stating that you changed the software and should note the date and
 * nature of any such change. Please explicitly acknowledge the National
 * Institute of Standards and Technology as the source of the software.
 *
 * NIST-developed software is expressly provided "AS IS." NIST MAKES NO
 * WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF
 * LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST
 * NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE
 * UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST
 * DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE
 * SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE
 * CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
 *
 * You are solely responsible for determining the appropriateness of using and
 * distributing the software and you assume all risks associated with its use,
 * including but not limited to the risks and costs of program errors,
 * compliance with applicable laws, damage to or loss of data, programs or
 * equipment, and the unavailability or interruption of operation. This
 * software is not intended to be used in any situation where a failure could
 * cause risk of injury or damage to property. The software developed by NIST
 * employees is not subject to copyright protection within the United States.
 */

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