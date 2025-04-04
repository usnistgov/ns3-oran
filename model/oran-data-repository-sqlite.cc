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

#include "oran-data-repository-sqlite.h"

#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/string.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranDataRepositorySqlite");

NS_OBJECT_ENSURE_REGISTERED(OranDataRepositorySqlite);

TypeId
OranDataRepositorySqlite::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::OranDataRepositorySqlite")
            .SetParent<OranDataRepository>()
            .AddConstructor<OranDataRepositorySqlite>()
            .AddAttribute("DatabaseFile",
                          "The database file path.",
                          StringValue("oran-repository.db"),
                          MakeStringAccessor(&OranDataRepositorySqlite::m_dbPath),
                          MakeStringChecker())
            .AddTraceSource("QueryRc",
                            "Return code for SQL queries",
                            MakeTraceSourceAccessor(&OranDataRepositorySqlite::m_queryRc),
                            "ns3::OranDataRepositorySqlite::QueryTracedCallback")

        ;

    return tid;
}

OranDataRepositorySqlite::OranDataRepositorySqlite()
    : OranDataRepository(),
      m_db(nullptr)
{
    NS_LOG_FUNCTION(this);

    InitStatements();
}

OranDataRepositorySqlite::~OranDataRepositorySqlite()
{
    NS_LOG_FUNCTION(this);
}

void
OranDataRepositorySqlite::Activate()
{
    NS_LOG_FUNCTION(this);

    OranDataRepository::Activate();

    if (!IsDbOpen())
    {
        OpenDb();
    }
}

void
OranDataRepositorySqlite::Deactivate()
{
    NS_LOG_FUNCTION(this);

    if (IsDbOpen())
    {
        CloseDb();
    }

    OranDataRepository::Deactivate();
}

bool
OranDataRepositorySqlite::IsNodeRegistered(uint64_t e2NodeId)
{
    NS_LOG_FUNCTION(this);

    bool registered = false;
    if (m_active)
    {
        int rc;
        sqlite3_stmt* stmt = nullptr;

        sqlite3_prepare_v2(m_db, m_queryStmtsStrings[CHECK_NODE_REGISTERED].c_str(), -1, &stmt, 0);
        sqlite3_bind_int64(stmt, 1, e2NodeId);

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            registered = sqlite3_column_int(stmt, 0);
        }

        CheckQueryReturnCode(stmt, rc, FormatBoundArgsList(e2NodeId));
        sqlite3_finalize(stmt);
    }
    return registered;
}

uint64_t
OranDataRepositorySqlite::RegisterNode(OranNearRtRic::NodeType type, uint64_t id)
{
    NS_LOG_FUNCTION(this);

    uint64_t e2NodeId = 0;

    if (m_active)
    {
        int rc;
        sqlite3_stmt* stmt = nullptr;

        if (id == 0)
        {
            // Insert or update the node information
            sqlite3_prepare_v2(m_db, m_queryStmtsStrings[INSERT_NODE_ADD].c_str(), -1, &stmt, 0);

            sqlite3_bind_int(stmt, 1, type);

            rc = sqlite3_step(stmt);

            CheckQueryReturnCode(stmt, rc, FormatBoundArgsList(type));

            e2NodeId = sqlite3_last_insert_rowid(m_db);
        }
        else
        {
            sqlite3_prepare_v2(m_db, m_queryStmtsStrings[INSERT_NODE_UPDATE].c_str(), -1, &stmt, 0);

            sqlite3_bind_int(stmt, 1, id);
            sqlite3_bind_int(stmt, 2, type);

            rc = sqlite3_step(stmt);

            CheckQueryReturnCode(stmt, rc, FormatBoundArgsList(type));

            e2NodeId = sqlite3_last_insert_rowid(m_db);
        }

        sqlite3_finalize(stmt);

        // Insert the registration information
        sqlite3_prepare_v2(m_db,
                           m_queryStmtsStrings[INSERT_NODE_REGISTRATION].c_str(),
                           -1,
                           &stmt,
                           0);

        sqlite3_bind_int64(stmt, 1, e2NodeId);
        sqlite3_bind_int(stmt, 2, 1);
        sqlite3_bind_int64(stmt, 3, Simulator::Now().GetTimeStep());

        rc = sqlite3_step(stmt);

        CheckQueryReturnCode(stmt,
                             rc,
                             FormatBoundArgsList(e2NodeId, true, Simulator::Now().GetTimeStep()));

        sqlite3_finalize(stmt);
    }

    return e2NodeId;
}

uint64_t
OranDataRepositorySqlite::RegisterNodeLteUe(uint64_t id, uint64_t imsi)
{
    NS_LOG_FUNCTION(this);
    uint64_t e2NodeId = 0;

    if (m_active)
    {
        int rc;
        sqlite3_stmt* stmt = nullptr;
        e2NodeId = RegisterNode(OranNearRtRic::NodeType::LTEUE, id);

        sqlite3_prepare_v2(m_db, m_queryStmtsStrings[INSERT_LTE_UE_NODE].c_str(), -1, &stmt, 0);

        sqlite3_bind_int64(stmt, 1, id);
        sqlite3_bind_int64(stmt, 2, imsi);

        rc = sqlite3_step(stmt);
        CheckQueryReturnCode(stmt, rc, FormatBoundArgsList(id, imsi));
        sqlite3_finalize(stmt);
    }
    return e2NodeId;
}

