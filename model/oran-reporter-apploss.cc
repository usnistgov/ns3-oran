#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include <ns3/double.h>
#include <ns3/uinteger.h>

#include <ns3/packet.h>
#include <ns3/address.h>

#include "oran-reporter-apploss.h"
#include "oran-report-apploss.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("OranReporterAppLoss");
NS_OBJECT_ENSURE_REGISTERED (OranReporterAppLoss);

TypeId
OranReporterAppLoss::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::OranReporterAppLoss")
    .SetParent<OranReporter> ()
    .AddConstructor<OranReporterAppLoss> ()
  ;

  return tid;
}

OranReporterAppLoss::OranReporterAppLoss (void)
{
  NS_LOG_FUNCTION (this);
  
  m_txcount = 0;
  m_rxcount = 0;
}

OranReporterAppLoss::~OranReporterAppLoss (void)
{
  NS_LOG_FUNCTION (this);
}

void
OranReporterAppLoss::AddTx (Ptr<const Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  m_txcount ++;
}

void
OranReporterAppLoss::AddRx (Ptr<const Packet> p, const Address &from )
{
  NS_LOG_FUNCTION (this << p << from);

  m_rxcount ++; 
}

std::vector <Ptr<OranReport> >
OranReporterAppLoss::GenerateReports (void)
{
  NS_LOG_FUNCTION (this);

  std::vector<Ptr<OranReport> > reports;

  if (m_active)
    {
      NS_ABORT_MSG_IF (m_terminator == nullptr, "Attempting to generate reports in reporter with NULL E2 Terminator");
     
      double loss = 0;
      if (m_rxcount <= m_txcount && m_txcount > 0) 
        {
          //loss = 1 - (m_rxcount * 1.0 / m_txcount);
          loss = (m_txcount - m_rxcount) * 1.0 / m_txcount;
        }

      Ptr<OranReportAppLoss> lossReport = CreateObject<OranReportAppLoss> ();
      lossReport->SetAttribute ("ReporterE2NodeId", UintegerValue (m_terminator->GetE2NodeId ()));
      lossReport->SetAttribute ("Time", TimeValue (Simulator::Now ()));
      lossReport->SetAttribute ("Loss", DoubleValue (loss));

      reports.push_back (lossReport);
      m_txcount = 0;
      m_rxcount = 0;
    }

  return reports;
}

} // namespace ns3
