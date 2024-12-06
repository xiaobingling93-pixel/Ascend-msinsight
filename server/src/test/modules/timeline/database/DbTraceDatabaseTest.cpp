/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include "DbTraceDataBase.h"
#include "DataBaseManager.h"
#include "../../../DatabaseTestCaseMockUtil.cpp"
using namespace Dic::Global::PROFILER::MockUtil;
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
    std::string comcaInfoSql =
        "CREATE TABLE COMMUNICATION_TASK_INFO (name INTEGER,globalTaskId INTEGER,taskType INTEGER,planeId "
        "INTEGER,groupName INTEGER,notifyId INTEGER,rdmaType INTEGER,srcRank INTEGER,dstRank INTEGER,transportType "
        "INTEGER,size INTEGER,dataType INTEGER,linkType INTEGER,opId INTEGER);";
    std::string comcaOpSql = "CREATE TABLE COMMUNICATION_OP (opName INTEGER,startNs INTEGER,endNs INTEGER,connectionId "
        "INTEGER,groupName INTEGER,opId INTEGER PRIMARY KEY,relay INTEGER,retry INTEGER,dataType "
        "INTEGER,algType INTEGER,count NUMERIC,opType INTEGER, waitNs INTEGER);";
    std::string cannSql = "CREATE TABLE CANN_API (startNs INTEGER,endNs INTEGER,type INTEGER,globalTid "
        "INTEGER,connectionId INTEGER PRIMARY KEY,name INTEGER, depth integer);";
    std::string mstxSql = "CREATE TABLE MSTX_EVENTS (startNs INTEGER,endNs INTEGER,eventType INTEGER,rangeId "
        "INTEGER,category INTEGER,message INTEGER,globalTid INTEGER,endGlobalTid "
        "INTEGER,domainId INTEGER,connectionId INTEGER, depth integer);";
    std::string overlap = "CREATE TABLE OVERLAP_ANALYSIS (id INTEGER PRIMARY KEY AUTOINCREMENT, deviceId integer, "
        "startNs integer, endNs integer, type integer);";
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
    EXPECT_EQ(rankIds.size(), 0);
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
    const int64_t traceId = 0;
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
    const int64_t traceId = 0;
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
    requestParams.metaType = "Ascend Hardware";
    requestParams.orderBy = "name";
    requestParams.order = "DESC";
    Dic::Protocol::UnitThreadsOperatorsBody responseBody;
    const uint64_t minTimestamp = 0;
    const int64_t traceId = 0;
    bool result = database.QueryThreadSameOperatorsDetails(requestParams, responseBody, minTimestamp, traceId);
    EXPECT_EQ(result, true);
}

TEST_F(DbTraceDatabaseTest, TestQueryThreadSameOperatorsDetailsWhenHccl)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::OpenDBAndCreateTable(db, comcaInfoSql);
    DatabaseTestCaseMockUtil::OpenDBAndCreateTable(db, comcaOpSql);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    DatabaseTestCaseMockUtil::CreateTable(db, taskSql);
    database.SetDbPtr(db);
    Dic::Protocol::UnitThreadsOperatorsParams requestParams;
    requestParams.metaType = "HCCL";
    requestParams.orderBy = "name";
    requestParams.order = "DESC";
    Dic::Protocol::UnitThreadsOperatorsBody responseBody;
    const uint64_t minTimestamp = 0;
    const int64_t traceId = 0;
    bool result = database.QueryThreadSameOperatorsDetails(requestParams, responseBody, minTimestamp, traceId);
    EXPECT_EQ(result, false);
}

TEST_F(DbTraceDatabaseTest, TestQueryThreadSameOperatorsDetailsWhenCANN)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    DatabaseTestCaseMockUtil::CreateTable(db, cannSql);
    database.SetDbPtr(db);
    Dic::Protocol::UnitThreadsOperatorsParams requestParams;
    requestParams.metaType = "CANN_API";
    requestParams.orderBy = "name";
    requestParams.order = "DESC";
    Dic::Protocol::UnitThreadsOperatorsBody responseBody;
    const uint64_t minTimestamp = 0;
    const int64_t traceId = 0;
    bool result = database.QueryThreadSameOperatorsDetails(requestParams, responseBody, minTimestamp, traceId);
    EXPECT_EQ(result, true);
}

TEST_F(DbTraceDatabaseTest, TestQueryThreadSameOperatorsDetailsWhenMstx)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, mstxSql);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    database.SetDbPtr(db);
    Dic::Protocol::UnitThreadsOperatorsParams requestParams;
    requestParams.metaType = "MSTX_EVENTS";
    requestParams.orderBy = "depth";
    requestParams.order = "DESC";
    Dic::Protocol::UnitThreadsOperatorsBody responseBody;
    const uint64_t minTimestamp = 0;
    const int64_t traceId = 0;
    bool result = database.QueryThreadSameOperatorsDetails(requestParams, responseBody, minTimestamp, traceId);
    EXPECT_EQ(result, true);
}

TEST_F(DbTraceDatabaseTest, TestQueryThreadSameOperatorsDetailsWhenOverlap)
{
    std::recursive_mutex testMutex;
    MockDatabase2 database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    DatabaseTestCaseMockUtil::CreateTable(db, overlap);
    database.SetDbPtr(db);
    Dic::Protocol::UnitThreadsOperatorsParams requestParams;
    requestParams.metaType = "OVERLAP_ANALYSIS";
    requestParams.orderBy = "depth";
    requestParams.order = "DESC";
    Dic::Protocol::UnitThreadsOperatorsBody responseBody;
    const uint64_t minTimestamp = 0;
    const int64_t traceId = 0;
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
    DatabaseTestCaseMockUtil::CreateTable(db, pytorchSql);
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
    requestParams.pid = "17738580008830245";
    requestParams.metaType = "PYTORCH_API";
    requestParams.orderBy = "depth";
    requestParams.order = "DESC";
    const uint64_t min = 0;
    const uint64_t max = 1823510445651061410;
    requestParams.startTime = min;
    requestParams.endTime = max;
    requestParams.name = "FORMAT_ND";
    Dic::Protocol::UnitThreadsOperatorsBody responseBody;
    const uint64_t minTimestamp = 0;
    const int64_t traceId = 0;
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