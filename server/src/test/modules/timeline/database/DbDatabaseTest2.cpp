/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include "DatabaseTest.cpp"
#include "DbTraceDataBase.h"
#include "CollectionTimeService.h"
#include "TraceDatabaseHelper.h"
#include "NpuInfoRepoMock.h"
#include "../../../DatabaseTestCaseMockUtil.cpp"
using namespace Dic::Global::PROFILER::MockUtil;
class DbDatabaseTest2 : public ::testing::Test {
protected:
    const std::string taskTableSql = "CREATE TABLE TASK (startNs INTEGER,endNs INTEGER,deviceId INTEGER,connectionId "
        "INTEGER,globalTaskId INTEGER,globalPid INTEGER,taskType INTEGER,contextId "
        "INTEGER,streamId INTEGER,taskId INTEGER,modelId INTEGER, depth integer);";
    const std::string commucationInfoSql =
        "CREATE TABLE COMMUNICATION_TASK_INFO (name INTEGER,globalTaskId INTEGER,taskType INTEGER,planeId "
        "INTEGER,groupName INTEGER,notifyId INTEGER,rdmaType INTEGER,srcRank INTEGER,dstRank INTEGER,transportType "
        "INTEGER,size INTEGER,dataType INTEGER,linkType INTEGER,opId INTEGER);";
    const std::string commucationOpSql =
        "CREATE TABLE COMMUNICATION_OP (opName INTEGER,startNs INTEGER,endNs INTEGER,connectionId INTEGER,groupName "
        "INTEGER,opId INTEGER PRIMARY KEY,relay INTEGER,retry INTEGER,dataType INTEGER,algType INTEGER,count "
        "NUMERIC,opType INTEGER, waitNs INTEGER);";
    const std::string pytorchApiSql =
        "CREATE TABLE PYTORCH_API (startNs TEXT, endNs TEXT, globalTid INTEGER, connectionId INTEGER, name INTEGER, "
        "sequenceNumber INTEGER, fwdThreadId INTEGER, inputDtypes INTEGER, inputShapes INTEGER, callchainId INTEGER, "
        "type INTEGER, depth integer);";
    const std::string canSql = "CREATE TABLE CANN_API (startNs INTEGER,endNs INTEGER,type INTEGER,globalTid "
        "INTEGER,connectionId INTEGER PRIMARY KEY,name INTEGER, depth integer);";
    const std::string mstxSql = "CREATE TABLE MSTX_EVENTS (startNs INTEGER,endNs INTEGER,eventType INTEGER,rangeId "
        "INTEGER,category INTEGER,message INTEGER,globalTid INTEGER,endGlobalTid "
        "INTEGER,domainId INTEGER,connectionId INTEGER, depth integer);";
    const std::string enumApiType = "CREATE TABLE ENUM_API_TYPE (id INTEGER PRIMARY KEY,name TEXT);";
    std::string computeSql =
        "CREATE TABLE COMPUTE_TASK_INFO (name INTEGER,globalTaskId INTEGER PRIMARY KEY,blockDim INTEGER,mixBlockDim "
        "INTEGER,taskType INTEGER,opType INTEGER,inputFormats INTEGER,inputDataTypes INTEGER,inputShapes "
        "INTEGER,outputFormats INTEGER,outputDataTypes INTEGER,outputShapes INTEGER,attrInfo INTEGER, waitNs INTEGER);";
    std::string stringIdsSql = "CREATE TABLE STRING_IDS (id INTEGER PRIMARY KEY,value TEXT);";
    std::string overlap = "CREATE TABLE OVERLAP_ANALYSIS (id INTEGER PRIMARY KEY AUTOINCREMENT, deviceId integer, "
        "startNs integer, endNs integer, type integer);";
    const std::string schedulerTable = "CREATE table COMMUNICATION_SCHEDULE_TASK_INFO("
         "name INTEGER, globalTaskId INTEGER primary key, taskType INTEGER, opType INTEGER);";
};
class MockDatabase : public Dic::Module::FullDb::DbTraceDataBase {
public:
    explicit MockDatabase(std::recursive_mutex &sqlMutex) : DbTraceDataBase(sqlMutex) {}
    void SetDbPtr(sqlite3 *dbPtr)
    {
        isOpen = true;
        db = dbPtr;
        path = ":memory:";
        InitStringsCache();
        return;
    }
    void SetMetaVersion(const std::string &version)
    {
        metaVersion = version;
    }
};
TEST_F(DbDatabaseTest2, TestQueryUnitsMetadataWhenTaskNotExist)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    std::string fileId = "7";
    std::vector<std::unique_ptr<Dic::Protocol::UnitTrack>> metaData;
    database.QueryUnitsMetadata(fileId, metaData);
    EXPECT_EQ(metaData.size(), 0);
}

TEST_F(DbDatabaseTest2, TestQueryUnitsMetadataWhenStreamTrackExist)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    database.SetDbPtr(db);
    DatabaseTestCaseMockUtil::CreateTable(db, taskTableSql);
    DatabaseTestCaseMockUtil::CreateTable(db, commucationInfoSql);
    DatabaseTestCaseMockUtil::CreateTable(db, commucationOpSql);
    std::string taskTableInsert =
        "INSERT INTO \"TASK\" (\"startNs\", \"endNs\", \"deviceId\", \"connectionId\", \"globalTaskId\", "
        "\"globalPid\", \"taskType\", \"contextId\", \"streamId\", \"taskId\", \"modelId\", \"depth\") VALUES "
        "(1729733883833924932, 1729733883833924952, 7, 4294967295, 82550, 511284, 221, 4294967295, 2, 40, 4294967295, "
        "0);";
    DatabaseTestCaseMockUtil::InsertData(db, taskTableInsert);
    std::string fileId = "7";
    std::vector<std::unique_ptr<Dic::Protocol::UnitTrack>> metaData;
    const uint8_t expectProcessCount = 2;
    const uint8_t expectHardWareCount = 1;
    const std::string expectStreamName = "Stream 2";
    database.QueryUnitsMetadata(fileId, metaData);
    EXPECT_EQ(metaData.size(), expectProcessCount);
    EXPECT_EQ(metaData[0]->children.size(), expectHardWareCount);
    EXPECT_EQ(metaData[0]->children[0]->metaData.threadName, expectStreamName);
    EXPECT_EQ(metaData[0]->children[0]->metaData.cardId, fileId);
}

/**
 * 只存在task表不存在commucation相关表就一个泳道都没有
 */
TEST_F(DbDatabaseTest2, TestQueryUnitsMetadataWhenTaskExistCommucationNotExist)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    database.SetDbPtr(db);
    DatabaseTestCaseMockUtil::CreateTable(db, taskTableSql);
    std::string fileId = "7";
    std::vector<std::unique_ptr<Dic::Protocol::UnitTrack>> metaData;
    const uint8_t expectProcessCount = 1;
    database.QueryUnitsMetadata(fileId, metaData);
    EXPECT_EQ(metaData.size(), expectProcessCount);
}

/**
 * 查询hccl的plane泳道
 */
