/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include "DbTraceDataBase.h"
#include "DataBaseManager.h"
#include "DbSqlDefs.h"
#include "NpuInfoRepoMock.h"
#include "TraceDatabaseHelper.h"
#include "TrackInfoManager.h"
#include "../../../DatabaseTestCaseMockUtil.cpp"
using namespace Dic::Global::PROFILER::MockUtil;
using namespace Dic::Module::Timeline;
class DbTraceDatabaseTest : public ::testing::Test {
protected:
    class MockDatabase2 : public Dic::Module::FullDb::DbTraceDataBase {
    public:
        explicit MockDatabase2(std::recursive_mutex &sqlMutex) : DbTraceDataBase(sqlMutex) {}
        void SetDbPtr(sqlite3 *dbPtr)
        {
            isOpen = true;
            db = dbPtr;
            InitStringsCache();
        }
    };
    std::string pytorchSql = "CREATE TABLE PYTORCH_API (startNs TEXT, endNs TEXT, globalTid INTEGER, connectionId "
        "INTEGER, name INTEGER, sequenceNumber INTEGER, fwdThreadId INTEGER, inputDtypes INTEGER, "
        "inputShapes INTEGER, callchainId INTEGER, type INTEGER, depth integer);";
    std::string stringIdsSql = "CREATE TABLE STRING_IDS (id INTEGER PRIMARY KEY,value TEXT);";
    std::string computeSql =
        "CREATE TABLE COMPUTE_TASK_INFO (name INTEGER,globalTaskId INTEGER PRIMARY KEY,blockDim INTEGER,mixBlockDim "
        "INTEGER,taskType INTEGER,opType INTEGER,inputFormats INTEGER,inputDataTypes INTEGER,inputShapes "
        "INTEGER,outputFormats INTEGER,outputDataTypes INTEGER,outputShapes INTEGER,attrInfo INTEGER, waitNs INTEGER);";
    std::string taskSql = "CREATE TABLE TASK (startNs INTEGER,endNs INTEGER,deviceId INTEGER,connectionId "
        "INTEGER,globalTaskId INTEGER,globalPid INTEGER,taskType INTEGER,contextId INTEGER,streamId "
        "INTEGER,taskId INTEGER,modelId INTEGER, depth integer);";
    std::string schedulerTable = "CREATE table COMMUNICATION_SCHEDULE_TASK_INFO("
         "name INTEGER, globalTaskId INTEGER primary key, taskType INTEGER, opType INTEGER);";
    std::string comcaInfoSql =
        "CREATE TABLE COMMUNICATION_TASK_INFO (name INTEGER,globalTaskId INTEGER,taskType INTEGER,planeId "
        "INTEGER,groupName INTEGER,notifyId INTEGER,rdmaType INTEGER,srcRank INTEGER,dstRank INTEGER,transportType "
        "INTEGER,size INTEGER,dataType INTEGER,linkType INTEGER,opId INTEGER);";
    std::string comcaOpSql = "CREATE TABLE COMMUNICATION_OP (opName INTEGER,startNs INTEGER,endNs INTEGER,connectionId "
        "INTEGER,groupName INTEGER,opId INTEGER PRIMARY KEY,relay INTEGER,retry INTEGER,dataType "
        "INTEGER,algType INTEGER,count NUMERIC,opType INTEGER, waitNs INTEGER);";
    std::string cannSql = "CREATE TABLE CANN_API (startNs INTEGER,endNs INTEGER,type INTEGER,globalTid "
        "INTEGER,connectionId INTEGER PRIMARY KEY,name INTEGER, depth integer);";
    std::string connectionCatSql = "CREATE TABLE connectionCats(connectionId INT,cat);";
    std::string connectIds = "CREATE TABLE CONNECTION_IDS (id INTEGER, connectionId INTEGER);";
    std::string mstxSql = "CREATE TABLE MSTX_EVENTS (startNs INTEGER,endNs INTEGER,eventType INTEGER,rangeId "
        "INTEGER,category INTEGER,message INTEGER,globalTid INTEGER,endGlobalTid "
        "INTEGER,domainId INTEGER,connectionId INTEGER, depth integer);";
    std::string overlap = "CREATE TABLE OVERLAP_ANALYSIS (id INTEGER PRIMARY KEY AUTOINCREMENT, deviceId integer, "
        "startNs integer, endNs integer, type integer);";
    std::string npuInfoSql = "CREATE TABLE NPU_INFO(id INTEGER, name TEXT);";
    std::string accPmuSql = "CREATE TABLE ACC_PMU (accId INTEGER,readBwLevel INTEGER,writeBwLevel INTEGER,readOstLevel "
        "INTEGER,writeOstLevel INTEGER,timestampNs NUMERIC,deviceId INTEGER);";
    std::string socSql = "CREATE TABLE SOC_BANDWIDTH_LEVEL (l2BufferBwLevel INTEGER,mataBwLevel INTEGER,timestampNs "
        "NUMERIC,deviceId INTEGER);";
    std::string menSql =
        "CREATE TABLE NPU_MEM (type INTEGER,ddr NUMERIC,hbm NUMERIC,timestampNs INTEGER,deviceId INTEGER);";
    std::string hccsSql =
        "CREATE TABLE HCCS (type INTEGER,ddr NUMERIC,hbm NUMERIC,timestampNs INTEGER,deviceId INTEGER);";
    std::string pcieSql =
        "CREATE TABLE PCIE (type INTEGER,ddr NUMERIC,hbm NUMERIC,timestampNs INTEGER,deviceId INTEGER);";
    std::string aiCoreSql =
        "CREATE TABLE AICORE_FREQ (type INTEGER,ddr NUMERIC,hbm NUMERIC,timestampNs INTEGER,deviceId INTEGER);";

    std::string taskIncludingMSTXInsert1 =
        "INSERT INTO TASK (startNs, endNs, deviceId, connectionId, globalTaskId, "
        "globalPid, taskType, contextId, streamId, taskId, modelId, depth) "
        "VALUES (1742699319641107170, 1742699319641107190, 0, 4294967295, 7480, 1984976, 1, 4294967295, 2, 12658, "
        "4294967295, 0),"
        "(1729733883833924932, 1729733883833924952, 0, 4000000002, 82550, 511284, 221, 4294967295, 2, 40, 4294967295, "
        "0),"
        "(1729733883833924952, 1729733883833924992, 0, 4000000001, 82550, 511284, 221, 4294967295, 2, 40, 4294967295, "
        "0);";
    std::string mstxTableInsert1 =
        "INSERT INTO MSTX_EVENTS (startNs, endNs, eventType, rangeId, category, message, globalTid, endGlobalTid, "
        "domainId, connectionId, depth) VALUES "
        "(1729733883833924932, 1729733883833924952, 2, 4294967295, 4294967295, 447, "
        "4754301164515056, 4754301164515056, 239, 4000000001, 0),"
        "(1729733883833924932, 1729733883833924952, 2, 4294967295, 4294967295, 448, "
        "4754301164515056, 4754301164515056, 240, 4000000002, 0);";
    std::string stringIdsTableInsert1 =
        "INSERT INTO STRING_IDS(id, value) VALUES (239, 'compute'), (240, 'communication'), "
        "(447, 'start'), (448, 'hcom_allReduce')";
};
namespace Dic::Protocol {
using namespace Dic::Module::Timeline;
}

/**
 * RANK_DEVICE_MAP表和NPU_INFO都不存在的情况
 */
TEST_F(DbTraceDatabaseTest, TestQueryRankIdWithRankDeviceAndNpuInfoTableNotExist)
{
    Dic::Protocol::DataBaseManager::Instance().Clear();
    std::recursive_mutex testMutex;
    Dic::Module::FullDb::DbTraceDataBase database(testMutex);
    std::vector<std::string> rankIds = database.QueryRankId();
    const uint64_t expectSize = 1;
    EXPECT_EQ(rankIds.size(), expectSize);
    Dic::Protocol::DataBaseManager::Instance().Clear();
}

/**
 * RANK_DEVICE_MAP表存在的情况
 */
TEST_F(DbTraceDatabaseTest, TestQueryRankIdWithRankDeviceExist)
{
    class MockDatabase : public Dic::Module::FullDb::DbTraceDataBase {
    public:
        explicit MockDatabase(std::recursive_mutex &sqlMutex) : DbTraceDataBase(sqlMutex) {}
        void SetDbPtr(sqlite3 *dbPtr)
        {
            isOpen = true;
            db = dbPtr;
            return;
        }
    };
    Dic::Protocol::DataBaseManager::Instance().Clear();
    Dic::Protocol::DataBaseManager::Instance().SetFileType(Dic::Protocol::FileType::PYTORCH);
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    std::string sql = "CREATE TABLE RANK_DEVICE_MAP (rankId INTEGER, deviceId INTEGER);";
    DatabaseTestCaseMockUtil::OpenDBAndCreateTable(db, sql);
    std::string insertSql = "INSERT INTO RANK_DEVICE_MAP (rankId, deviceId) VALUES (999, 8), (276878, 7);";
    DatabaseTestCaseMockUtil::InsertData(db, insertSql);
    database.SetDbPtr(db);
    std::vector<std::string> rankIds = database.QueryRankId();
    const uint32_t expectSize = 2;
    const std::string firstRankId = "999";
    const std::string secondRankId = "276878";
    const uint32_t first = 0;
    const uint32_t second = 1;
    EXPECT_EQ(rankIds.size(), expectSize);
    EXPECT_EQ(rankIds[first], firstRankId);
    EXPECT_EQ(rankIds[second], secondRankId);
    rankIds = database.QueryRankId();
    EXPECT_EQ(rankIds.size(), expectSize);
    EXPECT_EQ(rankIds[first], firstRankId);
    EXPECT_EQ(rankIds[second], secondRankId);
    Dic::Protocol::DataBaseManager::Instance().Clear();
}

/**
 * RANK_DEVICE_MAP表不存在但NPU_INFO存在的情况
 */
TEST_F(DbTraceDatabaseTest, TestQueryRankIdWithNpuInfoExist)
{
    class MockDatabase : public Dic::Module::FullDb::DbTraceDataBase {
    public:
        explicit MockDatabase(std::recursive_mutex &sqlMutex) : DbTraceDataBase(sqlMutex) {}
        void SetDbPtr(sqlite3 *dbPtr)
        {
            isOpen = true;
            db = dbPtr;
            return;
        }
    };
    Dic::Protocol::DataBaseManager::Instance().Clear();
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    std::string sql = "CREATE TABLE NPU_INFO (id INTEGER, name TEXT);";
    Dic::Protocol::DataBaseManager::Instance().SetFileType(Dic::Protocol::FileType::MS_PROF);
    DatabaseTestCaseMockUtil::OpenDBAndCreateTable(db, sql);
    std::string insertSql = "INSERT INTO NPU_INFO (id, name) VALUES (3, 'h'), (4, 'kk');";
    DatabaseTestCaseMockUtil::InsertData(db, insertSql);
    database.SetDbPtr(db);
    std::vector<std::string> rankIds = database.QueryRankId();
    const uint32_t expectSize = 2;
    const std::string firstRankId = "3";
    const std::string secondRankId = "4";
    const uint32_t first = 0;
    const uint32_t second = 1;
    EXPECT_EQ(rankIds.size(), expectSize);
    EXPECT_EQ(rankIds[first], firstRankId);
    EXPECT_EQ(rankIds[second], secondRankId);
    Dic::Protocol::DataBaseManager::Instance().Clear();
}