uint64_t
OranDataRepositorySqlite::RegisterNodeLteEnb(uint64_t id, uint16_t cellId)
{
    NS_LOG_FUNCTION(this << id << cellId);

    uint64_t e2NodeId = 0;

    if (m_active)
    {
        int rc;
        sqlite3_stmt* stmt = nullptr;
        e2NodeId = RegisterNode(OranNearRtRic::NodeType::LTEENB, id);

        sqlite3_prepare_v2(m_db, m_queryStmtsStrings[INSERT_LTE_ENB_NODE].c_str(), -1, &stmt, 0);

        sqlite3_bind_int64(stmt, 1, id);
        sqlite3_bind_int(stmt, 2, cellId);

        rc = sqlite3_step(stmt);
        CheckQueryReturnCode(stmt, rc, FormatBoundArgsList(id, cellId));
        sqlite3_finalize(stmt);
    }
    return e2NodeId;
}

uint64_t
OranDataRepositorySqlite::DeregisterNode(uint64_t e2NodeId)
{
    NS_LOG_FUNCTION(this << e2NodeId);

    uint64_t retVal = 0;
    if (m_active)
    {
        int rc;
        sqlite3_stmt* stmt = nullptr;

        retVal = e2NodeId;

        sqlite3_prepare_v2(m_db,
                           m_queryStmtsStrings[INSERT_NODE_REGISTRATION].c_str(),
                           -1,
                           &stmt,
                           0);

        sqlite3_bind_int64(stmt, 1, e2NodeId);
        sqlite3_bind_int(stmt, 2, false);
        sqlite3_bind_int64(stmt, 3, Simulator::Now().GetTimeStep());

        rc = sqlite3_step(stmt);
        CheckQueryReturnCode(stmt,
                             rc,
                             FormatBoundArgsList(e2NodeId, false, Simulator::Now().GetTimeStep()));
        sqlite3_finalize(stmt);
    }
    return retVal;
}

void
OranDataRepositorySqlite::SavePosition(uint64_t e2NodeId, Vector pos, Time t)
{
    NS_LOG_FUNCTION(this << e2NodeId << pos << t);

    if (m_active)
    {
        if (IsNodeRegistered(e2NodeId))
        {
            int rc;
            sqlite3_stmt* stmt = nullptr;

            sqlite3_prepare_v2(m_db,
                               m_queryStmtsStrings[INSERT_NODE_LOCATION].c_str(),
                               -1,
                               &stmt,
                               0);

            sqlite3_bind_int64(stmt, 1, e2NodeId);
            sqlite3_bind_double(stmt, 2, pos.x);
            sqlite3_bind_double(stmt, 3, pos.y);
            sqlite3_bind_double(stmt, 4, pos.z);
            sqlite3_bind_int64(stmt, 5, t.GetTimeStep());

            rc = sqlite3_step(stmt);
            CheckQueryReturnCode(
                stmt,
                rc,
                FormatBoundArgsList(e2NodeId, pos.x, pos.y, pos.z, t.GetTimeStep()));
            sqlite3_finalize(stmt);
        }
    }
}

void
OranDataRepositorySqlite::SaveLteUeCellInfo(uint64_t e2NodeId,
                                            uint16_t cellId,
                                            uint16_t rnti,
                                            Time t)
{
    NS_LOG_FUNCTION(this << e2NodeId << (uint32_t)cellId << (uint32_t)rnti << t);

    if (m_active)
    {
        if (IsNodeRegistered(e2NodeId))
        {
            int rc;
            sqlite3_stmt* stmt = nullptr;

            sqlite3_prepare_v2(m_db, m_queryStmtsStrings[INSERT_LTE_UE_CELL].c_str(), -1, &stmt, 0);

            sqlite3_bind_int64(stmt, 1, e2NodeId);
            sqlite3_bind_int(stmt, 2, cellId);
            sqlite3_bind_int(stmt, 3, rnti);
            sqlite3_bind_int64(stmt, 4, t.GetTimeStep());

            rc = sqlite3_step(stmt);
            CheckQueryReturnCode(stmt,
                                 rc,
                                 FormatBoundArgsList(e2NodeId, cellId, rnti, t.GetTimeStep()));
            sqlite3_finalize(stmt);
        }
    }
}

void
OranDataRepositorySqlite::SaveAppLoss(uint64_t e2NodeId, double appLoss, Time t)
{
    NS_LOG_FUNCTION(this << e2NodeId << appLoss << t);

    if (m_active)
    {
        if (IsNodeRegistered(e2NodeId))
        {
            int rc;
            std::string query;
            sqlite3_stmt* stmt = nullptr;

            query = "INSERT INTO nodeapploss (nodeid, loss, simulationtime)"
                    " VALUES (?, ?, ?)"
                    ";";

            sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, 0);

            sqlite3_bind_int64(stmt, 1, e2NodeId);
            sqlite3_bind_double(stmt, 2, appLoss);
            sqlite3_bind_int64(stmt, 3, t.GetTimeStep());

            rc = sqlite3_step(stmt);

            CheckQueryReturnCode(stmt, rc, FormatBoundArgsList(e2NodeId, appLoss, t.GetTimeStep()));
            sqlite3_finalize(stmt);
        }
    }
}