TEST_F(DbDatabaseTest2, TestQueryUnitsMetadataWhenPlaneTrackExist)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    database.SetDbPtr(db);
    DatabaseTestCaseMockUtil::CreateTable(db, taskTableSql);
    DatabaseTestCaseMockUtil::CreateTable(db, commucationInfoSql);
    DatabaseTestCaseMockUtil::CreateTable(db, commucationOpSql);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    std::string taskTableInsert =
        "INSERT INTO \"TASK\" (\"startNs\", \"endNs\", \"deviceId\", \"connectionId\", \"globalTaskId\", "
        "\"globalPid\", \"taskType\", \"contextId\", \"streamId\", \"taskId\", \"modelId\", \"depth\") VALUES "
        "(1729733883833924932, 1729733883833924952, 7, 4294967295, 21412, 511284, 221, 4294967295, 2, 40, 4294967295, "
        "0);";
    std::string commucationInfoInsertSql =
        "INSERT INTO \"COMMUNICATION_TASK_INFO\" (\"name\", \"globalTaskId\", \"taskType\", \"planeId\", "
        "\"groupName\", \"notifyId\", \"rdmaType\", \"srcRank\", \"dstRank\", \"transportType\", \"size\", "
        "\"dataType\", \"linkType\", \"opId\") VALUES (6, 21412, 7, 0, 8, 9223372036854775807, 65535, 0, 0, 0, 4, "
        "65535, 0, 1);";
    std::string commucationOpInsertSql =
        "INSERT INTO \"COMMUNICATION_OP\" (\"opName\", \"startNs\", \"endNs\", \"connectionId\", "
        "\"groupName\", \"opId\", \"relay\", \"retry\", \"dataType\", \"algType\", \"count\", \"opType\", \"waitNs\") "
        "VALUES (6, 1729773871230644118, 1729773871230661178, 144529, 8, 1, 0, 0, 4, 9, 1, 10, 726280);";
    DatabaseTestCaseMockUtil::InsertData(db, taskTableInsert);
    DatabaseTestCaseMockUtil::InsertData(db, commucationInfoInsertSql);
    DatabaseTestCaseMockUtil::InsertData(db, commucationOpInsertSql);
    std::string fileId = "7";
    std::vector<std::unique_ptr<Dic::Protocol::UnitTrack>> metaData;
    const uint8_t expectProcessCount = 3;
    const uint8_t first = 0;
    const uint8_t second = 1;
    const uint8_t three = 2;
    const std::string expectGroupName = "Group 0 Communication";
    const std::string expectPlaneName = "Plane 0";
    database.QueryUnitsMetadata(fileId, metaData);
    EXPECT_EQ(metaData.size(), expectProcessCount);
    EXPECT_EQ(metaData[second]->children.size(), three);
    EXPECT_EQ(metaData[second]->children[first]->metaData.threadName, expectGroupName);
    EXPECT_EQ(metaData[second]->children[second]->metaData.threadName, expectPlaneName);
}

/**
 * 查询hccl的plane泳道(deviceId唯一)
 */
TEST_F(DbDatabaseTest2, TestQueryUnitsMetadataWhenDeviceUnique)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    database.SetDbPtr(db);
    DatabaseTestCaseMockUtil::CreateTable(db, taskTableSql);
    DatabaseTestCaseMockUtil::CreateTable(db, commucationInfoSql);
    DatabaseTestCaseMockUtil::CreateTable(db, commucationOpSql);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    std::string taskTableInsert =
        "INSERT INTO \"TASK\" (\"startNs\", \"endNs\", \"deviceId\", \"connectionId\", \"globalTaskId\", "
        "\"globalPid\", \"taskType\", \"contextId\", \"streamId\", \"taskId\", \"modelId\", \"depth\") VALUES "
        "(1729733883833924932, 1729733883833924952, 0, 4294967295, 21412, 511284, 221, 4294967295, 2, 40, 4294967295, "
        "0);";
    std::string commucationOpInsertSql =
        "INSERT INTO \"COMMUNICATION_OP\" (\"opName\", \"startNs\", \"endNs\", \"connectionId\", "
        "\"groupName\", \"opId\", \"relay\", \"retry\", \"dataType\", \"algType\", \"count\", \"opType\", \"waitNs\") "
        "VALUES (6, 1729773871230644118, 1729773871230661178, 144529, 8, 1, 0, 0, 4, 9, 1, 10, 726280);";
    DatabaseTestCaseMockUtil::InsertData(db, taskTableInsert);
    DatabaseTestCaseMockUtil::InsertData(db, commucationOpInsertSql);
    MockNpuInfoRepoFunc();
    std::string fileId = "0";
    std::vector<std::unique_ptr<Dic::Protocol::UnitTrack>> metaData;
    const uint8_t expectProcessCount = 3;
    const uint8_t zero = 0;
    const uint8_t one = 1;
    const std::string expectGroupName = "Group 0 Communication";
    database.QueryUnitsMetadata(fileId, metaData);
    EXPECT_EQ(metaData.size(), expectProcessCount);
    EXPECT_EQ(metaData[one]->children.size(), one);
    EXPECT_EQ(metaData[one]->children[zero]->metaData.threadName, expectGroupName);
    RestoreRepoFunc();
}

/**
 * 过滤plane为4294967295的泳道
 */
TEST_F(DbDatabaseTest2, TestQueryUnitsMetadataWhenPlaneTrackIsWrong)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    database.SetDbPtr(db);
    DatabaseTestCaseMockUtil::CreateTable(db, taskTableSql);
    DatabaseTestCaseMockUtil::CreateTable(db, commucationInfoSql);
    DatabaseTestCaseMockUtil::CreateTable(db, commucationOpSql);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    std::string taskTableInsert =
        "INSERT INTO \"TASK\" (\"startNs\", \"endNs\", \"deviceId\", \"connectionId\", \"globalTaskId\", "
        "\"globalPid\", \"taskType\", \"contextId\", \"streamId\", \"taskId\", \"modelId\", \"depth\") VALUES "
        "(1729733883833924932, 1729733883833924952, 7, 4294967295, 21412, 511284, 221, 4294967295, 2, 40, 4294967295, "
        "0);";
    std::string commucationInfoInsertSql =
        "INSERT INTO \"COMMUNICATION_TASK_INFO\" (\"name\", \"globalTaskId\", \"taskType\", \"planeId\", "
        "\"groupName\", \"notifyId\", \"rdmaType\", \"srcRank\", \"dstRank\", \"transportType\", \"size\", "
        "\"dataType\", \"linkType\", \"opId\") VALUES (6, 21412, 7, 4294967295, 8, 9223372036854775807, 65535, 0, 0, "
        "0, 4, "
        "65535, 0, 1);";
    std::string commucationOpInsertSql =
        "INSERT INTO \"COMMUNICATION_OP\" (\"opName\", \"startNs\", \"endNs\", \"connectionId\", "
        "\"groupName\", \"opId\", \"relay\", \"retry\", \"dataType\", \"algType\", \"count\", \"opType\", \"waitNs\") "
        "VALUES (6, 1729773871230644118, 1729773871230661178, 144529, 8, 1, 0, 0, 4, 9, 1, 10, 726280);";
    DatabaseTestCaseMockUtil::InsertData(db, taskTableInsert);
    DatabaseTestCaseMockUtil::InsertData(db, commucationInfoInsertSql);
    DatabaseTestCaseMockUtil::InsertData(db, commucationOpInsertSql);
    std::string fileId = "7";
    std::vector<std::unique_ptr<Dic::Protocol::UnitTrack>> metaData;
    const uint8_t expectProcessCount = 3;
    const uint8_t first = 0;
    const uint8_t second = 1;
    const std::string expectGroupName = "Group 0 Communication";
    database.QueryUnitsMetadata(fileId, metaData);
    EXPECT_EQ(metaData.size(), expectProcessCount);
    EXPECT_EQ(metaData[second]->children.size(), second);
    EXPECT_EQ(metaData[second]->children[first]->metaData.threadName, expectGroupName);
}