/**
 * QueryRankIdAndDeviceMap函数RANK_DEVICE_MAP表和NPU_INFO都不存在的情况
 */
TEST_F(DbTraceDatabaseTest, TestQueryRankIdAndDeviceMapWithRankDeviceAndNpuInfoTableNotExist)
{
    Dic::Protocol::DataBaseManager::Instance().Clear();
    std::recursive_mutex testMutex;
    Dic::Module::FullDb::DbTraceDataBase database(testMutex);
    std::unordered_map<std::string, std::string> rankIds = database.QueryRankIdAndDeviceMap();
    EXPECT_EQ(rankIds.size(), 0);
    Dic::Protocol::DataBaseManager::Instance().Clear();
}

/**
 * QueryRankIdAndDeviceMap函数RANK_DEVICE_MAP表存在的情况
 */
TEST_F(DbTraceDatabaseTest, TestQueryRankIdAndDeviceMapWithRankDeviceExist)
{
    class MockDatabase : public Dic::Module::FullDb::DbTraceDataBase {
    public:
        explicit MockDatabase(std::recursive_mutex &sqlMutex) : DbTraceDataBase(sqlMutex) {}
        void SetDbPtr(sqlite3 *dbPtr)
        {
            isOpen = true;
            db = dbPtr;
            return;
        }
    };
    Dic::Protocol::DataBaseManager::Instance().Clear();
    Dic::Protocol::DataBaseManager::Instance().SetFileType(Dic::Protocol::FileType::PYTORCH);
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    std::string sql = "CREATE TABLE RANK_DEVICE_MAP (rankId INTEGER, deviceId INTEGER);";
    DatabaseTestCaseMockUtil::OpenDBAndCreateTable(db, sql);
    std::string insertSql = "INSERT INTO RANK_DEVICE_MAP (rankId, deviceId) VALUES (999, 8), (276878, 7);";
    DatabaseTestCaseMockUtil::InsertData(db, insertSql);
    database.SetDbPtr(db);
    std::unordered_map<std::string, std::string> rankIds = database.QueryRankIdAndDeviceMap();
    const uint32_t expectSize = 2;
    const std::string firstRankId = "999";
    const std::string secondRankId = "276878";
    const std::string first = "8";
    const std::string second = "7";
    EXPECT_EQ(rankIds.size(), expectSize);
    EXPECT_EQ(rankIds[firstRankId], first);
    EXPECT_EQ(rankIds[secondRankId], second);
    Dic::Protocol::DataBaseManager::Instance().Clear();
}

/**
 * QueryRankIdAndDeviceMap函数RANK_DEVICE_MAP表存在但列名错误
 */
TEST_F(DbTraceDatabaseTest, TestQueryRankIdAndDeviceMapWithRankDeviceWrongColumn)
{
    class MockDatabase : public Dic::Module::FullDb::DbTraceDataBase {
    public:
        explicit MockDatabase(std::recursive_mutex &sqlMutex) : DbTraceDataBase(sqlMutex) {}
        void SetDbPtr(sqlite3 *dbPtr)
        {
            isOpen = true;
            db = dbPtr;
            return;
        }
    };
    Dic::Protocol::DataBaseManager::Instance().Clear();
    Dic::Protocol::DataBaseManager::Instance().SetFileType(Dic::Protocol::FileType::PYTORCH);
    std::recursive_mutex testMutex;
    MockDatabase dataBase(testMutex);
    sqlite3 *db = nullptr;
    std::string sql = "CREATE TABLE RANK_DEVICE_MAP (rankId INTEGER, devicId INTEGER);";
    DatabaseTestCaseMockUtil::OpenDBAndCreateTable(db, sql);
    std::string insertSql = "INSERT INTO RANK_DEVICE_MAP (rankId, devicId) VALUES (999, 8), (276878, 7);";
    DatabaseTestCaseMockUtil::InsertData(db, insertSql);
    dataBase.SetDbPtr(db);
    std::unordered_map<std::string, std::string> rankIds = dataBase.QueryRankIdAndDeviceMap();
    EXPECT_EQ(rankIds.size(), 0);
    Dic::Protocol::DataBaseManager::Instance().Clear();
}

/**
 * GetDeviceId函数host存在
 */
TEST_F(DbTraceDatabaseTest, TestGetDeviceIdWithHostExist)
{
    class MockDatabase : public Dic::Module::FullDb::DbTraceDataBase {
    public:
        explicit MockDatabase(std::recursive_mutex &sqlMutex) : DbTraceDataBase(sqlMutex) {}
        void SetDbPtr(sqlite3 *dbPtr)
        {
            isOpen = true;
            db = dbPtr;
            return;
        }
    };
    Dic::Protocol::DataBaseManager::Instance().Clear();
    Dic::Protocol::DataBaseManager::Instance().SetFileType(Dic::Protocol::FileType::PYTORCH);
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    std::string sql = "CREATE TABLE RANK_DEVICE_MAP (rankId INTEGER, deviceId INTEGER);";
    DatabaseTestCaseMockUtil::OpenDBAndCreateTable(db, sql);
    std::string insertSql = "INSERT INTO RANK_DEVICE_MAP (rankId, deviceId) VALUES (999, 8), (276878, 7);";
    DatabaseTestCaseMockUtil::InsertData(db, insertSql);
    std::string hostSql = "CREATE TABLE HOST_INFO (hostUid TEXT,hostName TEXT);";
    DatabaseTestCaseMockUtil::CreateTable(db, hostSql);
    insertSql =
        "INSERT INTO \"main\".\"HOST_INFO\" (\"hostUid\", \"hostName\") VALUES ('3203900921565068809', 'ubuntu');";
    DatabaseTestCaseMockUtil::InsertData(db, insertSql);
    database.SetDbPtr(db);
    std::string fileId = "ubuntu3203900921565068809 999";
    std::string deviceId = database.GetDeviceId(fileId);
    const std::string expectDeviceId = "8";
    EXPECT_EQ(deviceId, expectDeviceId);
    Dic::Protocol::DataBaseManager::Instance().Clear();
}

/**
 * GetDeviceId函数host不存在
 */
TEST_F(DbTraceDatabaseTest, TestGetDeviceIdWithHostNotExist)
{
    class MockDatabase : public Dic::Module::FullDb::DbTraceDataBase {
    public:
        explicit MockDatabase(std::recursive_mutex &sqlMutex) : DbTraceDataBase(sqlMutex) {}
        void SetDbPtr(sqlite3 *dbPtr)
        {
            isOpen = true;
            db = dbPtr;
            return;
        }
    };
    Dic::Protocol::DataBaseManager::Instance().Clear();
    Dic::Protocol::DataBaseManager::Instance().SetFileType(Dic::Protocol::FileType::PYTORCH);
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    std::string sql = "CREATE TABLE RANK_DEVICE_MAP (rankId INTEGER, deviceId INTEGER);";
    DatabaseTestCaseMockUtil::OpenDBAndCreateTable(db, sql);
    std::string insertSql = "INSERT INTO RANK_DEVICE_MAP (rankId, deviceId) VALUES (999, 8), (276878, 7);";
    DatabaseTestCaseMockUtil::InsertData(db, insertSql);
    database.SetDbPtr(db);
    std::string fileId = "276878";
    std::string deviceId = database.GetDeviceId(fileId);
    const std::string expectDeviceId = "7";
    EXPECT_EQ(deviceId, expectDeviceId);
    Dic::Protocol::DataBaseManager::Instance().Clear();
}

TEST_F(DbTraceDatabaseTest, TestQueryAffinityOptimizerWhenDbNotOpen)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    const Dic::Protocol::KernelDetailsParams params;
    const std::string optimizers;
    std::vector<Dic::Protocol::ThreadTraces> data;
    const uint64_t minTimestamp = 0;
    bool result = database.QueryAffinityOptimizer(params, optimizers, data, minTimestamp);
    EXPECT_EQ(result, false);
}

TEST_F(DbTraceDatabaseTest, TestQueryAffinityOptimizerWhenTableExist)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDBAndCreateTable(db, pytorchSql);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    std::string pyData =
        "INSERT INTO \"main\".\"PYTORCH_API\" (\"startNs\", \"endNs\", \"globalTid\", \"connectionId\", \"name\", "
        "\"sequenceNumber\", \"fwdThreadId\", \"inputDtypes\", \"inputShapes\", \"callchainId\", \"type\", \"depth\") "
        "VALUES ('1723510445651039490', '1723510445651061410', 17738580008830245, 0, 268435456, NULL, NULL, NULL, "
        "NULL, NULL, 50002, 3);";
    std::string strData =
        "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (268435456, 'originOptimizer');";
    DatabaseTestCaseMockUtil::InsertData(db, pyData);
    DatabaseTestCaseMockUtil::InsertData(db, strData);
    database.SetDbPtr(db);
    Dic::Protocol::KernelDetailsParams params;
    params.orderBy = "name";
    params.order = "DESC";
    const std::string optimizers = "originOptimizer";
    std::vector<Dic::Protocol::ThreadTraces> data;
    const uint64_t minTimestamp = 0;
    bool result = database.QueryAffinityOptimizer(params, optimizers, data, minTimestamp);
    EXPECT_EQ(result, true);
}

TEST_F(DbTraceDatabaseTest, TestQueryAffinityOptimizerWhenTableNotExist)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDBAndCreateTable(db, pytorchSql);
    database.SetDbPtr(db);
    Dic::Protocol::KernelDetailsParams params;
    params.orderBy = "name";
    params.order = "DESC";
    const std::string optimizers = "originOptimizer";
    std::vector<Dic::Protocol::ThreadTraces> data;
    const uint64_t minTimestamp = 0;
    bool result = database.QueryAffinityOptimizer(params, optimizers, data, minTimestamp);
    EXPECT_EQ(result, false);
}

