#ifndef ORAN_REPORTER_APPLOSS
#define ORAN_REPORTER_APPLOSS

#include <vector>

#include <ns3/ptr.h>

#include "oran-reporter.h"
#include "oran-report.h"

namespace ns3 {

class Packet;
class Address;

class OranReporterAppLoss : public OranReporter
{
public:
  static TypeId GetTypeId (void);
  OranReporterAppLoss (void);
  ~OranReporterAppLoss (void) override;

  void AddTx (Ptr<const Packet> p);
  void AddRx (Ptr<const Packet> p, const Address &from);

protected:
  std::vector<Ptr<OranReport> > GenerateReports (void) override;

private:
  uint64_t m_txcount;
  uint64_t m_rxcount;
};

} // namespace ns3

#endif // ORAN_REPORTER_APPLOSS
