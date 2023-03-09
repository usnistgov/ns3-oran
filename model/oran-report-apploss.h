#ifndef ORAN_REPORT_APPLOSS
#define ORAN_REPORT_APPLOSS

#include <string>

#include "oran-report.h"

namespace ns3 {

class OranReportAppLoss : public OranReport
{
public:
  static TypeId GetTypeId (void);
  OranReportAppLoss (void);
  ~OranReportAppLoss (void) override;
  std::string ToString (void) const override;

  double GetLoss (void) const;

private:
  double m_loss;
}; // class OranReportAppLoss

} // namespace ns3

#endif  //ORAN_REPORT_APPLOSS