TEST_F(DbTraceDatabaseTest, TestQueryAICpuOpCanBeOptimizedWhenDbNotOpen)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    const Dic::Protocol::KernelDetailsParams params;
    const std::vector<std::string> replace;
    const std::map<std::string, Dic::Module::Timeline::AICpuCheckDataType> dataType;
    std::vector<Dic::Protocol::KernelBaseInfo> data;
    const uint64_t minTimestamp = 0;
    bool result = database.QueryAICpuOpCanBeOptimized(params, replace, dataType, data, minTimestamp);
    EXPECT_EQ(result, false);
}

TEST_F(DbTraceDatabaseTest, TestQueryAICpuOpCanBeOptimizedWhenDbOpen)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDBAndCreateTable(db, computeSql);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    DatabaseTestCaseMockUtil::CreateTable(db, taskSql);
    std::string comData = "INSERT INTO \"main\".\"COMPUTE_TASK_INFO\" (\"name\", \"globalTaskId\", \"blockDim\", "
        "\"mixBlockDim\", \"taskType\", \"opType\", \"inputFormats\", \"inputDataTypes\", "
        "\"inputShapes\", \"outputFormats\", \"outputDataTypes\", \"outputShapes\", \"attrInfo\", "
        "\"waitNs\") VALUES (3, 1, 8, 0, 1, 2, 6, 4, 8, 6, 5, 10, 5, 1240);";
    std::string strData = "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (1, 'AI_CPU');\n"
        "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (2, 'duration');\n"
        "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (3, 'MIX_AIV');\n"
        "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (4, 'NonZero');\n"
        "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (5, 'N/A');";
    std::string taskData = "INSERT INTO \"main\".\"TASK\" (\"startNs\", \"endNs\", \"deviceId\", \"connectionId\", "
        "\"globalTaskId\", \"globalPid\", \"taskType\", \"contextId\", \"streamId\", \"taskId\", "
        "\"modelId\", \"depth\") VALUES (1723510445634242160, 1723510445634253160, 0, 4294967295, "
        "1, 4130085, 293, 4294967295, 0, 39, 4294967295, 0);";
    DatabaseTestCaseMockUtil::InsertData(db, comData);
    DatabaseTestCaseMockUtil::InsertData(db, strData);
    DatabaseTestCaseMockUtil::InsertData(db, taskData);
    database.SetDbPtr(db);
    Dic::Protocol::KernelDetailsParams params;
    params.orderBy = "name";
    params.order = "DESC";
    std::vector<std::string> replace = { "duration" };
    std::map<std::string, Dic::Module::Timeline::AICpuCheckDataType> dataType;
    Dic::Module::Timeline::AICpuCheckDataType aiCpuCheckDataType;
    aiCpuCheckDataType.input.emplace_back("lll");
    aiCpuCheckDataType.output.emplace_back("mmm");
    dataType["other"] = aiCpuCheckDataType;
    std::vector<Dic::Protocol::KernelBaseInfo> data;
    const uint64_t minTimestamp = 0;
    bool result = database.QueryAICpuOpCanBeOptimized(params, replace, dataType, data, minTimestamp);
    EXPECT_EQ(result, true);
}

TEST_F(DbTraceDatabaseTest, TestQueryThreadSameOperatorsDetailsWhenSqlInject)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    Dic::Protocol::UnitThreadsOperatorsParams requestParams;
    Dic::Protocol::UnitThreadsOperatorsBody responseBody;
    requestParams.orderBy = "lll@#";
    const uint64_t minTimestamp = 0;
    const std::vector<uint64_t> traceId = {0};
    bool result = database.QueryThreadSameOperatorsDetails(requestParams, responseBody, minTimestamp, traceId);
    EXPECT_EQ(result, false);
}

TEST_F(DbTraceDatabaseTest, TestQueryThreadSameOperatorsDetailsWhenDbNotOpen)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    Dic::Protocol::UnitThreadsOperatorsParams requestParams;
    Dic::Protocol::UnitThreadsOperatorsBody responseBody;
    const uint64_t minTimestamp = 0;
    const std::vector<uint64_t> traceId = {0};
    bool result = database.QueryThreadSameOperatorsDetails(requestParams, responseBody, minTimestamp, traceId);
    EXPECT_EQ(result, false);
}

TEST_F(DbTraceDatabaseTest, TestQueryThreadSameOperatorsDetailsWhenDbOpenHardWare)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::OpenDBAndCreateTable(db, computeSql);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    DatabaseTestCaseMockUtil::CreateTable(db, taskSql);
    database.SetDbPtr(db);
    Dic::Protocol::UnitThreadsOperatorsParams requestParams;
    requestParams.processes.push_back(SimpleProcess {"17738580008830245", {"0"}});
    requestParams.metaTypeList = {"Ascend Hardware"};
    requestParams.orderBy = "duration";
    requestParams.order = "DESC";
    Dic::Protocol::UnitThreadsOperatorsBody responseBody;
    const uint64_t minTimestamp = 0;
    const std::vector<uint64_t> traceId = {0};
    bool result = database.QueryThreadSameOperatorsDetails(requestParams, responseBody, minTimestamp, traceId);
    EXPECT_EQ(result, false);
}

TEST_F(DbTraceDatabaseTest, TestQueryThreadSameOperatorsDetailsWhenDbOpenHardWareAndOverlap_NameInvalidForOverlap)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    DatabaseTestCaseMockUtil::CreateTable(db, computeSql);
    DatabaseTestCaseMockUtil::CreateTable(db, schedulerTable);
    DatabaseTestCaseMockUtil::CreateTable(db, taskSql);
    DatabaseTestCaseMockUtil::CreateTable(db, cannSql);
    DatabaseTestCaseMockUtil::CreateTable(db, comcaInfoSql);
    DatabaseTestCaseMockUtil::CreateTable(db, comcaOpSql);
    DatabaseTestCaseMockUtil::CreateTable(db, pytorchSql);
    DatabaseTestCaseMockUtil::CreateTable(db, mstxSql);
    DatabaseTestCaseMockUtil::CreateTable(db, overlap);
    std::string taskData =
        "INSERT INTO \"main\".\"TASK\" (\"startNs\", \"endNs\", \"deviceId\", \"connectionId\", \"globalTaskId\", "
        "\"globalPid\", \"taskType\", \"contextId\", \"streamId\", \"taskId\", \"modelId\", \"depth\") "
        "VALUES (1742699319641107170, 1742699319641107190, 0, 72907, 7480, 1984976, 1, 4294967295, 2, 12658, "
        "4294967295, 0);";
    std::string overlapData =
        "INSERT INTO \"main\".\"OVERLAP_ANALYSIS\" (\"id\", \"deviceId\", \"startNs\", \"endNs\", \"type\") VALUES "
        "(103984, 0, 1742699321190093818, 1742699321190208301, 2);";
    std::string strData = "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (1, 'EVENT_WAIT');";
    DatabaseTestCaseMockUtil::InsertData(db, taskData);
    DatabaseTestCaseMockUtil::InsertData(db, strData);
    database.SetDbPtr(db);
    Dic::Protocol::UnitThreadsOperatorsParams requestParams;
    requestParams.processes.push_back(SimpleProcess {"17738580008830245", {"2"}});
    requestParams.name = "EVENT_WAIT";
    requestParams.rankId = "0";
    requestParams.startTime = 1742699319641100000;
    requestParams.endTime = 1742699321190208302;
    requestParams.pageSize = 10;
    requestParams.orderBy = "duration";
    requestParams.order = "DESC";
    requestParams.metaTypeList = {"Ascend Hardware"};
    Dic::Protocol::UnitThreadsOperatorsBody responseBody;
    const uint64_t minTimestamp = 0;
    uint64_t trackId = TrackInfoManager::Instance().GetTrackId("", "17738580008830245", "2");
    const std::vector<uint64_t> traceIds = {trackId};
    bool result = database.QueryThreadSameOperatorsDetails(requestParams, responseBody, minTimestamp, traceIds);
    EXPECT_EQ(result, true);
    EXPECT_EQ(responseBody.sameOperatorsDetails.size(), 1);
    EXPECT_EQ(responseBody.sameOperatorsDetails[0].pid, "Ascend Hardware");
}

TEST_F(DbTraceDatabaseTest, TestQueryThreadSameOperatorsDetailsWhenHavingDeviceMSTXEvents)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    DatabaseTestCaseMockUtil::CreateTable(db, computeSql);
    DatabaseTestCaseMockUtil::CreateTable(db, schedulerTable);
    DatabaseTestCaseMockUtil::CreateTable(db, taskSql);
    DatabaseTestCaseMockUtil::CreateTable(db, cannSql);
    DatabaseTestCaseMockUtil::CreateTable(db, comcaInfoSql);
    DatabaseTestCaseMockUtil::CreateTable(db, comcaOpSql);
    DatabaseTestCaseMockUtil::CreateTable(db, pytorchSql);
    DatabaseTestCaseMockUtil::CreateTable(db, mstxSql);
    DatabaseTestCaseMockUtil::CreateTable(db, overlap);
    DatabaseTestCaseMockUtil::InsertData(db, taskIncludingMSTXInsert1);
    DatabaseTestCaseMockUtil::InsertData(db, mstxTableInsert1);
    DatabaseTestCaseMockUtil::InsertData(db, stringIdsTableInsert1);
    database.SetDbPtr(db);
    Dic::Protocol::UnitThreadsOperatorsParams requestParams;
    requestParams.processes.push_back(SimpleProcess {"Ascend Hardware", {"2", "2_239", "2_240"}});
    requestParams.name = "start";
    requestParams.rankId = "0";
    requestParams.startTime = 1720000000000000000; // 1720000000000000000
    requestParams.endTime = 1740000000000000000; // 1740000000000000000
    requestParams.pageSize = 10; // 10
    requestParams.orderBy = "duration";
    requestParams.order = "DESC";
    requestParams.metaTypeList = {"Ascend Hardware"};
    Dic::Protocol::UnitThreadsOperatorsBody responseBody;
    const uint64_t minTimestamp = 0;
    uint64_t trackId1 = TrackInfoManager::Instance().GetTrackId("0", "Ascend Hardware", "2");
    uint64_t trackId2 = TrackInfoManager::Instance().GetTrackId("0", "Ascend Hardware", "2_239");
    uint64_t trackId3 = TrackInfoManager::Instance().GetTrackId("0", "Ascend Hardware", "2_240");
    const std::vector<uint64_t> traceIds = {trackId1, trackId2, trackId3};
    bool result = database.QueryThreadSameOperatorsDetails(requestParams, responseBody, minTimestamp, traceIds);
    ASSERT_EQ(result, true);
    ASSERT_EQ(responseBody.sameOperatorsDetails.size(), 1);
    EXPECT_EQ(responseBody.sameOperatorsDetails[0].pid, "Ascend Hardware");
}