/**
 * 查询hccl的plane泳道
 * metaVersion 1.1.0
 */
TEST_F(DbDatabaseTest2, TestQueryUnitsMetadataWhenPlaneTrackExistVersion_1_1_0)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    database.SetMetaVersion("1.1.0");
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, taskTableSql);
    DatabaseTestCaseMockUtil::CreateTable(db, commucationInfoSql);
    DatabaseTestCaseMockUtil::CreateTable(db, commucationOpSql);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    std::string taskTableInsert =
        "INSERT INTO \"TASK\" (\"startNs\", \"endNs\", \"deviceId\", \"connectionId\", \"globalTaskId\", "
        "\"globalPid\", \"taskType\", \"contextId\", \"streamId\", \"taskId\", \"modelId\", \"depth\") VALUES "
        "(1729733883833924932, 1729733883833924952, 7, 4294967295, 21412, 511284, 221, 4294967295, 2, 40, 4294967295, "
        "0);";
    std::string commucationInfoInsertSql =
        "INSERT INTO \"COMMUNICATION_TASK_INFO\" (\"name\", \"globalTaskId\", \"taskType\", \"planeId\", "
        "\"groupName\", \"notifyId\", \"rdmaType\", \"srcRank\", \"dstRank\", \"transportType\", \"size\", "
        "\"dataType\", \"linkType\", \"opId\") VALUES (6, 21412, 7, 0, 8, 9223372036854775807, 65535, 0, 0, 0, 4, "
        "65535, 0, 1);";
    std::string commucationOpInsertSql =
        "INSERT INTO \"COMMUNICATION_OP\" (\"opName\", \"startNs\", \"endNs\", \"connectionId\", "
        "\"groupName\", \"opId\", \"relay\", \"retry\", \"dataType\", \"algType\", \"count\", \"opType\", \"waitNs\") "
        "VALUES (6, 1729773871230644118, 1729773871230661178, 144529, 8, 1, 0, 0, 4, 9, 1, 10, 726280);";
    const std::string groupNameValue = "90.90.97.96%enp194s0f0_60008_8_1735556595505601";
    const std::string stringIdsInsertSql = "INSERT INTO \"STRING_IDS\" (\"id\",\"value\") "
        "VALUES (8, '90.90.97.96%enp194s0f0_60008_8_1735556595505601');";
    DatabaseTestCaseMockUtil::InsertData(db, taskTableInsert);
    DatabaseTestCaseMockUtil::InsertData(db, commucationInfoInsertSql);
    DatabaseTestCaseMockUtil::InsertData(db, commucationOpInsertSql);
    DatabaseTestCaseMockUtil::InsertData(db, stringIdsInsertSql);
    database.SetDbPtr(db);
    std::string fileId = "7";
    std::vector<std::unique_ptr<Dic::Protocol::UnitTrack>> metaData;
    const uint8_t expectProcessCount = 3;
    const uint8_t first = 0;
    const uint8_t second = 1;
    const uint8_t three = 2;
    const std::string expectPlaneName = "Plane 0";
    database.QueryUnitsMetadata(fileId, metaData);
    EXPECT_EQ(metaData.size(), expectProcessCount);
    EXPECT_EQ(metaData[second]->children.size(), three);
    EXPECT_EQ(metaData[second]->children[first]->metaData.groupNameValue, groupNameValue);
    EXPECT_EQ(metaData[second]->children[second]->metaData.threadName, expectPlaneName);
    EXPECT_EQ(metaData[second]->children[second]->metaData.groupNameValue, "");
}


/**
 * 过滤plane为4294967295的泳道
 * metaVersion 1.1.0
 */
TEST_F(DbDatabaseTest2, TestQueryUnitsMetadataWhenPlaneTrackIsWrongVersion_1_1_0)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    database.SetMetaVersion("1.1.0");
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, taskTableSql);
    DatabaseTestCaseMockUtil::CreateTable(db, commucationInfoSql);
    DatabaseTestCaseMockUtil::CreateTable(db, commucationOpSql);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    std::string taskTableInsert =
        "INSERT INTO \"TASK\" (\"startNs\", \"endNs\", \"deviceId\", \"connectionId\", \"globalTaskId\", "
        "\"globalPid\", \"taskType\", \"contextId\", \"streamId\", \"taskId\", \"modelId\", \"depth\") VALUES "
        "(1729733883833924932, 1729733883833924952, 7, 4294967295, 21412, 511284, 221, 4294967295, 2, 40, 4294967295, "
        "0);";
    std::string commucationInfoInsertSql =
        "INSERT INTO \"COMMUNICATION_TASK_INFO\" (\"name\", \"globalTaskId\", \"taskType\", \"planeId\", "
        "\"groupName\", \"notifyId\", \"rdmaType\", \"srcRank\", \"dstRank\", \"transportType\", \"size\", "
        "\"dataType\", \"linkType\", \"opId\") VALUES (6, 21412, 7, 4294967295, 8, 9223372036854775807, 65535, 0, 0, "
        "0, 4, "
        "65535, 0, 1);";
    std::string commucationOpInsertSql =
        "INSERT INTO \"COMMUNICATION_OP\" (\"opName\", \"startNs\", \"endNs\", \"connectionId\", "
        "\"groupName\", \"opId\", \"relay\", \"retry\", \"dataType\", \"algType\", \"count\", \"opType\", \"waitNs\") "
        "VALUES (6, 1729773871230644118, 1729773871230661178, 144529, 8, 1, 0, 0, 4, 9, 1, 10, 726280);";
    const std::string groupNameValue = "90.90.97.96%enp194s0f0_60008_8_1735556595505601";
    const std::string stringIdsInsertSql = "INSERT INTO \"STRING_IDS\" (\"id\",\"value\") "
        "VALUES (8, '90.90.97.96%enp194s0f0_60008_8_1735556595505601');";
    DatabaseTestCaseMockUtil::InsertData(db, taskTableInsert);
    DatabaseTestCaseMockUtil::InsertData(db, commucationInfoInsertSql);
    DatabaseTestCaseMockUtil::InsertData(db, commucationOpInsertSql);
    DatabaseTestCaseMockUtil::InsertData(db, stringIdsInsertSql);
    database.SetDbPtr(db);
    std::string fileId = "7";
    std::vector<std::unique_ptr<Dic::Protocol::UnitTrack>> metaData;
    const uint8_t expectProcessCount = 3;
    const uint8_t first = 0;
    const uint8_t second = 1;
    database.QueryUnitsMetadata(fileId, metaData);
    EXPECT_EQ(metaData.size(), expectProcessCount);
    EXPECT_EQ(metaData[second]->children.size(), second);
    EXPECT_EQ(metaData[second]->children[first]->metaData.groupNameValue, groupNameValue);
}

