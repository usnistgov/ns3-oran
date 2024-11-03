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
 * compliance with applicable laws, damage to or rsrp of data, programs or
 * equipment, and the unavailability or interruption of operation. This
 * software is not intended to be used in any situation where a failure could
 * cause risk of injury or damage to property. The software developed by NIST
 * employees is not subject to copyright protection within the United States.
 */

#include "oran-report-lte-ue-rsrp-rsrq.h"

#include "oran-report.h"

#include <ns3/abort.h>
#include <ns3/boolean.h>
#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/uinteger.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranReportLteUeRsrpRsrq");
NS_OBJECT_ENSURE_REGISTERED(OranReportLteUeRsrpRsrq);

TypeId
OranReportLteUeRsrpRsrq::GetTypeId()
{
    static TypeId tid = TypeId("ns3::OranReportLteUeRsrpRsrq")
                            .SetParent<OranReport>()
                            .AddConstructor<OranReportLteUeRsrpRsrq>()
                            .AddAttribute("Rnti",
                                          "The RNTI.",
                                          UintegerValue(),
                                          MakeUintegerAccessor(&OranReportLteUeRsrpRsrq::m_rnti),
                                          MakeUintegerChecker<uint16_t>())
                            .AddAttribute("CellId",
                                          "The cell ID.",
                                          UintegerValue(),
                                          MakeUintegerAccessor(&OranReportLteUeRsrpRsrq::m_cellId),
                                          MakeUintegerChecker<uint16_t>())
                            .AddAttribute("Rsrp",
                                          "The RSRP.",
                                          DoubleValue(),
                                          MakeDoubleAccessor(&OranReportLteUeRsrpRsrq::m_rsrp),
                                          MakeDoubleChecker<double>())
                            .AddAttribute("Rsrq",
                                          "The RSRQ.",
                                          DoubleValue(),
                                          MakeDoubleAccessor(&OranReportLteUeRsrpRsrq::m_rsrq),
                                          MakeDoubleChecker<double>())
                            .AddAttribute("IsServingCell",
                                          "The flag that indicates if this for the serving cell.",
                                          BooleanValue(),
                                          MakeBooleanAccessor(&OranReportLteUeRsrpRsrq::m_isServingCell),
                                          MakeBooleanChecker())
                            .AddAttribute("ComponentCarrierId",
                                          "The component carrier ID.",
                                          UintegerValue(),
                                          MakeUintegerAccessor(&OranReportLteUeRsrpRsrq::m_componentCarrierId),
                                          MakeUintegerChecker<uint16_t>());

    return tid;
}

OranReportLteUeRsrpRsrq::OranReportLteUeRsrpRsrq()
{
    NS_LOG_FUNCTION(this);
}

OranReportLteUeRsrpRsrq::~OranReportLteUeRsrpRsrq()
{
    NS_LOG_FUNCTION(this);
}

std::string
OranReportLteUeRsrpRsrq::ToString() const
{
    NS_LOG_FUNCTION(this);

    std::stringstream ss;
    Time time = GetTime();

    ss << "OranReportLteUeRsrpRsrq("
       << "E2NodeId=" << GetReporterE2NodeId()
       << ";Time=" << time.As(Time::S)
       << ";RNTI=" << +m_rnti
       << ";Cell ID=" << +m_cellId
       << ";RSRP=" << m_rsrp
       << ";RSRQ=" << m_rsrq
       << ";Is Serving Cell=" << m_isServingCell
       << ";Component Carrier ID=" << +m_componentCarrierId
       << ")";

    return ss.str();
}

uint16_t
OranReportLteUeRsrpRsrq::GetRnti() const
{
  NS_LOG_FUNCTION(this);

  return m_rnti;
}

uint16_t
OranReportLteUeRsrpRsrq::GetCellId() const
{
  NS_LOG_FUNCTION(this);

  return m_cellId;
}

double
OranReportLteUeRsrpRsrq::GetRsrp() const
{
    NS_LOG_FUNCTION(this);

    return m_rsrp;
}

double
OranReportLteUeRsrpRsrq::GetRsrq() const
{
    NS_LOG_FUNCTION(this);

    return m_rsrp;
}

bool
OranReportLteUeRsrpRsrq::GetIsServingCell() const
{
  NS_LOG_FUNCTION(this);

  return m_isServingCell;
}

uint16_t
OranReportLteUeRsrpRsrq::GetComponentCarrierId() const
{
  NS_LOG_FUNCTION(this);

  return m_componentCarrierId;
}

} // namespace ns3