TEST_F(DbTraceDatabaseTest, TestQueryThreadSameOperatorsDetailsWhenCANN)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    DatabaseTestCaseMockUtil::CreateTable(db, computeSql);
    DatabaseTestCaseMockUtil::CreateTable(db, schedulerTable);
    DatabaseTestCaseMockUtil::CreateTable(db, taskSql);
    DatabaseTestCaseMockUtil::CreateTable(db, cannSql);
    DatabaseTestCaseMockUtil::CreateTable(db, comcaInfoSql);
    DatabaseTestCaseMockUtil::CreateTable(db, comcaOpSql);
    DatabaseTestCaseMockUtil::CreateTable(db, pytorchSql);
    DatabaseTestCaseMockUtil::CreateTable(db, mstxSql);
    DatabaseTestCaseMockUtil::CreateTable(db, overlap);
    database.SetDbPtr(db);
    Dic::Protocol::UnitThreadsOperatorsParams requestParams;
    requestParams.rankId = "";
    requestParams.processes.push_back(SimpleProcess {"17738580008830245", {"0"}});
    requestParams.metaTypeList = {"CANN_API"};
    requestParams.orderBy = "duration";
    requestParams.order = "DESC";
    Dic::Protocol::UnitThreadsOperatorsBody responseBody;
    const uint64_t minTimestamp = 0;
    std::vector<uint64_t> traceId;
    for (const auto& process : requestParams.processes) {
        for (const auto& tid : process.tidList) {
            traceId.emplace_back(TrackInfoManager::Instance().GetTrackId(requestParams.rankId, process.pid, tid));
        }
    }
    bool result = database.QueryThreadSameOperatorsDetails(requestParams, responseBody, minTimestamp, traceId);
    EXPECT_EQ(result, true);
}

TEST_F(DbTraceDatabaseTest, TestQueryThreadSameOperatorsDetailsWhenMstx)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    DatabaseTestCaseMockUtil::CreateTable(db, computeSql);
    DatabaseTestCaseMockUtil::CreateTable(db, schedulerTable);
    DatabaseTestCaseMockUtil::CreateTable(db, taskSql);
    DatabaseTestCaseMockUtil::CreateTable(db, cannSql);
    DatabaseTestCaseMockUtil::CreateTable(db, comcaInfoSql);
    DatabaseTestCaseMockUtil::CreateTable(db, comcaOpSql);
    DatabaseTestCaseMockUtil::CreateTable(db, pytorchSql);
    DatabaseTestCaseMockUtil::CreateTable(db, mstxSql);
    DatabaseTestCaseMockUtil::CreateTable(db, overlap);
    database.SetDbPtr(db);
    Dic::Protocol::UnitThreadsOperatorsParams requestParams;
    requestParams.rankId = "";
    requestParams.processes.push_back(SimpleProcess {"17738580008830245", {"0"}});
    requestParams.metaTypeList = {"MSTX_EVENTS"};
    requestParams.orderBy = "duration";
    requestParams.order = "DESC";
    Dic::Protocol::UnitThreadsOperatorsBody responseBody;
    const uint64_t minTimestamp = 0;
    std::vector<uint64_t> traceId;
    for (const auto& process : requestParams.processes) {
        for (const auto& tid : process.tidList) {
            traceId.emplace_back(TrackInfoManager::Instance().GetTrackId(requestParams.rankId, process.pid, tid));
        }
    }
    bool result = database.QueryThreadSameOperatorsDetails(requestParams, responseBody, minTimestamp, traceId);
    EXPECT_EQ(result, true);
}

TEST_F(DbTraceDatabaseTest, TestQueryThreadSameOperatorsDetailsWhenApi)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    DatabaseTestCaseMockUtil::CreateTable(db, computeSql);
    DatabaseTestCaseMockUtil::CreateTable(db, schedulerTable);
    DatabaseTestCaseMockUtil::CreateTable(db, taskSql);
    DatabaseTestCaseMockUtil::CreateTable(db, cannSql);
    DatabaseTestCaseMockUtil::CreateTable(db, comcaInfoSql);
    DatabaseTestCaseMockUtil::CreateTable(db, comcaOpSql);
    DatabaseTestCaseMockUtil::CreateTable(db, pytorchSql);
    DatabaseTestCaseMockUtil::CreateTable(db, mstxSql);
    DatabaseTestCaseMockUtil::CreateTable(db, overlap);
    std::string pyData =
        "INSERT INTO \"main\".\"PYTORCH_API\" (\"startNs\", \"endNs\", \"globalTid\", \"connectionId\", \"name\", "
        "\"sequenceNumber\", \"fwdThreadId\", \"inputDtypes\", \"inputShapes\", \"callchainId\", \"type\", \"depth\") "
        "VALUES ('1723510445651039490', '1723510445651061410', 17738580008830245, 0, 268435456, NULL, NULL, NULL, "
        "NULL, NULL, 50002, 3);";
    std::string strData = "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (268435456, 'FORMAT_ND');";
    DatabaseTestCaseMockUtil::InsertData(db, pyData);
    DatabaseTestCaseMockUtil::InsertData(db, strData);
    database.SetDbPtr(db);
    Dic::Protocol::UnitThreadsOperatorsParams requestParams;
    requestParams.rankId = "";
    requestParams.processes.push_back(SimpleProcess {"17738580008830245", {"0"}});
    requestParams.metaTypeList = {"PYTORCH_API"};
    requestParams.orderBy = "duration";
    requestParams.order = "DESC";
    const uint64_t min = 0;
    const uint64_t max = 1823510445651061410;
    requestParams.startTime = min;
    requestParams.endTime = max;
    requestParams.name = "FORMAT_ND";
    Dic::Protocol::UnitThreadsOperatorsBody responseBody;
    const uint64_t minTimestamp = 0;
    std::vector<uint64_t> traceId;
    for (const auto& process : requestParams.processes) {
        for (const auto& tid : process.tidList) {
            traceId.emplace_back(TrackInfoManager::Instance().GetTrackId(requestParams.rankId, process.pid, tid));
        }
    }
    bool result = database.QueryThreadSameOperatorsDetails(requestParams, responseBody, minTimestamp, traceId);
    EXPECT_EQ(result, true);
}

TEST_F(DbTraceDatabaseTest, TestQueryAclnnOpCountExceedThresholdWhenDbNotOpen)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    const Dic::Protocol::KernelDetailsParams params;
    const uint64_t threshold = 0;
    std::vector<Dic::Protocol::KernelBaseInfo> data;
    const uint64_t minTimestamp = 0;
    bool result = database.QueryAclnnOpCountExceedThreshold(params, threshold, data, minTimestamp);
    EXPECT_EQ(result, false);
}

TEST_F(DbTraceDatabaseTest, TestQueryAclnnOpCountExceedThresholdWhenDbOpen)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDBAndCreateTable(db, computeSql);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    DatabaseTestCaseMockUtil::CreateTable(db, taskSql);
    std::string comData = "INSERT INTO \"main\".\"COMPUTE_TASK_INFO\" (\"name\", \"globalTaskId\", \"blockDim\", "
        "\"mixBlockDim\", \"taskType\", \"opType\", \"inputFormats\", \"inputDataTypes\", "
        "\"inputShapes\", \"outputFormats\", \"outputDataTypes\", \"outputShapes\", \"attrInfo\", "
        "\"waitNs\") VALUES (1, 1, 8, 0, 1, 2, 6, 4, 8, 6, 5, 10, 5, 1240);";
    std::string strData = "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (1, 'aclnnuuuu8');\n"
        "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (2, 'duration');\n"
        "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (3, 'MIX_AIV');\n"
        "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (4, 'NonZero');\n"
        "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (5, 'N/A');";
    std::string taskData = "INSERT INTO \"main\".\"TASK\" (\"startNs\", \"endNs\", \"deviceId\", \"connectionId\", "
        "\"globalTaskId\", \"globalPid\", \"taskType\", \"contextId\", \"streamId\", \"taskId\", "
        "\"modelId\", \"depth\") VALUES (1723510445634242160, 1723510445634253160, 0, 4294967295, "
        "1, 4130085, 293, 4294967295, 0, 39, 4294967295, 0);";
    DatabaseTestCaseMockUtil::InsertData(db, comData);
    DatabaseTestCaseMockUtil::InsertData(db, strData);
    DatabaseTestCaseMockUtil::InsertData(db, taskData);
    database.SetDbPtr(db);
    Dic::Protocol::KernelDetailsParams params;
    params.orderBy = "name";
    params.order = "DESC";
    const uint64_t threshold = 0;
    std::vector<Dic::Protocol::KernelBaseInfo> data;
    const uint64_t minTimestamp = 0;
    bool result = database.QueryAclnnOpCountExceedThreshold(params, threshold, data, minTimestamp);
    EXPECT_EQ(result, true);
}

TEST_F(DbTraceDatabaseTest, TestQueryFuseableOpDataWhenDbOpen)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    Dic::Protocol::KernelDetailsParams params;
    params.orderBy = "name";
    params.order = "DESC";
    Dic::Module::Timeline::FuseableOpRule rule;
    rule.opList.emplace_back("Transpose");
    std::vector<Dic::Protocol::FlowLocation> data;
    const uint64_t minTimestamp = 0;
    bool result0 = database.QueryFuseableOpData(params, rule, data, minTimestamp);
    EXPECT_EQ(result0, false);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDBAndCreateTable(db, computeSql);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    DatabaseTestCaseMockUtil::CreateTable(db, taskSql);
    std::string comData = "INSERT INTO \"main\".\"COMPUTE_TASK_INFO\" (\"name\", \"globalTaskId\", \"blockDim\", "
        "\"mixBlockDim\", \"taskType\", \"opType\", \"inputFormats\", \"inputDataTypes\", "
        "\"inputShapes\", \"outputFormats\", \"outputDataTypes\", \"outputShapes\", \"attrInfo\", "
        "\"waitNs\") VALUES (1, 1, 8, 0, 1, 2, 6, 4, 8, 6, 5, 10, 5, 1240);";
    std::string strData = "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (1, 'Transpose');\n"
        "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (2, 'Transpose');\n"
        "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (3, 'MIX_AIV');\n"
        "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (4, 'NonZero');\n"
        "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (5, 'N/A');";
    std::string taskData = "INSERT INTO \"main\".\"TASK\" (\"startNs\", \"endNs\", \"deviceId\", \"connectionId\", "
        "\"globalTaskId\", \"globalPid\", \"taskType\", \"contextId\", \"streamId\", \"taskId\", "
        "\"modelId\", \"depth\") VALUES (1723510445634242160, 1723510445634253160, 0, 4294967295, "
        "1, 4130085, 293, 4294967295, 0, 39, 4294967295, 0);";
    DatabaseTestCaseMockUtil::InsertData(db, comData);
    DatabaseTestCaseMockUtil::InsertData(db, strData);
    DatabaseTestCaseMockUtil::InsertData(db, taskData);
    database.SetDbPtr(db);
    bool result = database.QueryFuseableOpData(params, rule, data, minTimestamp);
    EXPECT_EQ(result, true);
    rule.opList.emplace_back("Transpose2");
    bool result2 = database.QueryFuseableOpData(params, rule, data, minTimestamp);
    EXPECT_EQ(result, true);
}