/**
 * 测试pytorch，cann，mstx都存在的情况下的泳道信息
 */
TEST_F(DbDatabaseTest2, TestQueryHostMetadataWhenAllHostExistThenhaveThreeTrack)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    database.SetDbPtr(db);
    DatabaseTestCaseMockUtil::CreateTable(db, pytorchApiSql);
    DatabaseTestCaseMockUtil::CreateTable(db, canSql);
    DatabaseTestCaseMockUtil::CreateTable(db, mstxSql);
    DatabaseTestCaseMockUtil::CreateTable(db, enumApiType);
    std::string pytorchDataSql =
        "INSERT INTO \"main\".\"PYTORCH_API\" (\"startNs\", \"endNs\", \"globalTid\", \"connectionId\", \"name\", "
        "\"sequenceNumber\", \"fwdThreadId\", \"inputDtypes\", \"inputShapes\", \"callchainId\", \"depth\") VALUES "
        "('1718180918997274130', '1718180918997289000', 8785587534247538, 0, 268435456, NULL, NULL, NULL, NULL, NULL, "
        "8);";
    std::string cannDataSql = "INSERT INTO \"main\".\"CANN_API\" (\"startNs\", \"endNs\", \"type\", \"globalTid\", "
        "\"connectionId\", \"name\", \"depth\") VALUES (1729478236911261506, "
        "1729478236911265550, 20000, 1237912654215057, 250011, 5413, 0);";
    std::string mstxDataSql = "INSERT INTO \"main\".\"MSTX_EVENTS\" (\"startNs\", \"endNs\", \"eventType\", "
        "\"rangeId\", \"category\", \"message\", \"globalTid\", \"endGlobalTid\", \"domainId\", "
        "\"connectionId\", \"depth\") VALUES (947741767895850870, 947741768895903230, 2, "
        "4294967295, 4294967295, 8, 16884049020452276, 16884049020452276, 65535, 4000000001, 0);";
    std::string numeApiDataSql = "INSERT INTO \"main\".\"ENUM_API_TYPE\" (\"id\", \"name\") VALUES (20000, 'acl');";
    DatabaseTestCaseMockUtil::InsertData(db, pytorchDataSql);
    DatabaseTestCaseMockUtil::InsertData(db, cannDataSql);
    DatabaseTestCaseMockUtil::InsertData(db, mstxDataSql);
    DatabaseTestCaseMockUtil::InsertData(db, numeApiDataSql);
    std::vector<std::unique_ptr<Dic::Protocol::UnitTrack>> metaData;
    database.QueryHostMetadata(metaData);
    const uint64_t expectSize = 3;
    const uint64_t first = 0;
    const uint64_t second = 1;
    const uint64_t third = 2;
    EXPECT_EQ(metaData.size(), expectSize);
    EXPECT_EQ(metaData[first]->metaData.processName, "process 288224");
    EXPECT_EQ(metaData[first]->children[first]->metaData.metaType, "CANN_API");
    EXPECT_EQ(metaData[first]->children[first]->metaData.threadId, "292753");
    EXPECT_EQ(metaData[first]->children[first]->children[first]->metaData.threadId, "");
    EXPECT_EQ(metaData[first]->children[first]->children[first]->metaData.processId, "1237912654215057");
    EXPECT_EQ(metaData[second]->metaData.processName, "process 3931124");
    EXPECT_EQ(metaData[second]->children[first]->metaData.metaType, "CANN_API");
    EXPECT_EQ(metaData[second]->children[first]->metaData.threadId, "3931572");
    EXPECT_EQ(metaData[second]->children[first]->children[first]->metaData.threadId, "MsTx");
    EXPECT_EQ(metaData[second]->children[first]->children[first]->metaData.processId, "16884049020452276");
    EXPECT_EQ(metaData[third]->metaData.processName, "process 2045554");
    EXPECT_EQ(metaData[third]->children[first]->metaData.metaType, "CANN_API");
    EXPECT_EQ(metaData[third]->children[first]->metaData.threadId, "2045554");
    EXPECT_EQ(metaData[third]->children[first]->children[first]->metaData.threadId, "pytorch");
    EXPECT_EQ(metaData[third]->children[first]->children[first]->metaData.processId, "8785587534247538");
}

/**
 * 测试cann，mstx都存在但pytorch不存在的情况下的泳道信息
 */
TEST_F(DbDatabaseTest2, TestQueryHostMetadataWhenPytorchNotExistThenhaveTwoTrack)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    database.SetDbPtr(db);
    DatabaseTestCaseMockUtil::CreateTable(db, canSql);
    DatabaseTestCaseMockUtil::CreateTable(db, mstxSql);
    DatabaseTestCaseMockUtil::CreateTable(db, enumApiType);
    std::string cannDataSql = "INSERT INTO \"main\".\"CANN_API\" (\"startNs\", \"endNs\", \"type\", \"globalTid\", "
        "\"connectionId\", \"name\", \"depth\") VALUES (1729478236911261506, "
        "1729478236911265550, 20000, 1237912654215057, 250011, 5413, 0);";
    std::string mstxDataSql = "INSERT INTO \"main\".\"MSTX_EVENTS\" (\"startNs\", \"endNs\", \"eventType\", "
        "\"rangeId\", \"category\", \"message\", \"globalTid\", \"endGlobalTid\", \"domainId\", "
        "\"connectionId\", \"depth\") VALUES (947741767895850870, 947741768895903230, 2, "
        "4294967295, 4294967295, 8, 16884049020452276, 16884049020452276, 65535, 4000000001, 0);";
    std::string numeApiDataSql = "INSERT INTO \"main\".\"ENUM_API_TYPE\" (\"id\", \"name\") VALUES (20000, 'acl');";
    DatabaseTestCaseMockUtil::InsertData(db, cannDataSql);
    DatabaseTestCaseMockUtil::InsertData(db, mstxDataSql);
    DatabaseTestCaseMockUtil::InsertData(db, numeApiDataSql);
    std::vector<std::unique_ptr<Dic::Protocol::UnitTrack>> metaData;
    database.QueryHostMetadata(metaData);
    const uint64_t expectSize = 2;
    const uint64_t first = 0;
    const uint64_t second = 1;
    EXPECT_EQ(metaData.size(), expectSize);
    EXPECT_EQ(metaData[first]->metaData.processName, "process 288224");
    EXPECT_EQ(metaData[first]->children[first]->metaData.metaType, "CANN_API");
    EXPECT_EQ(metaData[first]->children[first]->metaData.threadId, "292753");
    EXPECT_EQ(metaData[first]->children[first]->children[first]->metaData.threadId, "");
    EXPECT_EQ(metaData[first]->children[first]->children[first]->metaData.processId, "1237912654215057");
    EXPECT_EQ(metaData[second]->metaData.processName, "process 3931124");
    EXPECT_EQ(metaData[second]->children[first]->metaData.metaType, "CANN_API");
    EXPECT_EQ(metaData[second]->children[first]->metaData.threadId, "3931572");
    EXPECT_EQ(metaData[second]->children[first]->children[first]->metaData.threadId, "MsTx");
    EXPECT_EQ(metaData[second]->children[first]->children[first]->metaData.processId, "16884049020452276");
}