void
OranDataRepositorySqlite::SaveLteUeRsrpRsrq(uint64_t e2NodeId,
                                            Time t,
                                            uint16_t rnti,
                                            uint16_t cellId,
                                            double rsrp,
                                            double rsrq,
                                            bool isServing,
                                            uint8_t componentCarrierId)
{
    NS_LOG_FUNCTION(this << e2NodeId << t << +rnti << +cellId << rsrp << rsrq << isServing
                         << +componentCarrierId);

    if (m_active)
    {
        if (IsNodeRegistered(e2NodeId))
        {
            int rc;
            std::string query;
            sqlite3_stmt* stmt = nullptr;

            sqlite3_prepare_v2(m_db,
                               m_queryStmtsStrings[INSERT_LTE_UE_RSRP_RSRQ].c_str(),
                               -1,
                               &stmt,
                               0);

            sqlite3_bind_int64(stmt, 1, e2NodeId);
            sqlite3_bind_int64(stmt, 2, t.GetTimeStep());
            sqlite3_bind_int(stmt, 3, rnti);
            sqlite3_bind_int(stmt, 4, cellId);
            sqlite3_bind_double(stmt, 5, rsrp);
            sqlite3_bind_double(stmt, 6, rsrq);
            sqlite3_bind_int(stmt, 7, isServing);
            sqlite3_bind_int(stmt, 8, componentCarrierId);

            rc = sqlite3_step(stmt);

            CheckQueryReturnCode(stmt,
                                 rc,
                                 FormatBoundArgsList(e2NodeId,
                                                     t.GetTimeStep(),
                                                     rnti,
                                                     cellId,
                                                     rsrp,
                                                     rsrq,
                                                     isServing,
                                                     componentCarrierId));
            sqlite3_finalize(stmt);
        }
    }
}

std::map<Time, Vector>
OranDataRepositorySqlite::GetNodePositions(uint64_t e2NodeId,
                                           Time fromTime,
                                           Time toTime,
                                           uint64_t maxEntries)
{
    NS_LOG_FUNCTION(this << e2NodeId << fromTime << toTime << maxEntries);

    std::map<Time, Vector> nodePositions;

    if (m_active)
    {
        if (IsNodeRegistered(e2NodeId))
        {
            int rc;
            sqlite3_stmt* stmt = nullptr;

            sqlite3_prepare_v2(m_db,
                               m_queryStmtsStrings[GET_NODE_ALL_POSITIONS].c_str(),
                               -1,
                               &stmt,
                               0);

            sqlite3_bind_int64(stmt, 1, e2NodeId);
            sqlite3_bind_int64(stmt, 2, fromTime.GetTimeStep());
            sqlite3_bind_int64(stmt, 3, toTime.GetTimeStep());
            sqlite3_bind_int64(stmt, 4, maxEntries);

            while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
            {
                uint64_t timeStep = sqlite3_column_int64(stmt, 0);
                double x = sqlite3_column_double(stmt, 1);
                double y = sqlite3_column_double(stmt, 2);
                double z = sqlite3_column_double(stmt, 3);

                Time t = Time(timeStep);
                Vector pos = Vector(x, y, z);

                nodePositions[t] = pos;
            }

            CheckQueryReturnCode(
                stmt,
                rc,
                FormatBoundArgsList(e2NodeId, fromTime.GetTimeStep(), toTime.GetTimeStep()));
            sqlite3_finalize(stmt);
        }
    }
    return nodePositions;
}

std::tuple<bool, uint16_t, uint16_t>
OranDataRepositorySqlite::GetLteUeCellInfo(uint64_t e2NodeId)
{
    NS_LOG_FUNCTION(this << e2NodeId);

    auto retVal = std::make_tuple(false, 0, 0);
    if (m_active)
    {
        if (IsNodeRegistered(e2NodeId))
        {
            int rc;
            sqlite3_stmt* stmt = nullptr;

            sqlite3_prepare_v2(m_db,
                               m_queryStmtsStrings[GET_LTE_UE_CELLINFO].c_str(),
                               -1,
                               &stmt,
                               0);
            sqlite3_bind_int64(stmt, 1, e2NodeId);

            while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
            {
                uint16_t cellId = sqlite3_column_int(stmt, 0);
                uint16_t rnti = sqlite3_column_int(stmt, 1);
                retVal = std::make_tuple(true, cellId, rnti);
            }

            CheckQueryReturnCode(stmt, rc, FormatBoundArgsList(e2NodeId));
            sqlite3_finalize(stmt);
        }
    }
    return retVal;
}

std::vector<uint64_t>
OranDataRepositorySqlite::GetLteUeE2NodeIds()
{
    NS_LOG_FUNCTION(this);

    std::vector<uint64_t> e2NodeIds;

    if (m_active)
    {
        int rc;
        sqlite3_stmt* stmt = nullptr;

        if (sqlite3_prepare_v2(m_db,
                               m_queryStmtsStrings[GET_LTE_ALL_UE_E2NODEIDS].c_str(),
                               -1,
                               &stmt,
                               0) != SQLITE_OK)
        {
            std::cerr << "SQL Error: " << sqlite3_errmsg(m_db) << std::endl;
        }

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            e2NodeIds.push_back(sqlite3_column_int64(stmt, 0));
        }

        CheckQueryReturnCode(stmt, rc);
        sqlite3_finalize(stmt);
    }
    return e2NodeIds;
}

double
OranDataRepositorySqlite::GetAppLoss(uint64_t e2NodeId)
{
    NS_LOG_FUNCTION(this << e2NodeId);

    double loss = 0;

    if (m_active)
    {
        if (IsNodeRegistered(e2NodeId))
        {
            int rc;
            std::string query;
            sqlite3_stmt* stmt = nullptr;

            query = "SELECT loss"
                    " FROM nodeapploss"
                    " WHERE nodeid = ?"
                    " ORDER BY entryid DESC LIMIT 1"
                    ";";

            sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, 0);

            sqlite3_bind_int64(stmt, 1, e2NodeId);

            while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
            {
                loss = sqlite3_column_double(stmt, 0);
            }

            CheckQueryReturnCode(stmt, rc, FormatBoundArgsList(e2NodeId));
            sqlite3_finalize(stmt);
        }
    }
    return loss;
}