TEST_F(DbTraceDatabaseTest, TestQueryAffinityAPIDataWhenDbOpen)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    DatabaseTestCaseMockUtil::CreateTable(db, pytorchSql);
    std::string pyData =
        "INSERT INTO \"main\".\"PYTORCH_API\" (\"startNs\", \"endNs\", \"globalTid\", \"connectionId\", \"name\", "
        "\"sequenceNumber\", \"fwdThreadId\", \"inputDtypes\", \"inputShapes\", \"callchainId\", \"type\", \"depth\") "
        "VALUES ('1723510445651039490', '1723510445651061410', 17738580008830245, 0, 268435456, NULL, NULL, NULL, "
        "NULL, NULL, 50002, 3);";
    std::string strData = "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (268435456, 'npu::kkk');";
    DatabaseTestCaseMockUtil::InsertData(db, pyData);
    DatabaseTestCaseMockUtil::InsertData(db, strData);
    database.SetDbPtr(db);
    const Dic::Protocol::KernelDetailsParams params;
    const std::set<std::string> pattern;
    const uint64_t minTimestamp = 0;
    std::map<uint64_t, std::vector<Dic::Protocol::FlowLocation>> data;
    std::map<uint64_t, std::vector<uint32_t>> indexes;
    bool result = database.QueryAffinityAPIData(params, pattern, minTimestamp, data, indexes);
    EXPECT_EQ(result, true);
}

TEST_F(DbTraceDatabaseTest, TestQueryAffinityAPIDataWhenDbNotOpen)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    const Dic::Protocol::KernelDetailsParams params;
    const std::set<std::string> pattern;
    const uint64_t minTimestamp = 0;
    std::map<uint64_t, std::vector<Dic::Protocol::FlowLocation>> data;
    std::map<uint64_t, std::vector<uint32_t>> indexes;
    bool result = database.QueryAffinityAPIData(params, pattern, minTimestamp, data, indexes);
    EXPECT_EQ(result, false);
}

TEST_F(DbTraceDatabaseTest, TestGetCounterUnitsAndDataTypesWhenACCPMU)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, accPmuSql);
    DatabaseTestCaseMockUtil::CreateTable(db, socSql);
    DatabaseTestCaseMockUtil::CreateTable(db, menSql);
    DatabaseTestCaseMockUtil::CreateTable(db, hccsSql);
    DatabaseTestCaseMockUtil::CreateTable(db, pcieSql);
    DatabaseTestCaseMockUtil::CreateTable(db, aiCoreSql);
    database.SetDbPtr(db);
    const std::string fileId = "ll";
    std::vector<std::unique_ptr<Dic::Protocol::UnitTrack>> metaData;
    bool result = database.QueryUnitsMetadata(fileId, metaData);
    EXPECT_EQ(result, false);
}

TEST_F(DbTraceDatabaseTest, TestQueryUnitFlowsWhenConnectionIdIsNotEmptyThenReturnFalse)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    for (const auto &item : Dic::FULL_DB_TABLE_MAP) {
        DatabaseTestCaseMockUtil::CreateTable(db, item.second);
    }
    database.SetDbPtr(db);
    const std::string fileId = "ll";
    const Dic::Protocol::UnitFlowsParams requestParams;
    Dic::Protocol::UnitFlowsBody responseBody;
    bool result = database.QueryUnitFlows(requestParams, responseBody, 0, 0);
    EXPECT_EQ(result, false);
}

TEST_F(DbTraceDatabaseTest, TestQueryUnitFlowsWhenConnectionIdIsOneThenReturnFalse)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, cannSql);
    DatabaseTestCaseMockUtil::CreateTable(db, mstxSql);
    DatabaseTestCaseMockUtil::CreateTable(db, pytorchSql);
    DatabaseTestCaseMockUtil::CreateTable(db, computeSql);
    DatabaseTestCaseMockUtil::CreateTable(db, taskSql);
    DatabaseTestCaseMockUtil::CreateTable(db, comcaOpSql);
    DatabaseTestCaseMockUtil::CreateTable(db, connectIds);
    DatabaseTestCaseMockUtil::CreateTable(db, connectionCatSql);
    const std::string connectIdCatData =
        "INSERT INTO \"main\".\"connectionCats\" (\"connectionId\", \"cat\") VALUES (476320, 'HostToDevice');";
    const std::string cannApiData =
        "INSERT INTO \"main\".\"CANN_API\" (\"startNs\", \"endNs\", \"type\", \"globalTid\", \"connectionId\", "
        "\"name\", \"depth\") VALUES (1734925661693760506, 1734925661693790778, 10000, 87471303975183, 476320, 7052, "
        "0);";
    DatabaseTestCaseMockUtil::InsertData(db, connectIdCatData);
    DatabaseTestCaseMockUtil::InsertData(db, cannApiData);
    database.SetDbPtr(db);
    const std::string fileId = "ll";
    Dic::Protocol::UnitFlowsParams requestParams;
    requestParams.id = "476320";
    requestParams.metaType = "CANN_API";
    Dic::Protocol::UnitFlowsBody responseBody;
    bool result = database.QueryUnitFlows(requestParams, responseBody, 0, 0);
    EXPECT_EQ(result, false);
}

TEST_F(DbTraceDatabaseTest, TestQueryUnitFlowsWhenConnectionIdIsTwoThenReturnTrue)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, cannSql);
    DatabaseTestCaseMockUtil::CreateTable(db, mstxSql);
    DatabaseTestCaseMockUtil::CreateTable(db, pytorchSql);
    DatabaseTestCaseMockUtil::CreateTable(db, computeSql);
    DatabaseTestCaseMockUtil::CreateTable(db, taskSql);
    DatabaseTestCaseMockUtil::CreateTable(db, comcaOpSql);
    DatabaseTestCaseMockUtil::CreateTable(db, connectIds);
    DatabaseTestCaseMockUtil::CreateTable(db, connectionCatSql);
    const std::string connectIdCatData =
        "INSERT INTO \"main\".\"connectionCats\" (\"connectionId\", \"cat\") VALUES (476320, 'HostToDevice');";
    const std::string cannApiData =
        "INSERT INTO \"main\".\"CANN_API\" (\"startNs\", \"endNs\", \"type\", \"globalTid\", \"connectionId\", "
        "\"name\", \"depth\") VALUES (1734925661693760506, 1734925661693790778, 10000, 87471303975183, 476320, 7052, "
        "0);";
    const std::string taskData =
        "INSERT INTO \"main\".\"TASK\" (\"startNs\", \"endNs\", \"deviceId\", \"connectionId\", \"globalTaskId\", "
        "\"globalPid\", \"taskType\", \"contextId\", \"streamId\", \"taskId\", \"modelId\", \"depth\") VALUES "
        "(1734925661780577867, 1734925661780577887, 15, 476320, 183022, 20366, 7166, 4294967295, 0, 39, "
        "4294967295, 0);";
    const std::string computeData =
        "INSERT INTO \"main\".\"COMPUTE_TASK_INFO\" (\"name\", \"globalTaskId\", \"blockDim\", \"mixBlockDim\", "
        "\"taskType\", \"opType\", \"inputFormats\", \"inputDataTypes\", \"inputShapes\", \"outputFormats\", "
        "\"outputDataTypes\", \"outputShapes\", \"attrInfo\", \"waitNs\") VALUES (0, 183022, 48, 0, 1, 2, 4, 5, 6, 4, "
        "5, 6, "
        "3, 0);";
    DatabaseTestCaseMockUtil::InsertData(db, connectIdCatData);
    DatabaseTestCaseMockUtil::InsertData(db, cannApiData);
    DatabaseTestCaseMockUtil::InsertData(db, taskData);
    DatabaseTestCaseMockUtil::InsertData(db, computeData);
    database.SetDbPtr(db);
    const std::string fileId = "ll";
    Dic::Protocol::UnitFlowsParams requestParams;
    requestParams.id = "476320";
    requestParams.metaType = "CANN_API";
    requestParams.rankId = "15";
    Dic::Protocol::UnitFlowsBody responseBody;
    bool result = database.QueryUnitFlows(requestParams, responseBody, 0, 0);
    EXPECT_EQ(result, true);
    EXPECT_EQ(responseBody.unitAllFlows.front().flows.front().from.rankId, "15");
    EXPECT_EQ(responseBody.unitAllFlows.front().flows.front().to.rankId, "15");
}

TEST_F(DbTraceDatabaseTest, TestQueryUnitFlowsWhenDeviceUniqueThenReturnTrue)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, cannSql);
    DatabaseTestCaseMockUtil::CreateTable(db, mstxSql);
    DatabaseTestCaseMockUtil::CreateTable(db, pytorchSql);
    DatabaseTestCaseMockUtil::CreateTable(db, computeSql);
    DatabaseTestCaseMockUtil::CreateTable(db, taskSql);
    DatabaseTestCaseMockUtil::CreateTable(db, comcaOpSql);
    DatabaseTestCaseMockUtil::CreateTable(db, connectIds);
    DatabaseTestCaseMockUtil::CreateTable(db, connectionCatSql);
    const std::string connectIdCatData =
        "INSERT INTO \"main\".\"connectionCats\" (\"connectionId\", \"cat\") VALUES (476320, 'HostToDevice');";
    const std::string cannApiData =
        "INSERT INTO \"main\".\"CANN_API\" (\"startNs\", \"endNs\", \"type\", \"globalTid\", \"connectionId\", "
        "\"name\", \"depth\") VALUES (1734925661693760506, 1734925661693790778, 10000, 87471303975183, 476320, 7052, "
        "0);";
    const std::string taskData =
        "INSERT INTO \"main\".\"TASK\" (\"startNs\", \"endNs\", \"deviceId\", \"connectionId\", \"globalTaskId\", "
        "\"globalPid\", \"taskType\", \"contextId\", \"streamId\", \"taskId\", \"modelId\", \"depth\") VALUES "
        "(1734925661780577867, 1734925661780577887, 15, 476320, 183022, 20366, 7166, 4294967295, 0, 39, "
        "4294967295, 0);";
    const std::string computeData =
        "INSERT INTO \"main\".\"COMPUTE_TASK_INFO\" (\"name\", \"globalTaskId\", \"blockDim\", \"mixBlockDim\", "
        "\"taskType\", \"opType\", \"inputFormats\", \"inputDataTypes\", \"inputShapes\", \"outputFormats\", "
        "\"outputDataTypes\", \"outputShapes\", \"attrInfo\", \"waitNs\") VALUES (0, 183022, 48, 0, 1, 2, 4, 5, 6, 4, "
        "5, 6, "
        "3, 0);";
    DatabaseTestCaseMockUtil::InsertData(db, connectIdCatData);
    DatabaseTestCaseMockUtil::InsertData(db, cannApiData);
    DatabaseTestCaseMockUtil::InsertData(db, taskData);
    DatabaseTestCaseMockUtil::InsertData(db, computeData);
    database.SetDbPtr(db);
    const std::string fileId = "ll";
    Dic::Protocol::UnitFlowsParams requestParams;
    requestParams.id = "476320";
    requestParams.metaType = "CANN_API";
    requestParams.rankId = "15";
    Dic::Protocol::UnitFlowsBody responseBody;
    MockNpuInfoRepoFunc();
    bool result = database.QueryUnitFlows(requestParams, responseBody, 0, 0);
    EXPECT_EQ(result, true);
    EXPECT_EQ(responseBody.unitAllFlows.front().flows.front().from.rankId, "15");
    EXPECT_EQ(responseBody.unitAllFlows.front().flows.front().to.rankId, "15");
    RestoreRepoFunc();
}