TEST_F(DbDatabaseTest2, TestQuerySystemViewDataWhenDbNotOpen)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    const Dic::Protocol::SystemViewParams requestParams;
    Dic::Protocol::SystemViewBody responseBody;
    bool result = database.QuerySystemViewData(requestParams, responseBody);
    EXPECT_EQ(result, false);
}

TEST_F(DbDatabaseTest2, TestQuerySystemViewDataWhenSqlInject)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    database.SetDbPtr(db);
    Dic::Protocol::SystemViewParams requestParams;
    requestParams.orderBy = "####@";
    Dic::Protocol::SystemViewBody responseBody;
    bool result = database.QuerySystemViewData(requestParams, responseBody);
    EXPECT_EQ(result, false);
}

TEST_F(DbDatabaseTest2, TestQuerySystemViewDataWhenHardware)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, taskTableSql);
    DatabaseTestCaseMockUtil::CreateTable(db, computeSql);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    DatabaseTestCaseMockUtil::CreateTable(db, schedulerTable);
    database.SetDbPtr(db);
    Dic::Protocol::SystemViewParams requestParams;
    requestParams.orderBy = "name";
    requestParams.layer = "Ascend Hardware";
    Dic::Protocol::SystemViewBody responseBody;
    bool result = database.QuerySystemViewData(requestParams, responseBody);
    EXPECT_EQ(result, true);
}

TEST_F(DbDatabaseTest2, TestQuerySystemViewDataWhenHCCL)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, taskTableSql);
    DatabaseTestCaseMockUtil::CreateTable(db, commucationInfoSql);
    DatabaseTestCaseMockUtil::CreateTable(db, commucationOpSql);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    database.SetDbPtr(db);
    Dic::Protocol::SystemViewParams requestParams;
    requestParams.orderBy = "name";
    requestParams.layer = "HCCL";
    Dic::Protocol::SystemViewBody responseBody;
    bool result = database.QuerySystemViewData(requestParams, responseBody);
    EXPECT_EQ(result, true);
}

TEST_F(DbDatabaseTest2, TestQuerySystemViewDataWhenCANN)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, taskTableSql);
    DatabaseTestCaseMockUtil::CreateTable(db, canSql);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    database.SetDbPtr(db);
    Dic::Protocol::SystemViewParams requestParams;
    requestParams.orderBy = "name";
    requestParams.layer = "CANN";
    Dic::Protocol::SystemViewBody responseBody;
    bool result = database.QuerySystemViewData(requestParams, responseBody);
    EXPECT_EQ(result, true);
}

TEST_F(DbDatabaseTest2, TestQuerySystemViewDataWhenPython)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, taskTableSql);
    DatabaseTestCaseMockUtil::CreateTable(db, pytorchApiSql);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    database.SetDbPtr(db);
    Dic::Protocol::SystemViewParams requestParams;
    requestParams.orderBy = "name";
    requestParams.layer = "Python";
    Dic::Protocol::SystemViewBody responseBody;
    bool result = database.QuerySystemViewData(requestParams, responseBody);
    EXPECT_EQ(result, true);
}

TEST_F(DbDatabaseTest2, TestQuerySystemViewDataWhenOverlap)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, overlap);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    std::string overlapData = "INSERT INTO \"main\".\"OVERLAP_ANALYSIS\" (\"id\", \"deviceId\", \"startNs\", "
        "\"endNs\", \"type\") VALUES (5, 0, 1723510445657911820, 1723510445657974180, 1);";
    std::string sql = "CREATE TABLE RANK_DEVICE_MAP (rankId INTEGER, deviceId INTEGER);";
    DatabaseTestCaseMockUtil::CreateTable(db, sql);
    std::string insertSql = "INSERT INTO RANK_DEVICE_MAP (rankId, deviceId) VALUES (0, 0);";
    DatabaseTestCaseMockUtil::InsertData(db, insertSql);
    DatabaseTestCaseMockUtil::InsertData(db, overlapData);
    database.SetDbPtr(db);
    Dic::Protocol::SystemViewParams requestParams;
    requestParams.orderBy = "name";
    requestParams.layer = "Overlap Analysis";
    requestParams.searchName = "Communication";
    requestParams.rankId = "0";
    const uint64_t cur = 1;
    const uint64_t size = 100;
    requestParams.pageSize = size;
    requestParams.current = cur;
    Dic::Protocol::SystemViewBody responseBody;
    bool result = database.QuerySystemViewData(requestParams, responseBody);
    EXPECT_EQ(result, true);
}

TEST_F(DbDatabaseTest2, TestQueryFlowCategoryListWhenDbOpen)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    std::string table = "CREATE TABLE connectionCats (id INTEGER, cat TEXT);";
    std::string tableData = "INSERT INTO \"main\".\"connectionCats\" (\"id\", \"cat\") VALUES (1, '612484');";
    DatabaseTestCaseMockUtil::CreateTable(db, table);
    DatabaseTestCaseMockUtil::InsertData(db, tableData);
    database.SetDbPtr(db);
    std::vector<std::string> categories;
    const std::string rankId;
    bool result = database.QueryFlowCategoryList(categories, rankId);
    EXPECT_EQ(result, true);
}

TEST_F(DbDatabaseTest2, TestQueryCommunicationStatisticsData)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    const Dic::Protocol::SummaryStatisticParams requestParams;
    Dic::Protocol::SummaryStatisticsBody responseBody;
    bool result = database.QueryCommunicationStatisticsData(requestParams, responseBody);
    EXPECT_EQ(result, false);
}