uint64_t
OranDataRepositorySqlite::GetLteUeE2NodeIdFromCellInfo(uint16_t cellId, uint16_t rnti)
{
    NS_LOG_FUNCTION(this << cellId << rnti);

    uint64_t id = 0;
    if (m_active)
    {
        int rc;
        sqlite3_stmt* stmt = nullptr;

        sqlite3_prepare_v2(m_db,
                           m_queryStmtsStrings[GET_LTE_UE_E2NODEID_FROM_CELLINFO].c_str(),
                           -1,
                           &stmt,
                           0);
        sqlite3_bind_int(stmt, 1, cellId);
        sqlite3_bind_int(stmt, 2, rnti);

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            id = sqlite3_column_int64(stmt, 0);
        }

        CheckQueryReturnCode(stmt, rc, FormatBoundArgsList(cellId, rnti));
        sqlite3_finalize(stmt);
    }
    return id;
}

std::tuple<bool, uint16_t>
OranDataRepositorySqlite::GetLteEnbCellInfo(uint64_t e2NodeId)
{
    NS_LOG_FUNCTION(this << e2NodeId);

    auto retVal = std::make_tuple(false, 0);
    if (m_active)
    {
        if (IsNodeRegistered(e2NodeId))
        {
            int rc;
            sqlite3_stmt* stmt = nullptr;

            sqlite3_prepare_v2(m_db,
                               m_queryStmtsStrings[GET_LTE_CELLID_FROM_E2NODEID].c_str(),
                               -1,
                               &stmt,
                               0);
            sqlite3_bind_int64(stmt, 1, e2NodeId);

            while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
            {
                uint16_t cellId = sqlite3_column_int(stmt, 0);
                retVal = std::make_tuple(true, cellId);
            }

            CheckQueryReturnCode(stmt, rc, FormatBoundArgsList(e2NodeId));
            sqlite3_finalize(stmt);
        }
    }
    return retVal;
}

std::vector<uint64_t>
OranDataRepositorySqlite::GetLteEnbE2NodeIds()
{
    NS_LOG_FUNCTION(this);

    std::vector<uint64_t> e2NodeIds;

    if (m_active)
    {
        int rc;
        sqlite3_stmt* stmt = nullptr;

        sqlite3_prepare_v2(m_db,
                           m_queryStmtsStrings[GET_LTE_ALL_ENB_E2NODEIDS].c_str(),
                           -1,
                           &stmt,
                           0);

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            e2NodeIds.push_back(sqlite3_column_int64(stmt, 0));
        }

        CheckQueryReturnCode(stmt, rc);
        sqlite3_finalize(stmt);
    }
    return e2NodeIds;
}

std::vector<std::tuple<uint64_t, Time>>
OranDataRepositorySqlite::GetLastRegistrationRequests()
{
    NS_LOG_FUNCTION(this);

    std::vector<std::tuple<uint64_t, Time>> requests;
    if (m_active)
    {
        int rc;
        sqlite3_stmt* stmt = nullptr;

        sqlite3_prepare_v2(m_db,
                           m_queryStmtsStrings[GET_ALL_LAST_REGISTRATION_TIMES].c_str(),
                           -1,
                           &stmt,
                           0);

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            uint64_t e2NodeId = sqlite3_column_int64(stmt, 0);
            Time t = Time(sqlite3_column_int64(stmt, 1));

            requests.push_back(std::make_tuple(e2NodeId, t));
        }

        CheckQueryReturnCode(stmt, rc);
        sqlite3_finalize(stmt);
    }

    return requests;
}

std::vector<std::tuple<uint16_t, uint16_t, double, double, bool, uint8_t>>
OranDataRepositorySqlite::GetLteUeRsrpRsrq(uint64_t e2NodeId)
{
    NS_LOG_FUNCTION(this << e2NodeId);

    std::vector<std::tuple<uint16_t, uint16_t, double, double, bool, uint8_t>> retVal;

    if (m_active)
    {
        if (IsNodeRegistered(e2NodeId))
        {
            int rc;
            sqlite3_stmt* stmt = nullptr;

            sqlite3_prepare_v2(m_db,
                               m_queryStmtsStrings[GET_LTE_UE_RSRP_RSRQ].c_str(),
                               -1,
                               &stmt,
                               0);
            sqlite3_bind_int64(stmt, 1, e2NodeId);
            sqlite3_bind_int64(stmt, 2, e2NodeId);

            while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
            {
                uint16_t rnti = sqlite3_column_int(stmt, 0);
                uint16_t cellId = sqlite3_column_int(stmt, 1);
                double rsrp = sqlite3_column_double(stmt, 2);
                double rsrq = sqlite3_column_double(stmt, 3);
                bool isServing = sqlite3_column_int(stmt, 4);
                uint8_t componentCarrierId = sqlite3_column_int(stmt, 5);

                retVal.push_back(
                    std::make_tuple(rnti, cellId, rsrp, rsrq, isServing, componentCarrierId));
            }

            CheckQueryReturnCode(stmt, rc, FormatBoundArgsList(e2NodeId, e2NodeId));
            sqlite3_finalize(stmt);
        }
    }
    return retVal;
}

