/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include "DatabaseTest.cpp"
#include "DbTraceDataBase.h"
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
    const std::string expectGroupName = "Group 1 Communication";
    const std::string expectPlaneName = "Plane 0";
    database.QueryUnitsMetadata(fileId, metaData);
    EXPECT_EQ(metaData.size(), expectProcessCount);
    EXPECT_EQ(metaData[second]->children.size(), three);
    EXPECT_EQ(metaData[second]->children[first]->metaData.threadName, expectGroupName);
    EXPECT_EQ(metaData[second]->children[second]->metaData.threadName, expectPlaneName);
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
    const std::string expectGroupName = "Group 1 Communication";
    database.QueryUnitsMetadata(fileId, metaData);
    EXPECT_EQ(metaData.size(), expectProcessCount);
    EXPECT_EQ(metaData[second]->children.size(), second);
    EXPECT_EQ(metaData[second]->children[first]->metaData.threadName, expectGroupName);
}