TEST_F(DbDatabaseTest2, TestQueryCommunicationKernelInfoWhenDbOpen)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, commucationOpSql);
    DatabaseTestCaseMockUtil::CreateTable(db, commucationInfoSql);
    DatabaseTestCaseMockUtil::CreateTable(db, taskTableSql);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    std::string opData =
        "INSERT INTO \"main\".\"COMMUNICATION_OP\" (\"opName\", \"startNs\", \"endNs\", \"connectionId\", "
        "\"groupName\", \"opId\", \"relay\", \"retry\", \"dataType\", \"algType\", \"count\", \"opType\", \"waitNs\") "
        "VALUES (322, 1723510445656562660, 1723510445656625680, 149336, 324, 1, 0, 0, 5, 325, 8192, 326, 1412060);";
    std::string infoData = "INSERT INTO \"main\".\"COMMUNICATION_TASK_INFO\" (\"name\", \"globalTaskId\", "
        "\"taskType\", \"planeId\", \"groupName\", \"notifyId\", \"rdmaType\", \"srcRank\", "
        "\"dstRank\", \"transportType\", \"size\", \"dataType\", \"linkType\", \"opId\") VALUES "
        "(1, 6901, 323, 0, 324, 9223372036854775807, 65535, 0, 0, 0, 65536, 65535, 0, 1);";
    std::string taskData = "INSERT INTO \"main\".\"TASK\" (\"startNs\", \"endNs\", \"deviceId\", \"connectionId\", "
        "\"globalTaskId\", \"globalPid\", \"taskType\", \"contextId\", \"streamId\", \"taskId\", "
        "\"modelId\", \"depth\") VALUES (1723510445634242160, 1723510445634242160, 0, 4294967295, "
        "6901, 4130085, 293, 4294967295, 0, 39, 4294967295, 0);";
    std::string strData = "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (1, 'device');";
    std::string sql = "CREATE TABLE RANK_DEVICE_MAP (rankId INTEGER, deviceId INTEGER);";
    DatabaseTestCaseMockUtil::CreateTable(db, sql);
    std::string insertSql = "INSERT INTO RANK_DEVICE_MAP (rankId, deviceId) VALUES (0, 0);";
    DatabaseTestCaseMockUtil::InsertData(db, opData);
    DatabaseTestCaseMockUtil::InsertData(db, infoData);
    DatabaseTestCaseMockUtil::InsertData(db, taskData);
    DatabaseTestCaseMockUtil::InsertData(db, strData);
    DatabaseTestCaseMockUtil::InsertData(db, insertSql);
    database.SetDbPtr(db);
    const std::string name = "device";
    const std::string rankId = "0";
    Dic::Protocol::CommunicationKernelBody body;
    bool result = database.QueryCommunicationKernelInfo(name, rankId, body);
    EXPECT_EQ(result, true);
}

TEST_F(DbDatabaseTest2, TestQueryCommunicationKernelInfoWhenUniqueDevice)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, commucationOpSql);
    DatabaseTestCaseMockUtil::CreateTable(db, commucationInfoSql);
    DatabaseTestCaseMockUtil::CreateTable(db, taskTableSql);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    std::string opData =
        "INSERT INTO \"main\".\"COMMUNICATION_OP\" (\"opName\", \"startNs\", \"endNs\", \"connectionId\", "
        "\"groupName\", \"opId\", \"relay\", \"retry\", \"dataType\", \"algType\", \"count\", \"opType\", \"waitNs\") "
        "VALUES (1, 1723510445656562660, 1723510445656625680, 149336, 324, 1, 0, 0, 5, 325, 8192, 326, 1412060);";
    std::string taskData = "INSERT INTO \"main\".\"TASK\" (\"startNs\", \"endNs\", \"deviceId\", \"connectionId\", "
        "\"globalTaskId\", \"globalPid\", \"taskType\", \"contextId\", \"streamId\", \"taskId\", "
        "\"modelId\", \"depth\") VALUES (1723510445634242160, 1723510445634242160, 0, 4294967295, "
        "6901, 4130085, 293, 4294967295, 0, 39, 4294967295, 0);";
    std::string strData = "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (1, 'device');";
    std::string sql = "CREATE TABLE RANK_DEVICE_MAP (rankId INTEGER, deviceId INTEGER);";
    DatabaseTestCaseMockUtil::CreateTable(db, sql);
    std::string insertSql = "INSERT INTO RANK_DEVICE_MAP (rankId, deviceId) VALUES (0, 0);";
    DatabaseTestCaseMockUtil::InsertData(db, opData);
    DatabaseTestCaseMockUtil::InsertData(db, taskData);
    DatabaseTestCaseMockUtil::InsertData(db, strData);
    DatabaseTestCaseMockUtil::InsertData(db, insertSql);
    database.SetDbPtr(db);
    const std::string name = "device";
    const std::string rankId = "0";
    MockNpuInfoRepoFunc();
    Dic::Protocol::CommunicationKernelBody body;
    bool result = database.QueryCommunicationKernelInfo(name, rankId, body);
    EXPECT_EQ(result, true);
    EXPECT_EQ(body.depth, 0);
    EXPECT_EQ(body.pid, "HCCL");
    EXPECT_EQ(body.threadId, "324group");
    RestoreRepoFunc();
}

TEST_F(DbDatabaseTest2, TestQueryCommunicationKernelInfoWhenDbNotOpen)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    const std::string name = "device";
    const std::string rankId = "0";
    Dic::Protocol::CommunicationKernelBody body;
    bool result = database.QueryCommunicationKernelInfo(name, rankId, body);
    EXPECT_EQ(result, false);
}

TEST_F(DbDatabaseTest2, TestQueryHostInfoWhenTableIsWrong)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    std::string table = "CREATE TABLE HOST_INFO (hostUi INTEGER,hostame TEXT);";
    DatabaseTestCaseMockUtil::CreateTable(db, table);
    std::string data =
        "INSERT INTO \"main\".\"HOST_INFO\" (\"hostUi\", \"hostame\") VALUES (4973977386493930762, 'ubuntu2204');";
    DatabaseTestCaseMockUtil::InsertData(db, data);
    database.SetDbPtr(db);
    std::string result = database.QueryHostInfo();
    EXPECT_EQ(std::empty(result), true);
}

TEST_F(DbDatabaseTest2, TestQueryHostInfoWhenTimeTableIsExist)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    std::string table = "CREATE TABLE HOST_INFO (hostUid INTEGER,hostName TEXT);";
    std::string table2 = "CREATE TABLE SESSION_TIME_INFO (startTimeNs INTEGER,endTimeNs INTEGER);";
    DatabaseTestCaseMockUtil::CreateTable(db, table);
    DatabaseTestCaseMockUtil::CreateTable(db, table2);
    std::string data =
        "INSERT INTO \"main\".\"HOST_INFO\" (\"hostUid\", \"hostName\") VALUES (4973977386493930762, 'ubuntu2204');";
    std::string data2 = "INSERT INTO \"main\".\"SESSION_TIME_INFO\" (\"startTimeNs\", \"endTimeNs\") VALUES "
        "(1723510445508818000, 1723510450298869000);";
    DatabaseTestCaseMockUtil::InsertData(db, data);
    DatabaseTestCaseMockUtil::InsertData(db, data2);
    database.SetDbPtr(db);
    std::string result = database.QueryHostInfo();
    EXPECT_EQ(std::empty(result), false);
    Dic::Module::FullDb::CollectionTimeService::Instance().Reset();
}