void
OranDataRepositorySqlite::LogCommandE2Terminator(Ptr<OranCommand> cmd)
{
    NS_LOG_FUNCTION(this);

    if (m_active)
    {
        if (IsNodeRegistered(cmd->GetTargetE2NodeId()))
        {
            int rc;
            sqlite3_stmt* stmt = nullptr;

            sqlite3_prepare_v2(m_db,
                               m_queryStmtsStrings[LOG_E2TERMINATOR_COMMAND].c_str(),
                               -1,
                               &stmt,
                               0);

            sqlite3_bind_int64(stmt, 1, cmd->GetTargetE2NodeId());
            sqlite3_bind_int64(stmt, 2, Simulator::Now().GetTimeStep());
            sqlite3_bind_text(stmt, 3, cmd->ToString().c_str(), -1, 0);

            rc = sqlite3_step(stmt);
            CheckQueryReturnCode(stmt,
                                 rc,
                                 FormatBoundArgsList(cmd->GetTargetE2NodeId(),
                                                     Simulator::Now().GetTimeStep(),
                                                     cmd->ToString()));
            sqlite3_finalize(stmt);
        }
    }
}

void
OranDataRepositorySqlite::LogCommandLm(std::string lm, Ptr<OranCommand> cmd)
{
    NS_LOG_FUNCTION(this);

    if (m_active)
    {
        int rc;
        sqlite3_stmt* stmt = nullptr;

        sqlite3_prepare_v2(m_db, m_queryStmtsStrings[LOG_LM_COMMAND].c_str(), -1, &stmt, 0);

        sqlite3_bind_text(stmt, 1, lm.c_str(), -1, 0);
        sqlite3_bind_int64(stmt, 2, Simulator::Now().GetTimeStep());
        sqlite3_bind_text(stmt, 3, cmd->ToString().c_str(), -1, 0);

        rc = sqlite3_step(stmt);
        CheckQueryReturnCode(
            stmt,
            rc,
            FormatBoundArgsList(lm, Simulator::Now().GetTimeStep(), cmd->ToString()));
        sqlite3_finalize(stmt);
    }
}

void
OranDataRepositorySqlite::LogActionLm(std::string lm, std::string logStr)
{
    NS_LOG_FUNCTION(this << lm << logStr);

    if (m_active)
    {
        int rc;
        sqlite3_stmt* stmt = nullptr;

        sqlite3_prepare_v2(m_db, m_queryStmtsStrings[LOG_LM_ACTION].c_str(), -1, &stmt, 0);

        sqlite3_bind_text(stmt, 1, lm.c_str(), -1, 0);
        sqlite3_bind_int64(stmt, 2, Simulator::Now().GetTimeStep());
        sqlite3_bind_text(stmt, 3, logStr.c_str(), -1, 0);

        rc = sqlite3_step(stmt);

        CheckQueryReturnCode(stmt,
                             rc,
                             FormatBoundArgsList(lm, Simulator::Now().GetTimeStep(), logStr));
        sqlite3_finalize(stmt);
    }
}

void
OranDataRepositorySqlite::LogActionCmm(std::string cmm, std::string logStr)
{
    NS_LOG_FUNCTION(this << cmm << logStr);

    if (m_active)
    {
        int rc;
        sqlite3_stmt* stmt = nullptr;

        sqlite3_prepare_v2(m_db, m_queryStmtsStrings[LOG_CMM_ACTION].c_str(), -1, &stmt, 0);

        sqlite3_bind_text(stmt, 1, cmm.c_str(), -1, 0);
        sqlite3_bind_int64(stmt, 2, Simulator::Now().GetTimeStep());
        sqlite3_bind_text(stmt, 3, logStr.c_str(), -1, 0);

        rc = sqlite3_step(stmt);

        CheckQueryReturnCode(stmt,
                             rc,
                             FormatBoundArgsList(cmm, Simulator::Now().GetTimeStep(), logStr));
        sqlite3_finalize(stmt);
    }
}

void
OranDataRepositorySqlite::CheckQueryReturnCode(sqlite3_stmt* stmt,
                                               int rc,
                                               std::string boundParmsStr) const
{
    NS_LOG_FUNCTION(this << stmt << rc);

    // Get the formated string of the prepared statement
    std::string stmtStr = sqlite3_sql(stmt);

    // Trace the result of the query
    m_queryRc(stmtStr, boundParmsStr, rc);

    if (rc == SQLITE_OK || rc == SQLITE_DONE)
    {
        NS_LOG_INFO("Query SUCCESSFUL: \"" << stmtStr << "\"; " << boundParmsStr);
    }
    else
    {
        NS_ABORT_MSG("Query FAILED: \"" << stmtStr << "\"; (" << boundParmsStr << "); RC = " << rc);
    }
}

void
OranDataRepositorySqlite::CloseDb()
{
    NS_LOG_FUNCTION(this);

    sqlite3_close(m_db);
    m_db = nullptr;
}

void
OranDataRepositorySqlite::DoDispose()
{
    NS_LOG_FUNCTION(this);

    if (IsDbOpen())
    {
        CloseDb();
    }

    OranDataRepository::DoDispose();
}

bool
OranDataRepositorySqlite::IsDbOpen() const
{
    NS_LOG_FUNCTION(this);

    return m_db != nullptr;
}