TEST_F(DbTraceDatabaseTest, TestQueryUnitFlowsWhenRankIdAndDeviceIdNotSame)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, cannSql);
    DatabaseTestCaseMockUtil::CreateTable(db, mstxSql);
    DatabaseTestCaseMockUtil::CreateTable(db, pytorchSql);
    DatabaseTestCaseMockUtil::CreateTable(db, computeSql);
    DatabaseTestCaseMockUtil::CreateTable(db, taskSql);
    DatabaseTestCaseMockUtil::CreateTable(db, comcaOpSql);
    DatabaseTestCaseMockUtil::CreateTable(db, connectIds);
    DatabaseTestCaseMockUtil::CreateTable(db, connectionCatSql);
    const std::string connectIdCatData =
        "INSERT INTO \"main\".\"connectionCats\" (\"connectionId\", \"cat\") VALUES (476320, 'HostToDevice');";
    const std::string cannApiData =
        "INSERT INTO \"main\".\"CANN_API\" (\"startNs\", \"endNs\", \"type\", \"globalTid\", \"connectionId\", "
        "\"name\", \"depth\") VALUES (1734925661693760506, 1734925661693790778, 10000, 87471303975183, 476320, 7052, "
        "0);";
    const std::string taskData =
        "INSERT INTO \"main\".\"TASK\" (\"startNs\", \"endNs\", \"deviceId\", \"connectionId\", \"globalTaskId\", "
        "\"globalPid\", \"taskType\", \"contextId\", \"streamId\", \"taskId\", \"modelId\", \"depth\") VALUES "
        "(1734925661780577867, 1734925661780577887, 15, 476320, 183022, 20366, 7166, 4294967295, 0, 39, "
        "4294967295, 0);";
    const std::string computeData =
        "INSERT INTO \"main\".\"COMPUTE_TASK_INFO\" (\"name\", \"globalTaskId\", \"blockDim\", \"mixBlockDim\", "
        "\"taskType\", \"opType\", \"inputFormats\", \"inputDataTypes\", \"inputShapes\", \"outputFormats\", "
        "\"outputDataTypes\", \"outputShapes\", \"attrInfo\", \"waitNs\") VALUES (0, 183022, 48, 0, 1, 2, 4, 5, 6, 4, "
        "5, 6, "
        "3, 0);";
    DatabaseTestCaseMockUtil::InsertData(db, connectIdCatData);
    DatabaseTestCaseMockUtil::InsertData(db, cannApiData);
    DatabaseTestCaseMockUtil::InsertData(db, taskData);
    DatabaseTestCaseMockUtil::InsertData(db, computeData);
    std::string sql = "CREATE TABLE RANK_DEVICE_MAP (rankId INTEGER, deviceId INTEGER);";
    DatabaseTestCaseMockUtil::CreateTable(db, sql);
    std::string insertSql = "INSERT INTO RANK_DEVICE_MAP (rankId, deviceId) VALUES (999, 15), (276878, 7);";
    DatabaseTestCaseMockUtil::InsertData(db, insertSql);
    database.SetDbPtr(db);
    const std::string fileId = "ll";
    Dic::Protocol::UnitFlowsParams requestParams;
    requestParams.id = "476320";
    requestParams.metaType = "CANN_API";
    requestParams.rankId = "999";
    Dic::Protocol::UnitFlowsBody responseBody;
    bool result = database.QueryUnitFlows(requestParams, responseBody, 0, 0);
    EXPECT_EQ(result, true);
    EXPECT_EQ(responseBody.unitAllFlows.front().flows.front().from.rankId, "999");
    EXPECT_EQ(responseBody.unitAllFlows.front().flows.front().to.rankId, "999");
}

TEST_F(DbTraceDatabaseTest, TestQueryUnitFlowsFromPyTorchToCANNToAscendHardware)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, connectIds);
    DatabaseTestCaseMockUtil::CreateTable(db, cannSql);
    DatabaseTestCaseMockUtil::CreateTable(db, pytorchSql);
    DatabaseTestCaseMockUtil::CreateTable(db, taskSql);
    DatabaseTestCaseMockUtil::CreateTable(db, npuInfoSql);
    DatabaseTestCaseMockUtil::CreateTable(db, mstxSql);
    DatabaseTestCaseMockUtil::CreateTable(db, comcaOpSql);
    const std::string connectIdsData =
        "INSERT INTO CONNECTION_IDS (id, connectionId) VALUES (1, 19);";
    const std::string cannApiData =
        "INSERT INTO CANN_API (startNs, endNs, type, globalTid, connectionId, "
        "name, depth) VALUES (50, 70, 10000, 87471303975183, 19, 7052, 0);";
    std::string pytorchData =
        "INSERT INTO PYTORCH_API (startNs, endNs, globalTid, connectionId, name, "
        "sequenceNumber, fwdThreadId, inputDtypes, inputShapes, callchainId, type, depth) "
        "VALUES (20, 40, 17738580008830245, 1, 268435456, NULL, NULL, NULL, NULL, NULL, 50002, 3);";
    const std::string taskData =
        "INSERT INTO TASK (startNs, endNs, deviceId, connectionId, globalTaskId, "
        "globalPid, taskType, contextId, streamId, taskId, modelId, depth) VALUES "
        "(80, 100, 1, 19, 183022, 20366, 7166, 4294967295, 0, 39, 4294967295, 0);";
    const std::string npuInfoData = "INSERT INTO NPU_INFO (id, name) VALUES (1, 'abc')";
    DatabaseTestCaseMockUtil::InsertData(db, connectIdsData);
    DatabaseTestCaseMockUtil::InsertData(db, cannApiData);
    DatabaseTestCaseMockUtil::InsertData(db, taskData);
    DatabaseTestCaseMockUtil::InsertData(db, pytorchData);
    DatabaseTestCaseMockUtil::InsertData(db, npuInfoData);
    std::string sql = "CREATE TABLE RANK_DEVICE_MAP (rankId INTEGER, deviceId INTEGER);";
    DatabaseTestCaseMockUtil::CreateTable(db, sql);
    std::string insertSql = "INSERT INTO RANK_DEVICE_MAP (rankId, deviceId) VALUES (1, 1);";
    DatabaseTestCaseMockUtil::InsertData(db, insertSql);
    database.SetDbPtr(db);

    Dic::Protocol::UnitFlowsParams requestParams;
    requestParams.id = "19";
    requestParams.metaType = "CANN_API";
    requestParams.rankId = "1";

    Dic::Protocol::UnitFlowsBody responseBody;
    bool result = database.QueryUnitFlows(requestParams, responseBody, 0, 0);
    ASSERT_EQ(result, true);
    ASSERT_EQ(responseBody.unitAllFlows.size(), 2); // 2
    EXPECT_EQ(responseBody.unitAllFlows[0].flows[0].from.timestamp, 50); // 50
    EXPECT_EQ(responseBody.unitAllFlows[0].flows[0].to.timestamp, 80); // 80
    EXPECT_EQ(responseBody.unitAllFlows[1].flows[0].from.timestamp, 20); // 20
    EXPECT_EQ(responseBody.unitAllFlows[1].flows[0].to.timestamp, 50); // 50
}

TEST_F(DbTraceDatabaseTest, GetLockRangeSqlWhenPython)
{
    std::vector<Dic::Module::Timeline::TrackQuery> trackQueryVec;
    Dic::Module::Timeline::TrackQuery item;
    Dic::Module::Timeline::SearchAllSliceParams params;
    item.metaType = PROCESS_TYPE_ES.at(PROCESS_TYPE::API);
    trackQueryVec.emplace_back(item);
    params.order = "descend";
    params.isMatchCase = true;
    params.isMatchExact = true;
    std::string sql = Dic::Module::Timeline::TraceDatabaseHelper::GetLockRangeSql(params, trackQueryVec);
    EXPECT_EQ(sql,
        "with ids as (select id, value from STRING_IDS where value like ?)  SELECT api.ROWID as id, 'pytorch' as tid, "
        "api.globalTid as pid, api.startNs as timestamp, api.endNs as endTime, api.depth, '' as deviceId, ids.value as "
        "value from PYTORCH_API  api join ids on ids.id = api.name WHERE api.globalTid = ? AND api.startNs >= ? AND "
        "api.endNs <= ?  ORDER BY timestamp DESC  LIMIT ? OFFSET ?");
}

TEST_F(DbTraceDatabaseTest, GetLockRangeSqlWhenCann)
{
    std::vector<Dic::Module::Timeline::TrackQuery> trackQueryVec;
    Dic::Module::Timeline::TrackQuery item;
    Dic::Module::Timeline::SearchAllSliceParams params;
    item.metaType = PROCESS_TYPE_ES.at(PROCESS_TYPE::CANN_API);
    trackQueryVec.emplace_back(item);
    params.order = "asc";
    params.isMatchCase = true;
    params.isMatchExact = false;
    std::string sql = Dic::Module::Timeline::TraceDatabaseHelper::GetLockRangeSql(params, trackQueryVec);
    EXPECT_EQ(sql,
        "with ids as (select id, value from STRING_IDS where value like '%'||?||'%')  SELECT cann.connectionId as id, "
        "cann.globalTid as pid, cann.type as tid, cann.startNs as timestamp, cann.endNs as endTime, cann.depth, '' as "
        "deviceId, ids.value from CANN_API  cann join ids on ids.id = cann.name WHERE globalTid = ? AND type = ? AND "
        "startNs >= ? AND endNs <= ?  ORDER BY timestamp DESC  LIMIT ? OFFSET ?");
}