TEST_F(DbDatabaseTest2, TestQueryFwdBwdDataByFlowWhenTableNotRight)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, pytorchApiSql);
    std::string connTable = "CREATE TABLE CONNECTION_IDS (id INTEGER, connectionId INTEGER);";
    std::string catTable = "CREATE TABLE connectionCats(connectionId INT,cat);";
    std::string apiTypeTable = "CREATE TABLE ENUM_API_TYPE (id INTEGER PRIMARY KEY,name TEXT);";
    DatabaseTestCaseMockUtil::CreateTable(db, connTable);
    DatabaseTestCaseMockUtil::CreateTable(db, catTable);
    DatabaseTestCaseMockUtil::CreateTable(db, apiTypeTable);
    database.SetDbPtr(db);
    const std::string rankId;
    const uint64_t offset = 0;
    const Dic::Protocol::ExtremumTimestamp range;
    std::vector<Dic::Protocol::ThreadTraces> fwdBwdData;
    bool result = database.QueryFwdBwdDataByFlow(rankId, offset, range, fwdBwdData);
    EXPECT_EQ(result, false);
}

TEST_F(DbDatabaseTest2, TestQueryP2PCommunicationOpDataWhenDbNotOpen)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    const std::string rankId;
    const uint64_t offset = 0;
    const Dic::Protocol::ExtremumTimestamp range;
    std::vector<Dic::Protocol::ThreadTraces> fwdBwdData;
    bool result = database.QueryP2PCommunicationOpData(rankId, offset, range, fwdBwdData);
    EXPECT_EQ(result, false);
}

TEST_F(DbDatabaseTest2, TestQueryConnectionIdWhenDbNotOpen)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    const Dic::Protocol::UnitFlowsParams requestParams;
    EXPECT_THROW(Dic::Protocol::TraceDatabaseHelper::QueryConnectionId(stmt, requestParams),
        Dic::Module::DatabaseException);
}

TEST_F(DbDatabaseTest2, TestQueryConnectionIdWhenHccl)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::UnitFlowsParams requestParams;
    requestParams.metaType = "HCCL";
    EXPECT_THROW(Dic::Protocol::TraceDatabaseHelper::QueryConnectionId(stmt, requestParams),
        Dic::Module::DatabaseException);
}

TEST_F(DbDatabaseTest2, TestQueryConnectionIdWhenCann)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::UnitFlowsParams requestParams;
    requestParams.metaType = "CANN_API";
    EXPECT_THROW(Dic::Protocol::TraceDatabaseHelper::QueryConnectionId(stmt, requestParams),
        Dic::Module::DatabaseException);
}

TEST_F(DbDatabaseTest2, TestQueryConnectionIdWhenApi)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::UnitFlowsParams requestParams;
    requestParams.metaType = "PYTORCH_API";
    EXPECT_THROW(Dic::Protocol::TraceDatabaseHelper::QueryConnectionId(stmt, requestParams),
        Dic::Module::DatabaseException);
}

TEST_F(DbDatabaseTest2, TestQueryConnectionIdWhenMstx)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::UnitFlowsParams requestParams;
    requestParams.metaType = "MSTX_EVENTS";
    EXPECT_THROW(Dic::Protocol::TraceDatabaseHelper::QueryConnectionId(stmt, requestParams),
        Dic::Module::DatabaseException);
}

TEST_F(DbDatabaseTest2, TestQueryUnitCounterWhenSoc)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::UnitCounterParams requestParams;
    requestParams.metaType = "SOC_BANDWIDTH_LEVEL";
    const uint64_t minTimestamp = 0;
    const std::string rankId;
    EXPECT_THROW(Dic::Protocol::TraceDatabaseHelper::QueryUnitCounter(stmt, requestParams, minTimestamp, rankId),
        Dic::Module::DatabaseException);
}

TEST_F(DbDatabaseTest2, TestQueryUnitCounterWhenAcc)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::UnitCounterParams requestParams;
    requestParams.metaType = "ACC_PMU";
    const uint64_t minTimestamp = 0;
    const std::string rankId;
    EXPECT_THROW(Dic::Protocol::TraceDatabaseHelper::QueryUnitCounter(stmt, requestParams, minTimestamp, rankId),
        Dic::Module::DatabaseException);
}

TEST_F(DbDatabaseTest2, TestQueryUnitCounterWhenNPU)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::UnitCounterParams requestParams;
    requestParams.metaType = "NPU_MEM";
    const uint64_t minTimestamp = 0;
    const std::string rankId;
    EXPECT_THROW(Dic::Protocol::TraceDatabaseHelper::QueryUnitCounter(stmt, requestParams, minTimestamp, rankId),
        Dic::Module::DatabaseException);
}

TEST_F(DbDatabaseTest2, TestQueryUnitCounterWhenSimple)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::UnitCounterParams requestParams;
    requestParams.metaType = "SAMPLE_PMU_TIMELINE";
    const uint64_t minTimestamp = 0;
    const std::string rankId;
    EXPECT_THROW(Dic::Protocol::TraceDatabaseHelper::QueryUnitCounter(stmt, requestParams, minTimestamp, rankId),
        Dic::Module::DatabaseException);
}

TEST_F(DbDatabaseTest2, TestQueryUnitCounterWhenRoce)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::UnitCounterParams requestParams;
    requestParams.metaType = "RoCE";
    const uint64_t minTimestamp = 0;
    const std::string rankId;
    EXPECT_THROW(Dic::Protocol::TraceDatabaseHelper::QueryUnitCounter(stmt, requestParams, minTimestamp, rankId),
        Dic::Module::DatabaseException);
}

TEST_F(DbDatabaseTest2, TestQueryUnitCounterWhenRoH)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::UnitCounterParams requestParams;
    requestParams.metaType = "RoH";
    const uint64_t minTimestamp = 0;
    const std::string rankId;
    EXPECT_THROW(Dic::Protocol::TraceDatabaseHelper::QueryUnitCounter(stmt, requestParams, minTimestamp, rankId),
        Dic::Module::DatabaseException);
}

TEST_F(DbDatabaseTest2, TestQueryUnitCounterWhenNIC)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::UnitCounterParams requestParams;
    requestParams.metaType = "NIC";
    const uint64_t minTimestamp = 0;
    const std::string rankId;
    EXPECT_THROW(Dic::Protocol::TraceDatabaseHelper::QueryUnitCounter(stmt, requestParams, minTimestamp, rankId),
        Dic::Module::DatabaseException);
}

TEST_F(DbDatabaseTest2, TestQueryUnitCounterWhenHCCS)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::UnitCounterParams requestParams;
    requestParams.metaType = "HCCS";
    const uint64_t minTimestamp = 0;
    const std::string rankId;
    EXPECT_THROW(Dic::Protocol::TraceDatabaseHelper::QueryUnitCounter(stmt, requestParams, minTimestamp, rankId),
        Dic::Module::DatabaseException);
}

TEST_F(DbDatabaseTest2, TestQueryUnitCounterWhenPCIE)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::UnitCounterParams requestParams;
    requestParams.metaType = "PCIE";
    const uint64_t minTimestamp = 0;
    const std::string rankId;
    EXPECT_THROW(Dic::Protocol::TraceDatabaseHelper::QueryUnitCounter(stmt, requestParams, minTimestamp, rankId),
        Dic::Module::DatabaseException);
}