void
OranDataRepositorySqlite::OpenDb()
{
    NS_LOG_FUNCTION(this);

    // Check for special file names and print a warning if we find them
    if (m_dbPath == ":memory:")
    {
        NS_LOG_WARN("Using in-memory DB for the ORAN Storage. DB will not be saved to disk.");
        std::cerr
            << "WARNING: Using in-memory DB for the ORAN Storage. DB will not be saved to disk."
            << std::endl;
    }
    else
    {
        // Check for DB names that are URIs. We do not support those
        if (m_dbPath.find(":") != std::string::npos)
        {
            NS_ABORT_MSG("File name for the ORAN Storage DB ("
                         << m_dbPath << ") is an URI. URI-named DBs are not supported");
        }
    }

    if (m_dbPath.empty())
    {
        NS_LOG_WARN("Using a randomly named temporary file as DB for the ORAN Storage. DB will not "
                    "be available after simulation ends.");
        std::cerr << "WARNING: Using a randomly named temporary file as DB for the ORAN Storage. "
                     "DB will not be available after simulation ends."
                  << std::endl;
    }

    int error = sqlite3_open(m_dbPath.c_str(), &m_db);
    if (error != 0)
    {
        NS_ABORT_MSG("Could not open database: " << sqlite3_errmsg(m_db));
    }
    else
    {
        NS_LOG_INFO("Oran repository \"" << m_dbPath << "\" connected to successfully!");
        ;
    }

    InitDb();
}

void
OranDataRepositorySqlite::InitDb()
{
    NS_LOG_FUNCTION(this);

    // E2 Node Table
    RunCreateStatement(m_createStmtsStrings[TABLE_NODE]);
    RunCreateStatement(m_createStmtsStrings[INDEX_NODE]);

    // E2 Node Registration
    RunCreateStatement(m_createStmtsStrings[TABLE_NODE_REGISTRATION]);
    RunCreateStatement(m_createStmtsStrings[INDEX_NODE_REGISTRATION]);

    // E2 Node Location
    RunCreateStatement(m_createStmtsStrings[TABLE_NODE_LOCATION]);
    RunCreateStatement(m_createStmtsStrings[INDEX_NODE_LOCATION]);

    // LTE eNB
    RunCreateStatement(m_createStmtsStrings[TABLE_LTE_ENB]);
    RunCreateStatement(m_createStmtsStrings[INDEX_LTE_ENB_NODEID]);
    RunCreateStatement(m_createStmtsStrings[INDEX_LTE_ENB_CELLID]);

    // LTE UE
    RunCreateStatement(m_createStmtsStrings[TABLE_LTE_UE]);
    RunCreateStatement(m_createStmtsStrings[INDEX_LTE_UE_NODEID]);
    RunCreateStatement(m_createStmtsStrings[INDEX_LTE_UE_IMSI]);

    // LTE UE Cell Information
    RunCreateStatement(m_createStmtsStrings[TABLE_LTE_UE_CELL]);
    RunCreateStatement(m_createStmtsStrings[TABLE_LTE_UE_RSRP_RSRQ]);
    RunCreateStatement(m_createStmtsStrings[INDEX_LTE_UE_CELL_NODEID]);
    RunCreateStatement(m_createStmtsStrings[INDEX_LTE_UE_CELL_CELLID]);

    RunCreateStatement(m_createStmtsStrings[TABLE_APPLOSS_COMMAND]);

    // E2 Terminator Commands
    RunCreateStatement(m_createStmtsStrings[TABLE_TERMINATOR_COMMAND]);

    // LM Commands
    RunCreateStatement(m_createStmtsStrings[TABLE_LM_COMMAND]);

    // LM Actions (Internal Log)
    RunCreateStatement(m_createStmtsStrings[TABLE_LM_ACTION]);

    // CMM Actions (Internal Log)
    RunCreateStatement(m_createStmtsStrings[TABLE_CMM_ACTION]);
}

