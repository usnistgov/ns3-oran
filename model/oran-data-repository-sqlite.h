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

#ifndef ORAN_DATA_REPOSITORY_SQLITE_H
#define ORAN_DATA_REPOSITORY_SQLITE_H

#include "oran-data-repository.h"

#include "ns3/traced-callback.h"

#include <sqlite3.h>
#include <sstream>

namespace ns3
{

/**
 * @ingroup oran
 *
 * A Data Repository implementation that uses an SQLite database as
 * the storage backend.
 *
 * The database used as the backend may be created new, or it may be an existing
 * database. This class does not provide methods for deleting existing database
 * files; if this is required, the user must take care of that in the scenario.
 *
 * The methods defined in the OranDataRepository API build SQL prepared
 * statements to access the database, validating the return code after each
 * database query.
 */
class OranDataRepositorySqlite : public OranDataRepository
{
  public:
    /**
     * Gets the TypeId of the OranDataRepositorySqlite class.
     *
     * @return The TypeId.
     */
    static TypeId GetTypeId();
    /**
     * Creates an instance of the OranDataRepositorySqlite class.
     */
    OranDataRepositorySqlite();
    /**
     * The destructor of the OranDataRepositorySqlite class.
     */
    ~OranDataRepositorySqlite() override;
    /**
     * Activate the data storage. If the database is not open,
     * this method will call OpenDb.
     */
    void Activate() override;
    /**
     * Deactivate the data storage. If the database is open,
     * this method will call CloseDb.
     */
    void Deactivate() override;

    /* Data Storage API */
    bool IsNodeRegistered(uint64_t e2NodeId) override;

    uint64_t RegisterNode(OranNearRtRic::NodeType type, uint64_t id) override;
    uint64_t RegisterNodeLteUe(uint64_t id, uint64_t imsi) override;
    uint64_t RegisterNodeLteEnb(uint64_t id, uint16_t cellId) override;
    uint64_t DeregisterNode(uint64_t e2NodeId) override;
    void SavePosition(uint64_t e2NodeId, Vector pos, Time t) override;
    void SaveLteUeCellInfo(uint64_t e2NodeId, uint16_t cellId, uint16_t rnti, Time t) override;
    void SaveAppLoss(uint64_t e2NodeId, double appLoss, Time t) override;
    void SaveLteUeRsrpRsrq(uint64_t e2NodeId,
                           Time t,
                           uint16_t rnti,
                           uint16_t cellId,
                           double rsrp,
                           double rsrq,
                           bool isServingCell,
                           uint8_t componentCarrierId) override;

    std::map<Time, Vector> GetNodePositions(uint64_t e2NodeId,
                                            Time fromTime,
                                            Time toTime,
                                            uint64_t maxEntries = 1) override;
    std::tuple<bool, uint16_t, uint16_t> GetLteUeCellInfo(uint64_t e2NodeId) override;
    std::vector<uint64_t> GetLteUeE2NodeIds() override;
    uint64_t GetLteUeE2NodeIdFromCellInfo(uint16_t cellId, uint16_t rnti) override;
    std::tuple<bool, uint16_t> GetLteEnbCellInfo(uint64_t e2NodeId) override;
    std::vector<uint64_t> GetLteEnbE2NodeIds() override;
    std::vector<std::tuple<uint64_t, Time>> GetLastRegistrationRequests() override;
    double GetAppLoss(uint64_t e2NodeId) override;
    std::vector<std::tuple<uint16_t, uint16_t, double, double, bool, uint8_t>> GetLteUeRsrpRsrq(
        uint64_t e2NodeId) override;

    void LogCommandE2Terminator(Ptr<OranCommand> cmd) override;
    void LogCommandLm(std::string lm, Ptr<OranCommand> cmd) override;
    void LogActionLm(std::string lm, std::string logstr) override;
    void LogActionCmm(std::string cmm, std::string logstr) override;

    /**
     * TracedCallback signature for SQL Queries. Traces the queries and the result code
     * (does not trace the returned records).
     *
     * @param [in] query The SQL prepared statement
     * @param [in] args The bound arguments (if any)
     * @param [in] rc The return code
     */
    typedef void (*QueryRcTracedCallback)(std::string query, std::string args, int rc);

  protected:
    /**
     * Enumeration with the type of SQL statement.
     * To be used as key for the map with the statement's strings
     */
    enum StatementType
    {
        CHECK_NODE_REGISTERED = 0,         //!< Query if a node is registered
        GET_ALL_LAST_REGISTRATION_TIMES,   //!< Get node registation times
        GET_LTE_ALL_ENB_E2NODEIDS,         //!< Get all LTE eNB E2 IDs
        GET_LTE_ALL_UE_E2NODEIDS,          //!< Get all LTE UE E2 IDs
        GET_LTE_CELLID_FROM_E2NODEID,      //!< Get the cell ID of an LTE eNB from its E2 Node ID
        GET_LTE_UE_CELLINFO,               //!< Get the cell information associated with LTE UE
        GET_LTE_UE_E2NODEID_FROM_CELLINFO, //!< Get the E2 ID of a UE from the cell information
        GET_LTE_UE_RSRP_RSRQ,              //!< Get the UE RSRP and RSRQ measurements
        GET_NODE_ALL_POSITIONS,            //!< The location of all nodes E2 nodes
        INSERT_LTE_ENB_NODE,               //!< Add an LTE eNB E2 node
        INSERT_LTE_UE_CELL,                //!< Add LTE UE cell information for an E2 node
        INSERT_LTE_UE_NODE,                //!< Add an LTE UE E2 node
        INSERT_NODE_ADD,                   //!< Add an E2 node
        INSERT_NODE_UPDATE,                //!< Update an E2 node's information
        INSERT_NODE_LOCATION,              //!< Add an E2 node's location
        INSERT_NODE_REGISTRATION,          //!< Add an E2 node registration request
        INSERT_LTE_UE_RSRP_RSRQ,           //!< Add LTE UE RSRP and RSRQ
        LOG_CMM_ACTION,                    //!< Log a CM module action
        LOG_E2TERMINATOR_COMMAND,          //!< Log an E2 terminator command from the RIC
        LOG_LM_ACTION,                     //!< Log an LM action
        LOG_LM_COMMAND                     //!< Log an LM command
    };