TEST_F(DbDatabaseTest2, TestQueryUnitCounterWhenAICORE)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::UnitCounterParams requestParams;
    requestParams.metaType = "AICORE_FREQ";
    const uint64_t minTimestamp = 0;
    const std::string rankId;
    EXPECT_THROW(Dic::Protocol::TraceDatabaseHelper::QueryUnitCounter(stmt, requestParams, minTimestamp, rankId),
        Dic::Module::DatabaseException);
}

TEST_F(DbDatabaseTest2, TestQueryThreadsByPidWhenApi)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    const uint64_t startTime = 0;
    const uint64_t endTime = 0;
    Dic::Protocol::Metadata metaData;
    metaData.metaType = "PYTORCH_API";
    const std::string rankId;
    EXPECT_THROW(Dic::Protocol::TraceDatabaseHelper::QueryThreadsByPid(stmt, startTime, endTime, metaData, rankId),
        Dic::Module::DatabaseException);
}

TEST_F(DbDatabaseTest2, TestQueryThreadsByPidWhenOVERLAP)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    const uint64_t startTime = 0;
    const uint64_t endTime = 0;
    Dic::Protocol::Metadata metaData;
    metaData.metaType = "OVERLAP_ANALYSIS";
    const std::string rankId;
    EXPECT_THROW(Dic::Protocol::TraceDatabaseHelper::QueryThreadsByPid(stmt, startTime, endTime, metaData, rankId),
        Dic::Module::DatabaseException);
}

TEST_F(DbDatabaseTest2, TestQueryThreadsByPidWhenMstx)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    const uint64_t startTime = 0;
    const uint64_t endTime = 0;
    Dic::Protocol::Metadata metaData;
    metaData.metaType = "MSTX_EVENTS";
    const std::string rankId;
    EXPECT_THROW(Dic::Protocol::TraceDatabaseHelper::QueryThreadsByPid(stmt, startTime, endTime, metaData, rankId),
        Dic::Module::DatabaseException);
}

TEST_F(DbDatabaseTest2, TestQueryEventsView4PytorchWhenApi)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::EventsViewParams params;
    params.metaType = "PYTORCH_API";
    Dic::Protocol::EventsViewBody body;
    const uint64_t minTimestamp = 0;
    const std::string rankId;
    bool res = Dic::Protocol::TraceDatabaseHelper::QueryEventsViewData4Db(stmt, params, body, minTimestamp, rankId);
    EXPECT_EQ(res, false);
}

TEST_F(DbDatabaseTest2, TestQueryEventsView4PytorchWhenHardWare)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::EventsViewParams params;
    params.metaType = "Ascend Hardware";
    Dic::Protocol::EventsViewBody body;
    const uint64_t minTimestamp = 0;
    const std::string rankId;
    bool res = Dic::Protocol::TraceDatabaseHelper::QueryEventsViewData4Db(stmt, params, body, minTimestamp, rankId);
    EXPECT_EQ(res, false);
}

TEST_F(DbDatabaseTest2, TestQueryEventsView4PytorchWhenHardWareAndTidIsNotEmpty)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::EventsViewParams params;
    params.metaType = "Ascend Hardware";
    params.tid = "ppp";
    Dic::Protocol::EventsViewBody body;
    const uint64_t minTimestamp = 0;
    const std::string rankId;
    bool res = Dic::Protocol::TraceDatabaseHelper::QueryEventsViewData4Db(stmt, params, body, minTimestamp, rankId);
    EXPECT_EQ(res, false);
}

TEST_F(DbDatabaseTest2, TestQueryEventsView4PytorchWhenHccl)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::EventsViewParams params;
    params.metaType = "HCCL";
    Dic::Protocol::EventsViewBody body;
    const uint64_t minTimestamp = 0;
    const std::string rankId;
    bool res = Dic::Protocol::TraceDatabaseHelper::QueryEventsViewData4Db(stmt, params, body, minTimestamp, rankId);
    EXPECT_EQ(res, false);
}

TEST_F(DbDatabaseTest2, TestQueryEventsView4PytorchWhenHcclAndTidNotEmpty)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::EventsViewParams params;
    params.metaType = "HCCL";
    params.tid = "lllll";
    Dic::Protocol::EventsViewBody body;
    const uint64_t minTimestamp = 0;
    const std::string rankId;
    bool res = Dic::Protocol::TraceDatabaseHelper::QueryEventsViewData4Db(stmt, params, body, minTimestamp, rankId);
    EXPECT_EQ(res, false);
}

TEST_F(DbDatabaseTest2, TestQueryEventsView4PytorchWhenOverlap)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::EventsViewParams params;
    params.metaType = "OVERLAP_ANALYSIS";
    Dic::Protocol::EventsViewBody body;
    const uint64_t minTimestamp = 0;
    const std::string rankId;
    bool res = Dic::Protocol::TraceDatabaseHelper::QueryEventsViewData4Db(stmt, params, body, minTimestamp, rankId);
    EXPECT_EQ(res, false);
}

TEST_F(DbDatabaseTest2, TestQueryEventsView4PytorchWhenOverlapAndTidNotEmpty)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::EventsViewParams params;
    params.metaType = "OVERLAP_ANALYSIS";
    params.tid = "kkkkkkkk";
    Dic::Protocol::EventsViewBody body;
    const uint64_t minTimestamp = 0;
    const std::string rankId;
    bool res = Dic::Protocol::TraceDatabaseHelper::QueryEventsViewData4Db(stmt, params, body, minTimestamp, rankId);
    EXPECT_EQ(res, false);
}

TEST_F(DbDatabaseTest2, TestQueryEventsView4PytorchWhenOther)
{
    std::recursive_mutex testMutex;
    MockDatabase database(testMutex);
    auto stmt = database.CreatPreparedStatement();
    Dic::Protocol::EventsViewParams params;
    params.metaType = "unknown";
    params.tid = "kkkkkkkk";
    Dic::Protocol::EventsViewBody body;
    const uint64_t minTimestamp = 0;
    const std::string rankId;
    bool res = Dic::Protocol::TraceDatabaseHelper::QueryEventsViewData4Db(stmt, params, body, minTimestamp, rankId);
    EXPECT_EQ(res, false);
}

TEST_F(DbDatabaseTest2, TestIsValidGroupNameValueSuccess)
{
    const std::string groupNameValue = "10.170.22.98%enp67s0f5_60000_0_1708156014257149";
    const bool res = Dic::Protocol::TraceDatabaseHelper::IsValidHCCLGroupNameValue(groupNameValue);
    EXPECT_EQ(res, true);
}

TEST_F(DbDatabaseTest2, TestIsValidGroupNameValueFail)
{
    const std::string groupNameValue = "0";
    const bool res = Dic::Protocol::TraceDatabaseHelper::IsValidHCCLGroupNameValue(groupNameValue);
    EXPECT_EQ(res, false);
}