void
OranDataRepositorySqlite::InitStatements()
{
    NS_LOG_FUNCTION(this);

    // Initialize the create statements
    m_createStmtsStrings[INDEX_LTE_ENB_CELLID] = "CREATE INDEX IF NOT EXISTS "
                                                 "idx_lteenb_cellid ON lteenb(cellid);";

    m_createStmtsStrings[INDEX_LTE_ENB_NODEID] = "CREATE INDEX IF NOT EXISTS "
                                                 "idx_lteenb_nodeid ON lteenb(nodeid);";

    m_createStmtsStrings[INDEX_LTE_UE_CELL_CELLID] = "CREATE INDEX IF NOT EXISTS "
                                                     "idx_lteuecell_cellid ON lteuecell(cellid);";

    m_createStmtsStrings[INDEX_LTE_UE_CELL_NODEID] = "CREATE INDEX IF NOT EXISTS "
                                                     "idx_lteuecell_nodeid ON lteuecell(nodeid);";

    m_createStmtsStrings[INDEX_LTE_UE_IMSI] = "CREATE INDEX IF NOT EXISTS "
                                              "idx_lteue_imsi ON lteue(imsi);";

    m_createStmtsStrings[INDEX_LTE_UE_NODEID] = "CREATE INDEX IF NOT EXISTS "
                                                "idx_lteue_nodeid ON lteue(nodeid);";

    m_createStmtsStrings[INDEX_NODE] = "CREATE INDEX IF NOT EXISTS "
                                       "idx_node_nodeid ON node (nodeid);";

    m_createStmtsStrings[INDEX_NODE_LOCATION] = "CREATE INDEX IF NOT EXISTS "
                                                "idx_nodelocation_nodeid ON nodelocation(nodeid);";

    m_createStmtsStrings[INDEX_NODE_REGISTRATION] =
        "CREATE INDEX IF NOT EXISTS "
        "idx_noderegistration_nodeid ON noderegistration(nodeid);";

    m_createStmtsStrings[TABLE_CMM_ACTION] =
        "CREATE TABLE IF NOT EXISTS cmmaction ("
        "entryid        INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
        "cmmname        TEXT                              NOT NULL, "
        "simulationtime INTEGER                           NOT NULL, "
        "description    TEXT                              NOT NULL);";

    m_createStmtsStrings[TABLE_LM_ACTION] =
        "CREATE TABLE IF NOT EXISTS lmaction ("
        "entryid        INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
        "lmname         TEXT                              NOT NULL, "
        "simulationtime INTEGER                           NOT NULL, "
        "description    TEXT                              NOT NULL);";

    m_createStmtsStrings[TABLE_LM_COMMAND] =
        "CREATE TABLE IF NOT EXISTS lmcommand ("
        "entryid        INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
        "lmname         TEXT                              NOT NULL, "
        "simulationtime INTEGER                           NOT NULL, "
        "cmdname        TEXT                              NOT NULL);";

    m_createStmtsStrings[TABLE_LTE_ENB] = "CREATE TABLE IF NOT EXISTS lteenb ("
                                          "nodeid INTEGER PRIMARY KEY NOT NULL, "
                                          "cellid INTEGER             NOT NULL, "
                                          "FOREIGN KEY(nodeid) REFERENCES node(nodeid));";

    m_createStmtsStrings[TABLE_LTE_UE] = "CREATE TABLE IF NOT EXISTS lteue ("
                                         "nodeid INTEGER PRIMARY KEY NOT NULL, "
                                         "imsi   INTEGER UNIQUE      NOT NULL, "
                                         "FOREIGN KEY(nodeid) REFERENCES node(nodeid));";

    m_createStmtsStrings[TABLE_LTE_UE_CELL] =
        "CREATE TABLE IF NOT EXISTS lteuecell ("
        "entryid        INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
        "nodeid         INTEGER                           NOT NULL, "
        "cellid         INTEGER                           NOT NULL, "
        "rnti           INTEGER                           NOT NULL, "
        "simulationtime INTEGER                           NOT NULL, "
        "FOREIGN KEY(cellid) REFERENCES lteenb(cellid), "
        "FOREIGN KEY(nodeid) REFERENCES lteue(nodeid));";

    m_createStmtsStrings[TABLE_LTE_UE_RSRP_RSRQ] =
        "CREATE TABLE IF NOT EXISTS lteuersrprsrq ("
        "entryid        INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
        "nodeid         INTEGER                           NOT NULL, "
        "simulationtime INTEGER                           NOT NULL, "
        "rnti           INTEGER                           NOT NULL, "
        "cellid         INTEGER                           NOT NULL, "
        "rsrp           REAL                              NOT NULL, "
        "rsrq           REAL                              NOT NULL, "
        "serving        BOOLEAN                           NOT NULL, "
        "ccid           BOOLEAN                           NOT NULL, "
        "FOREIGN KEY(cellid) REFERENCES lteenb(cellid), "
        "FOREIGN KEY(nodeid) REFERENCES lteue(nodeid));";

    m_createStmtsStrings[TABLE_NODE] =
        "CREATE TABLE IF NOT EXISTS node ("
        "nodeid         INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
        "nodetype       INTEGER                           NOT NULL);";

    m_createStmtsStrings[TABLE_NODE_LOCATION] =
        "CREATE TABLE IF NOT EXISTS nodelocation ("
        "entryid        INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
        "nodeid         INTEGER                           NOT NULL, "
        "x              REAL                              NOT NULL, "
        "y              REAL                              NOT NULL, "
        "z              REAL                              NOT NULL, "
        "simulationtime INTEGER                           NOT NULL, "
        "FOREIGN KEY(nodeid) REFERENCES node(nodeid));";

    m_createStmtsStrings[TABLE_NODE_REGISTRATION] =
        "CREATE TABLE IF NOT EXISTS noderegistration ("
        "entryid        INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
        "nodeid         INTEGER                           NOT NULL, "
        "registered     BOOLEAN                           NOT NULL, "
        "simulationtime INTEGER                           NOT NULL, "
        "FOREIGN KEY(nodeid) REFERENCES node(nodeid));";

    m_createStmtsStrings[TABLE_TERMINATOR_COMMAND] =
        "CREATE TABLE IF NOT EXISTS terminatorcommand ("
        "entryid        INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
        "targetid       INTEGER                           NOT NULL, "
        "simulationtime INTEGER                           NOT NULL, "
        "cmdname        TEXT                              NOT NULL, "
        "FOREIGN KEY(targetid) REFERENCES node(nodeid));";

    m_createStmtsStrings[TABLE_APPLOSS_COMMAND] =
        "CREATE TABLE IF NOT EXISTS nodeapploss ("
        "entryid        INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
        "nodeid         INTEGER                           NOT NULL, "
        "loss           REAL                              NOT NULL, "
        "simulationtime INTEGER                           NOT NULL, "
        "FOREIGN KEY(nodeid) REFERENCES node(nodeid)              );";

    // Query Statements
    m_queryStmtsStrings[CHECK_NODE_REGISTERED] = "SELECT registered "
                                                 "FROM noderegistration "
                                                 "WHERE nodeid = ? "
                                                 "ORDER BY simulationtime DESC, entryid DESC "
                                                 "LIMIT 1;";

    m_queryStmtsStrings[GET_ALL_LAST_REGISTRATION_TIMES] = "SELECT nodeid, MAX(simulationtime) "
                                                           "FROM noderegistration "
                                                           "GROUP BY nodeid "
                                                           "HAVING registered = 1 "
                                                           "ORDER BY nodeid;";

    m_queryStmtsStrings[GET_LTE_ALL_ENB_E2NODEIDS] =
        "SELECT nr.nodeid, MAX(nr.simulationtime) "
        "FROM noderegistration AS nr "
        "INNER JOIN lteenb ON lteenb.nodeid = nr.nodeid "
        "GROUP BY nr.nodeid "
        "HAVING nr.registered = 1 "
        "ORDER BY nr.nodeid;";

    m_queryStmtsStrings[GET_LTE_ALL_UE_E2NODEIDS] = "SELECT nr.nodeid, MAX(nr.simulationtime) "
                                                    "FROM noderegistration AS nr "
                                                    "INNER JOIN lteue ON lteue.nodeid = nr.nodeid "
                                                    "GROUP BY nr.nodeid "
                                                    "HAVING nr.registered = 1 "
                                                    "ORDER BY nr.nodeid;";

    m_queryStmtsStrings[GET_LTE_CELLID_FROM_E2NODEID] = "SELECT cellid "
                                                        "FROM lteenb "
                                                        "WHERE nodeid = ?;";

    m_queryStmtsStrings[GET_LTE_UE_CELLINFO] = "SELECT cellid, rnti "
                                               "FROM lteuecell "
                                               "WHERE nodeid = ? "
                                               "ORDER BY simulationtime DESC, entryid DESC "
                                               "LIMIT 1;";

    m_queryStmtsStrings[GET_LTE_UE_E2NODEID_FROM_CELLINFO] = "SELECT nodeid "
                                                             "FROM lteuecell "
                                                             "WHERE cellid = ? AND rnti = ? "
                                                             "ORDER BY entryid DESC LIMIT 1;";

    m_queryStmtsStrings[GET_NODE_ALL_POSITIONS] =
        "SELECT simulationtime, x, y, z "
        "FROM nodelocation "
        "WHERE nodeid = ? AND simulationtime >= ? AND simulationtime <= ? "
        "ORDER BY simulationtime DESC, entryid DESC LIMIT ? ;";

    m_queryStmtsStrings[GET_LTE_UE_RSRP_RSRQ] = "SELECT rnti, cellid, rsrp, rsrq, serving, ccid "
                                                "FROM lteuersrprsrq "
                                                "WHERE nodeid = ? "
                                                "AND simulationtime IN ("
                                                "SELECT simulationtime "
                                                "FROM lteuersrprsrq "
                                                "WHERE nodeid = ? "
                                                "ORDER BY simulationtime DESC LIMIT 1"
                                                ");";

    m_queryStmtsStrings[INSERT_LTE_ENB_NODE] = "INSERT OR REPLACE INTO lteenb "
                                               "(nodeid, cellid) VALUES (?, ?);";

    m_queryStmtsStrings[INSERT_LTE_UE_CELL] =
        "INSERT INTO lteuecell "
        "(nodeid, cellid, rnti, simulationtime) VALUES (?, ?, ?, ?);";

    m_queryStmtsStrings[INSERT_LTE_UE_NODE] = "INSERT OR REPLACE INTO lteue "
                                              "(nodeid, imsi) VALUES (?, ?);";

    m_queryStmtsStrings[INSERT_NODE_ADD] = "INSERT INTO node "
                                           "(nodetype) VALUES (?);";

    m_queryStmtsStrings[INSERT_NODE_UPDATE] = "INSERT OR REPLACE INTO node "
                                              "(nodeid, nodetype) VALUES (?, ?);";

    m_queryStmtsStrings[INSERT_NODE_LOCATION] =
        "INSERT INTO nodelocation "
        "(nodeid, x, y, z, simulationtime) VALUES (?, ?, ?, ?, ?);";

    m_queryStmtsStrings[INSERT_NODE_REGISTRATION] =
        "INSERT INTO noderegistration "
        "(nodeid, registered, simulationtime) VALUES (?, ?, ?);";

    m_queryStmtsStrings[INSERT_LTE_UE_RSRP_RSRQ] =
        "INSERT INTO lteuersrprsrq "
        "(nodeid, simulationtime, rnti, cellid, rsrp, rsrq, serving, ccid) VALUES (?, ?, ?, ?, ?, "
        "?, ?, ?);";

    m_queryStmtsStrings[LOG_CMM_ACTION] =
        "INSERT INTO cmmaction "
        "(cmmname, simulationtime, description) VALUES (?, ?, ?);";

    m_queryStmtsStrings[LOG_E2TERMINATOR_COMMAND] =
        "INSERT INTO terminatorcommand "
        "(targetid, simulationtime, cmdname) VALUES (?, ?, ?);";

    m_queryStmtsStrings[LOG_LM_ACTION] = "INSERT INTO lmaction "
                                         "(lmname, simulationtime, description) VALUES (?, ?, ?);";

    m_queryStmtsStrings[LOG_LM_COMMAND] = "INSERT INTO lmcommand "
                                          "(lmname, simulationtime, cmdname) VALUES (?, ?, ?);";
}

void
OranDataRepositorySqlite::RunCreateStatement(std::string statement)
{
    NS_LOG_FUNCTION(this << statement);

    // Keep the return code in a separate variable to make it easier to debug
    // Otherwise, we could just run the sqlite3_step as the 2nd argument to the
    // CheckQueryReturnCode call
    int rc;
    sqlite3_stmt* stmt;

    sqlite3_prepare_v2(m_db, statement.c_str(), -1, &stmt, 0);
    rc = sqlite3_step(stmt);
    CheckQueryReturnCode(stmt, rc);
    sqlite3_finalize(stmt);
}

} // namespace ns3