TEST_F(DbTraceDatabaseTest, GetLockRangeSqlWhenMstx)
{
    std::vector<Dic::Module::Timeline::TrackQuery> trackQueryVec;
    Dic::Module::Timeline::TrackQuery item;
    Dic::Module::Timeline::SearchAllSliceParams params;
    item.metaType = PROCESS_TYPE_ES.at(PROCESS_TYPE::MS_TX);
    trackQueryVec.emplace_back(item);
    params.order = "asc";
    params.isMatchCase = false;
    params.isMatchExact = false;
    std::string sql = Dic::Module::Timeline::TraceDatabaseHelper::GetLockRangeSql(params, trackQueryVec);
    EXPECT_EQ(sql,
        "with ids as (select id, value from STRING_IDS where lower(value) like lower('%'||?||'%'))  SELECT mstx.ROWID "
        "as id, mstx.globalTid as pid, mstx.domainId as tid, mstx.startNs as timestamp, mstx.endNs as endTime, mstx.depth, '' "
        "as deviceId, ids.value from MSTX_EVENTS  mstx join ids on ids.id = mstx.message WHERE globalTid = ? AND "
        "startNs >= ? AND endNs <= ?  ORDER BY timestamp DESC  LIMIT ? OFFSET ?");
}

TEST_F(DbTraceDatabaseTest, GetLockRangeSqlWhenHardWare)
{
    std::vector<Dic::Module::Timeline::TrackQuery> trackQueryVec;
    Dic::Module::Timeline::TrackQuery item;
    Dic::Module::Timeline::SearchAllSliceParams params;
    item.metaType = PROCESS_TYPE_ES.at(PROCESS_TYPE::ASCEND_HARDWARE);
    trackQueryVec.emplace_back(item);
    params.order = "asc";
    params.isMatchCase = false;
    params.isMatchExact = false;
    std::string sql = Dic::Module::Timeline::TraceDatabaseHelper::GetLockRangeSql(params, trackQueryVec);
    EXPECT_EQ(sql,
        "with ids as (select id, value from STRING_IDS where lower(value) like lower('%'||?||'%')) SELECT hadware.id "
        "as id, hadware.pid as pid, hadware.tid as tid, hadware.timestamp as timestamp, hadware.endTime as endTime, "
        "hadware.depth as depth, hadware.deviceId as deviceId, ids.value  FROM (SELECT coalesce(c.name, m.message, "
        "s.name, main.taskType) as name, main.ROWID AS id, 'Ascend Hardware' as pid, main.streamId as tid,main.startNs "
        "as timestamp, main.endNs as endTime, main.depth as depth, main.deviceId as deviceId FROM TASK main left join "
        "COMPUTE_TASK_INFO c on c.globalTaskId = main.globalTaskId left join MSTX_EVENTS m on (m.connectionId = "
        "main.connectionId and  m.connectionId != 4294967295 ) left join COMMUNICATION_SCHEDULE_TASK_INFO s on "
        "main.globalTaskId = s.globalTaskId WHERE main.deviceId = ? AND main.streamId = ? AND main.startNs >= ? AND "
        "main.endNs <= ?) hadware  join ids on ids.id = hadware.name  ORDER BY timestamp DESC  LIMIT ? OFFSET ?");
}

TEST_F(DbTraceDatabaseTest, GetLockRangeSqlWhenGroup)
{
    std::vector<Dic::Module::Timeline::TrackQuery> trackQueryVec;
    Dic::Module::Timeline::TrackQuery item;
    Dic::Module::Timeline::SearchAllSliceParams params;
    item.metaType = PROCESS_TYPE_ES.at(PROCESS_TYPE::HCCL);
    trackQueryVec.emplace_back(item);
    item.threadId = "999group";
    params.order = "asc";
    params.isMatchCase = false;
    params.isMatchExact = false;
    std::string sql = Dic::Module::Timeline::TraceDatabaseHelper::GetLockRangeSql(params, trackQueryVec);
    EXPECT_EQ(sql,
        "with ids as (select id, value from STRING_IDS where lower(value) like lower('%'||?||'%')) SELECT main.ROWID "
        "as id, 'HCCL' as pid, ci.groupName||'_'||ci.planeId as tid, main.startNs as timestamp, main.endNs as endTime, "
        "main.depth, main.deviceId as deviceId, ids.value from TASK main join COMMUNICATION_TASK_INFO ci on "
        "ci.globalTaskId = main.globalTaskId join ids on ids.id = ci.taskType WHERE main.deviceId = ? and ci.groupName "
        "= ? AND ci.planeId = ? AND main.startNs >= ? AND main.endNs <= ? ORDER BY timestamp DESC  LIMIT ? OFFSET ?");
}

TEST_F(DbTraceDatabaseTest, GetLockRangeSqlWhenPlane)
{
    std::vector<Dic::Module::Timeline::TrackQuery> trackQueryVec;
    Dic::Module::Timeline::TrackQuery item;
    Dic::Module::Timeline::SearchAllSliceParams params;
    item.metaType = PROCESS_TYPE_ES.at(PROCESS_TYPE::HCCL);
    trackQueryVec.emplace_back(item);
    item.threadId = "999";
    params.order = "asc";
    params.isMatchCase = false;
    params.isMatchExact = false;
    std::string sql = Dic::Module::Timeline::TraceDatabaseHelper::GetLockRangeSql(params, trackQueryVec);
    EXPECT_EQ(sql,
        "with ids as (select id, value from STRING_IDS where lower(value) like lower('%'||?||'%')) SELECT main.ROWID "
        "as id, 'HCCL' as pid, ci.groupName||'_'||ci.planeId as tid, main.startNs as timestamp, main.endNs as endTime, "
        "main.depth, main.deviceId as deviceId, ids.value from TASK main join COMMUNICATION_TASK_INFO ci on "
        "ci.globalTaskId = main.globalTaskId join ids on ids.id = ci.taskType WHERE main.deviceId = ? and ci.groupName "
        "= ? AND ci.planeId = ? AND main.startNs >= ? AND main.endNs <= ? ORDER BY timestamp DESC  LIMIT ? OFFSET ?");
}

TEST_F(DbTraceDatabaseTest, GetSearchSliceNameWithLockRangeSqlWhenPython)
{
    std::vector<Dic::Module::Timeline::TrackQuery> trackQueryVec;
    Dic::Module::Timeline::TrackQuery item;
    std::string path = "lll";
    item.rankId = path;
    Dic::Module::Timeline::SearchSliceParams params;
    item.metaType = PROCESS_TYPE_ES.at(PROCESS_TYPE::API);
    trackQueryVec.emplace_back(item);
    params.isMatchCase = true;
    params.isMatchExact = true;
    std::string sql =
        Dic::Module::Timeline::TraceDatabaseHelper::GetSearchSliceNameWithLockRangeSql(params, trackQueryVec, path);
    EXPECT_EQ(sql, "with ids as (select id from STRING_IDS where value like ?)  SELECT api.ROWID as id, 'pytorch' as "
        "tid, api.globalTid as pid, api.startNs as timestamp, api.endNs as endTime, api.depth from "
        "PYTORCH_API  api join ids on ids.id = api.name WHERE api.globalTid = ? AND api.startNs >= ? AND "
        "api.endNs <= ?  ORDER BY timestamp ASC LIMIT 1 OFFSET ?");
}

TEST_F(DbTraceDatabaseTest, GetSearchSliceNameWithLockRangeSqlWhenCann)
{
    std::vector<Dic::Module::Timeline::TrackQuery> trackQueryVec;
    Dic::Module::Timeline::TrackQuery item;
    std::string path = "lll";
    item.rankId = path;
    Dic::Module::Timeline::SearchSliceParams params;
    item.metaType = PROCESS_TYPE_ES.at(PROCESS_TYPE::CANN_API);
    trackQueryVec.emplace_back(item);
    params.isMatchCase = true;
    params.isMatchExact = false;
    std::string sql =
        Dic::Module::Timeline::TraceDatabaseHelper::GetSearchSliceNameWithLockRangeSql(params, trackQueryVec, path);
    EXPECT_EQ(sql, "with ids as (select id from STRING_IDS where value like '%'||?||'%')  SELECT cann.connectionId as "
        "id, cann.globalTid as pid, cann.type as tid, cann.startNs as timestamp, cann.endNs as endTime, "
        "cann.depth from CANN_API  cann join ids on ids.id = cann.name WHERE globalTid = ? AND type = ? AND "
        "startNs >= ? AND endNs <= ?  ORDER BY timestamp ASC LIMIT 1 OFFSET ?");
}

TEST_F(DbTraceDatabaseTest, GetSearchSliceNameWithLockRangeSqlWhenMstx)
{
    std::vector<Dic::Module::Timeline::TrackQuery> trackQueryVec;
    Dic::Module::Timeline::TrackQuery item;
    std::string path = "lll";
    item.rankId = path;
    Dic::Module::Timeline::SearchSliceParams params;
    item.metaType = PROCESS_TYPE_ES.at(PROCESS_TYPE::MS_TX);
    trackQueryVec.emplace_back(item);
    params.isMatchCase = false;
    params.isMatchExact = false;
    std::string sql =
        Dic::Module::Timeline::TraceDatabaseHelper::GetSearchSliceNameWithLockRangeSql(params, trackQueryVec, path);
    EXPECT_EQ(sql, "with ids as (select id from STRING_IDS where lower(value) like lower('%'||?||'%'))  SELECT "
        "mstx.ROWID as id, mstx.globalTid as pid, mstx.domainId as tid, mstx.startNs as timestamp, mstx.endNs as "
        "endTime, mstx.depth from MSTX_EVENTS  mstx join ids on ids.id = mstx.message WHERE globalTid = ? "
        "AND startNs >= ? AND endNs <= ?  ORDER BY timestamp ASC LIMIT 1 OFFSET ?");
}

