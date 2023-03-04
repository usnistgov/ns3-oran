#include <ns3/log.h>
#include <ns3/abort.h>
#include <ns3/double.h>

#include "oran-report.h"
#include "oran-report-apploss.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("OranReportAppLoss");
NS_OBJECT_ENSURE_REGISTERED (OranReportAppLoss);

TypeId
OranReportAppLoss::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::OranReportAppLoss")
    .SetParent<OranReport> ()
    .AddConstructor<OranReportAppLoss> ()
    .AddAttribute ("Loss", "App Loss Rate",
                  DoubleValue (),
                  MakeDoubleAccessor (&OranReportAppLoss::m_loss),
                  MakeDoubleChecker<double> ())
    ;

  return tid;
}

OranReportAppLoss::OranReportAppLoss (void)
{
  NS_LOG_FUNCTION (this);
}

OranReportAppLoss::~OranReportAppLoss (void)
{
  NS_LOG_FUNCTION (this);
}

std::string 
OranReportAppLoss::ToString (void) const
{
  NS_LOG_FUNCTION (this);

  std::stringstream ss;
  Time time = GetTime ();

  ss << "OranReportAppLoss("
     << "E2NodeId=" << GetReporterE2NodeId ()
     << ";Time=" << time.As (Time::S)
     << ";Loss=" << m_loss
     << ")";

  return ss.str ();
}

double 
OranReportAppLoss::GetLoss (void) const
{
  NS_LOG_FUNCTION (this);

  return m_loss;
}

}
