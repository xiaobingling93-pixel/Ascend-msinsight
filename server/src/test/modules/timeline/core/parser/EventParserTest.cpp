/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "TextTraceDatabase.h"
#include "../../../defaultMock/MockFileReader.h"
#include "EventParser.h"
#include "../../../../DatabaseTestCaseMockUtil.cpp"
#include "SliceTable.h"
#include "ParserStatusManager.h"
#include "SimulationSliceCacheManager.h"
#include "TrackInfoManager.h"
using ::testing::ByMove;
using ::testing::Return;
using namespace Dic::Module::Timeline;
class EventParserTest : public ::testing::Test {
protected:
    class MockDatabase : public TextTraceDatabase {
    public:
        explicit MockDatabase(std::recursive_mutex &sqlMutex) : TextTraceDatabase(sqlMutex) {}
        void SetDbPtr(sqlite3 *dbPtr)
        {
            isOpen = true;
            db = dbPtr;
            path = ":memory:";
        }
        ~MockDatabase() override
        {
            isOpen = false;
        }
    };
    const int64_t startPosition = 12;
    const int64_t endPosition = 98;
    const std::string filePath = "hhh";
    const std::string fileId = "lll";
    std::string sliceTableSql =
        "CREATE TABLE slice (id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp INTEGER, duration INTEGER, name TEXT, "
        "depth INTEGER, track_id INTEGER, cat TEXT, args TEXT, cname TEXT, end_time INTEGER, flag_id TEXT);";
    std::unique_ptr<MockFileReader> mockFileReader = std::make_unique<MockFileReader>();
    std::recursive_mutex sqlMutex;
    std::shared_ptr<MockDatabase> mockDatabase = std::make_unique<MockDatabase>(sqlMutex);
    void SetUp() override
    {
        ParserStatusManager::Instance().ClearAllParserStatus();
        SimulationSliceCacheManager::Instance().ClearAll();
        TrackInfoManager::Instance().Reset();
    }

    void TearDown() override
    {
        ParserStatusManager::Instance().ClearAllParserStatus();
        SimulationSliceCacheManager::Instance().ClearAll();
        TrackInfoManager::Instance().Reset();
    }
};

class EventParserMock : public EventParser {
public:
    EventParserMock(const std::string &filePath, const std::string &fileId,
        std::shared_ptr<TextTraceDatabase> textDatabase)
        : EventParser(filePath, fileId, textDatabase){};
    void SetFileReaderAndDatabase(std::unique_ptr<MockFileReader> fileReaderPtr)
    {
        fileReader = std::move(fileReaderPtr);
    }
};

/**
 * 测试解析type为s的json
 */
TEST_F(EventParserTest, TestSliceParse)
{
    std::string jsonContent = "[{\"name\": \"PROFILING_DISABLE\", \"pid\": 2094647552, \"tid\": 0, \"ts\": "
        "\"1718180920003154.317\", \"dur\": "
        "0, \"args\": {\"Model Id\": 4294967295, \"Task Type\": \"PLACE_HOLDER_SQE\", \"Physic Stream Id\": 0, \"Task "
        "Id\": 17, \"Batch Id\": 0, \"Subtask Id\": 4294967295, \"connection_id\": -1}, \"ph\": \"X\"}]";
    ParserStatusManager::Instance().SetParserStatus(fileId, ParserStatus::RUNNING);
    EXPECT_CALL(*mockFileReader, ReadJsonArray(filePath, startPosition, endPosition)).WillOnce(Return(jsonContent));
    sqlite3 *dbPtr = nullptr;
    Dic::Global::PROFILER::MockUtil::DatabaseTestCaseMockUtil::OpenDB(dbPtr);
    mockDatabase->SetDbPtr(dbPtr);
    mockDatabase->CreateTable();
    EventParserMock eventParserMock(filePath, fileId, mockDatabase);
    eventParserMock.SetFileReaderAndDatabase(std::move(mockFileReader));
    eventParserMock.Parse(startPosition, endPosition);
    Dic::Module::Timeline::SliceTable sliceTable;
    std::vector<SlicePO> slicePOs;
    sliceTable.Select(SliceColumn::ID, SliceColumn::TIMESTAMP)
        .Select(SliceColumn::DURATION, SliceColumn::NAME)
        .Select(SliceColumn::TRACKID)
        .Select(SliceColumn::ENDTIME, SliceColumn::ARGS)
        .ExcuteQuery(dbPtr, slicePOs);
    const uint64_t expectSize = 1;
    const uint64_t first = 0;
    const uint64_t exceptTime = 1718180920003154317;
    const uint64_t exceptDuration = 0;
    const uint64_t exceptTrackId = 1;
    const std::string name = "PROFILING_DISABLE";
    EXPECT_EQ(slicePOs.size(), expectSize);
    EXPECT_EQ(slicePOs[first].timestamp, exceptTime);
    EXPECT_EQ(slicePOs[first].duration, exceptDuration);
    EXPECT_EQ(slicePOs[first].name, name);
    EXPECT_EQ(slicePOs[first].trackId, exceptTrackId);
    EXPECT_EQ(slicePOs[first].endTime, exceptTime + exceptDuration);
    EXPECT_EQ(slicePOs[first].args.empty(), false);
}