TEST_F(DbTraceDatabaseTest, GetSearchSliceNameWithLockRangeSqlWhenHardWare)
{
    std::vector<Dic::Module::Timeline::TrackQuery> trackQueryVec;
    Dic::Module::Timeline::TrackQuery item;
    std::string path = "lll";
    Dic::Module::Timeline::SearchSliceParams params;
    item.metaType = PROCESS_TYPE_ES.at(PROCESS_TYPE::ASCEND_HARDWARE);
    trackQueryVec.emplace_back(item);
    params.isMatchCase = false;
    params.isMatchExact = false;
    std::string sql =
        Dic::Module::Timeline::TraceDatabaseHelper::GetSearchSliceNameWithLockRangeSql(params, trackQueryVec, path);
    EXPECT_EQ(sql,
        "with ids as (select id from STRING_IDS where lower(value) like lower('%'||?||'%')) SELECT hadware.id as id, "
        "hadware.pid as pid, hadware.tid as tid, hadware.timestamp as timestamp, hadware.endTime as endTime, "
        "hadware.depth as depth  FROM (SELECT coalesce(c.name, m.message, s.name, main.taskType) as name, main.ROWID "
        "AS id, 'Ascend Hardware' as pid, main.streamId as tid,main.startNs as timestamp, main.endNs as endTime, "
        "main.depth as depth FROM TASK main left join COMPUTE_TASK_INFO c on c.globalTaskId = main.globalTaskId left "
        "join MSTX_EVENTS m on  (m.connectionId = main.connectionId and  m.connectionId != 4294967295 ) left join "
        "COMMUNICATION_SCHEDULE_TASK_INFO s on main.globalTaskId = s.globalTaskId WHERE main.deviceId = ? AND "
        "main.streamId = ? AND main.startNs >= ? AND main.endNs <= ?) hadware  join ids on ids.id = hadware.name  "
        "ORDER BY timestamp ASC LIMIT 1 OFFSET ?");
}

TEST_F(DbTraceDatabaseTest, GetSearchSliceNameWithLockRangeSqlWhenHccl)
{
    std::vector<Dic::Module::Timeline::TrackQuery> trackQueryVec;
    Dic::Module::Timeline::TrackQuery item;
    std::string path = "lll";
    Dic::Module::Timeline::SearchSliceParams params;
    item.metaType = PROCESS_TYPE_ES.at(PROCESS_TYPE::HCCL);
    item.threadId = "888group";
    trackQueryVec.emplace_back(item);
    params.isMatchCase = false;
    params.isMatchExact = false;
    std::string sql =
        Dic::Module::Timeline::TraceDatabaseHelper::GetSearchSliceNameWithLockRangeSql(params, trackQueryVec, path);
    EXPECT_EQ(sql, "with ids as (select id from STRING_IDS where lower(value) like lower('%'||?||'%'))  SELECT op.opId "
        "as id, 'HCCL' as pid, op.groupName||'group' as tid, op.startNs as timestamp, op.endNs as endTime, "
        "0 as depth from COMMUNICATION_OP op join ids on id = op.opName WHERE op.groupName = ? AND "
        "op.startNs >= ? AND op.endNs <= ?  ORDER BY timestamp ASC LIMIT 1 OFFSET ?");
}

TEST_F(DbTraceDatabaseTest, GetSearchCountWithLockSqlWhenPython)
{
    std::vector<Dic::Module::Timeline::TrackQuery> trackQueryVec;
    Dic::Module::Timeline::TrackQuery item;
    std::string path = "lll";
    item.rankId = path;
    Dic::Module::Timeline::SearchCountParams params;
    item.metaType = PROCESS_TYPE_ES.at(PROCESS_TYPE::API);
    trackQueryVec.emplace_back(item);
    params.isMatchCase = true;
    params.isMatchExact = true;
    params.rankId = "ll Host";
    std::string sql = Dic::Module::Timeline::TraceDatabaseHelper::GetSearchCountWithLockSql(params, trackQueryVec);
    EXPECT_EQ(sql, "with ids as (select id from STRING_IDS where value like ?) SELECT count(1) FROM (SELECT name from "
        "PYTORCH_API WHERE globalTid = ? AND startNs >= ? AND endNs <= ?) api join ids on id = api.name ");
}

TEST_F(DbTraceDatabaseTest, GetSearchCountWithLockSqlWhenCann)
{
    std::vector<Dic::Module::Timeline::TrackQuery> trackQueryVec;
    Dic::Module::Timeline::TrackQuery item;
    std::string path = "lll";
    item.rankId = path;
    Dic::Module::Timeline::SearchCountParams params;
    item.metaType = PROCESS_TYPE_ES.at(PROCESS_TYPE::CANN_API);
    trackQueryVec.emplace_back(item);
    params.isMatchCase = true;
    params.isMatchExact = false;
    params.rankId = "ll Host";
    std::string sql = Dic::Module::Timeline::TraceDatabaseHelper::GetSearchCountWithLockSql(params, trackQueryVec);
    EXPECT_EQ(sql,
        "with ids as (select id from STRING_IDS where value like '%'||?||'%') SELECT count(1) FROM (SELECT name from "
        "CANN_API WHERE globalTid = ? AND type = ? AND startNs >= ? AND endNs <= ?) cann join ids on id = cann.name ");
}

TEST_F(DbTraceDatabaseTest, GetSearchCountWithLockSqlWhenMstx)
{
    std::vector<Dic::Module::Timeline::TrackQuery> trackQueryVec;
    Dic::Module::Timeline::TrackQuery item;
    std::string path = "lll";
    item.rankId = path;
    Dic::Module::Timeline::SearchCountParams params;
    item.metaType = PROCESS_TYPE_ES.at(PROCESS_TYPE::MS_TX);
    trackQueryVec.emplace_back(item);
    params.isMatchCase = false;
    params.isMatchExact = false;
    params.rankId = "ll Host";
    std::string sql = Dic::Module::Timeline::TraceDatabaseHelper::GetSearchCountWithLockSql(params, trackQueryVec);
    EXPECT_EQ(sql, "with ids as (select id from STRING_IDS where lower(value) like lower('%'||?||'%')) SELECT count(1) "
        "FROM (SELECT message from MSTX_EVENTS WHERE globalTid = ? AND startNs >= ? AND endNs <= ?) mstx "
        "join ids on id = mstx.message ");
}

TEST_F(DbTraceDatabaseTest, GetSearchCountWithLockSqlWhenHardWare)
{
    std::vector<Dic::Module::Timeline::TrackQuery> trackQueryVec;
    Dic::Module::Timeline::TrackQuery item;
    std::string path = "lll";
    Dic::Module::Timeline::SearchCountParams params;
    item.metaType = PROCESS_TYPE_ES.at(PROCESS_TYPE::ASCEND_HARDWARE);
    trackQueryVec.emplace_back(item);
    params.isMatchCase = false;
    params.isMatchExact = false;
    std::string sql = Dic::Module::Timeline::TraceDatabaseHelper::GetSearchCountWithLockSql(params, trackQueryVec);
    EXPECT_EQ(sql,
        "with ids as (select id from STRING_IDS where lower(value) like lower('%'||?||'%')) SELECT count(1) FROM "
        "(SELECT coalesce(c.name, m.message, s.name, main.taskType) as name FROM TASK main  left join "
        "COMPUTE_TASK_INFO c on c.globalTaskId = main.globalTaskId  left join MSTX_EVENTS m on  (m.connectionId = "
        "main.connectionId and  m.connectionId != 4294967295 ) left join COMMUNICATION_SCHEDULE_TASK_INFO s on "
        "main.globalTaskId = s.globalTaskId WHERE main.deviceId = ? AND main.streamId = ? AND main.startNs >= ? AND "
        "main.endNs <= ?) hadware  join ids on id = hadware.name ");
}

TEST_F(DbTraceDatabaseTest, GetSearchCountWithLockSqlWhenHccl)
{
    std::vector<Dic::Module::Timeline::TrackQuery> trackQueryVec;
    Dic::Module::Timeline::TrackQuery item;
    std::string path = "lll";
    Dic::Module::Timeline::SearchCountParams params;
    item.metaType = PROCESS_TYPE_ES.at(PROCESS_TYPE::HCCL);
    item.threadId = "888group";
    trackQueryVec.emplace_back(item);
    params.isMatchCase = false;
    params.isMatchExact = false;
    std::string sql = Dic::Module::Timeline::TraceDatabaseHelper::GetSearchCountWithLockSql(params, trackQueryVec);
    EXPECT_EQ(sql, "with ids as (select id from STRING_IDS where lower(value) like lower('%'||?||'%')) SELECT count(1) "
        "FROM (SELECT opName as name from COMMUNICATION_OP WHERE groupName = ? AND startNs >= ? AND endNs "
        "<= ?) op join ids on id = op.name ");
}

TEST_F(DbTraceDatabaseTest, GetSearchCountWithLockSqlWhenHcclPlane)
{
    std::vector<Dic::Module::Timeline::TrackQuery> trackQueryVec;
    Dic::Module::Timeline::TrackQuery item;
    std::string path = "lll";
    Dic::Module::Timeline::SearchSliceParams params;
    item.metaType = PROCESS_TYPE_ES.at(PROCESS_TYPE::HCCL);
    item.threadId = "888";
    trackQueryVec.emplace_back(item);
    params.isMatchCase = false;
    params.isMatchExact = false;
    std::string sql =
        Dic::Module::Timeline::TraceDatabaseHelper::GetSearchSliceNameWithLockRangeSql(params, trackQueryVec, path);
    EXPECT_EQ(sql,
        "with ids as (select id from STRING_IDS where lower(value) like lower('%'||?||'%')) SELECT main.ROWID as id, "
        "'HCCL' as pid, ci.groupName||'_'||ci.planeId as tid, main.startNs as timestamp, main.endNs as endTime, "
        "main.depth from TASK main join COMMUNICATION_TASK_INFO ci on ci.globalTaskId = main.globalTaskId join ids on "
        "ids.id = ci.taskType WHERE main.deviceId = ? and ci.groupName = ? AND ci.planeId = ? AND main.startNs >= ? "
        "AND main.endNs <= ? ORDER BY timestamp ASC LIMIT 1 OFFSET ?");
}

TEST_F(DbTraceDatabaseTest, ProcessByteAlignmentAnalyzerDataForDbTest)
{
    std::vector<Dic::Module::ByteAlignmentAnalyzerLargeOperatorInfo> largeOpInfo = {{"hcom1"}, {"hcom2"}};
    std::vector<Dic::Module::ByteAlignmentAnalyzerSmallOperatorInfo> smallOpInfo = {
        {"hcom3", "Memcpy", 2, "SDMA", "ON_CHIP"}, {"hcom1", "Memcpy", 2, "SDMA", "ON_CHIP"},
        {"hcom1", "Reduce_Inline", 2, "SDMA", "ON_CHIP"}, {"hcom2", "Memcpy", 2, "SDMA", "ON_CHIP"}};
    std::vector<Dic::Module::CommunicationLargeOperatorInfo> result;
    TraceDatabaseHelper::ProcessByteAlignmentAnalyzerDataForDb(result, largeOpInfo, smallOpInfo);
    ASSERT_EQ(result.size(), 2); // 2
    ASSERT_EQ(result[0].memcpyTasks.size(), 1);
    ASSERT_EQ(result[0].reduceInlineTasks.size(), 1);
    ASSERT_EQ(result[1].memcpyTasks.size(), 1);
    ASSERT_EQ(result[1].reduceInlineTasks.size(), 0);
}