    /**
     * Enumeration with the type of SQL CREATE TABLE statements
     * To be used as key for the map with the CREATE TABLE statements' strings
     */
    enum CreateStatementType
    {
        INDEX_LTE_ENB_CELLID = 0, //!< Index for the table with LTE eNB based on Cell IDs
        INDEX_LTE_ENB_NODEID,     //!< Index for the table with LTE eNB based on E2 Node IDs
        INDEX_LTE_UE_CELL_CELLID, //!< Index for the table with LTE UE Cell Information based on
                                  //!< Cell IDs
        INDEX_LTE_UE_CELL_NODEID, //!< Index for the table with LTE UE Cell Information based on E2
                                  //!< Node IDs
        INDEX_LTE_UE_IMSI,        //!< Index for the table with LTE UE based on IMSI
        INDEX_LTE_UE_NODEID,      //!< Index for the table with LTE UE based on E2 Node ID
        INDEX_NODE,               //!< Index for the table with E2 Node Information
        INDEX_NODE_LOCATION,      //!< Index for the table with Node Locations
        INDEX_NODE_REGISTRATION,  //!< Index for the table with Node Registrations
        TABLE_CMM_ACTION,         //!< Table with logs of CMM actions
        TABLE_LM_ACTION,          //!< Table with logs of LM actions
        TABLE_LM_COMMAND,         //!< Table with logs of LM commamds
        TABLE_LTE_ENB,            //!< Table with LTE eNB information
        TABLE_LTE_UE,             //!< Table with LTE UE information
        TABLE_LTE_UE_CELL,        //!< Table with LTE UE Cell Information
        TABLE_LTE_UE_RSRP_RSRQ,   //!< Table with LTE UE RSRP and RSRQ Information
        TABLE_NODE,               //!< Table with E2 Node Information
        TABLE_NODE_LOCATION,      //!< Table with Node Locations
        TABLE_NODE_REGISTRATION,  //!< Table with Node Registrations
        TABLE_TERMINATOR_COMMAND, //!< Table with logs of E2 Terminator Commands
        TABLE_APPLOSS_COMMAND     //!< Table with logs of application loss Commands
    };

    /**
     * Checks that a query was executed successfully. This method checks the return codeof a query,
     * and if there was an error, the simulation is aborted.
     *
     * @param stmt The query that was executed.
     * @param rc The return code.
     * @param boundParmsStr String with the bound parameters (if any). Defaults to empty string.
     */
    virtual void CheckQueryReturnCode(sqlite3_stmt* stmt,
                                      int rc,
                                      std::string boundParmsStr = "") const;

    /**
     * Converts the bound arguments of a prepared statement into a formatted string.
     * Newer versions of sqlite3 can print prepared statements directly, but using
     * this approach ensures support for older versions.
     *
     * @param arg1 The first argument
     * @param args The remaining list of arguments.
     *
     * @return A string with the bound arguments separated with commas
     */
    template <typename T, typename... BoundArgs>
    std::string FormatBoundArgsList(T arg1, BoundArgs... args) const
    {
        std::stringstream ss;
        ss << arg1 << ", " << FormatBoundArgsList(args...);
        return ss.str();
    }

    /**
     * Template for the end case of the recursions used to convert bound arguments into a string.
     *
     * @param arg1 The last argument in the list.
     *
     * @return The provided argument as a string
     */
    template <typename T>
    std::string FormatBoundArgsList(T arg1) const
    {
        std::stringstream ss;
        ss << arg1;
        return ss.str();
    }

    /**
     * Closes the connection to the database.
     */
    virtual void CloseDb();

    void DoDispose() override;
    /**
     * Indicates if the database connection has been established.
     *
     * @return True, if the database connection is open; otherwise, false.
     */
    virtual bool IsDbOpen() const;
    /**
     * Opens the database file stores the handler. This method
     * calls InitDb to ensure that the required tables and indexes are available.
     */
    virtual void OpenDb();
    /**
     * Used to report the return code of SQL queries.
     */
    TracedCallback<std::string, std::string, int> m_queryRc;

  private:
    /**
     * Ready the database schema. This method creates the required tables and indexes.
     * If the schema already exists, no change is made, allowing for reusing existing
     * database files and extending databases created with previous simulations.
     */
    void InitDb();

    /**
     * Initialize the maps with the prepared statements' strings
     */
    void InitStatements();

    /**
     * The database.
     */
    sqlite3* m_db;
    /**
     * The file path of the database.
     */
    std::string m_dbPath;
    /**
     * Map with the prepared statements' strings
     */
    std::map<StatementType, std::string> m_queryStmtsStrings;
    /**
     * Map with the table creation prepared statements' strings
     */
    std::map<CreateStatementType, std::string> m_createStmtsStrings;

    /**
     * Wrapper for the code needed to run the CREATE statements
     * It's only purpose is reduce the repeated code in the initialization
     * of the DB
     *
     * @param string The string with the SQL CREATE statement to run
     */
    void RunCreateStatement(std::string string);

}; // class OranDataRepositorySqlite

} // namespace ns3

#endif /* ORAN_DATA_REPOSITORY_SQLITE_H */
