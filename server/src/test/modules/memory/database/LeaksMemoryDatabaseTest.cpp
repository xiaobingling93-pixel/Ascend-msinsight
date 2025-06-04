/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "MemoryProtocolRequest.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "LeaksMemoryDatabase.h"
#include "LeaksMemoryService.h"
#include "../../../TestSuit.cpp"

using namespace Dic::Module::Timeline;
using namespace Dic::Module::FullDb;
using namespace Dic;

class LeaksMemoryDatabaseTest : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        std::string currPath = Dic::FileUtil::GetCurrPath();
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        std::string dbPath3 = R"(/src/test/test_data/full_db/)";
        DataBaseManager::Instance().SetDataType(DataType::DB);
        DataBaseManager::Instance().SetFileType(FileType::LEAKS);
        auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("0");
        ASSERT_TRUE(memoryDatabase->OpenDb(currPath + dbPath3 + "leaks_dump_2025.dat", false));
        ASSERT_TRUE(memoryDatabase->DropMemoryAllocationAndBlockTable());
        ASSERT_TRUE(LeaksMemoryService::ParseMemoryLeaksDumpEvents("0"));
    }
    static void TearDownTestSuite()
    {
        auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("0");
        memoryDatabase->CloseDb();
        DataBaseManager::Instance().Clear();
    }
};

TEST_F(LeaksMemoryDatabaseTest, QueryEntireEventsTable)
{
    auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("0");
    std::vector<MemoryEvent> events;
    memoryDatabase->QueryEntireEventsTable(events);
    size_t expectSize = 15128;
    EXPECT_EQ(events.size(), expectSize);
}

TEST_F(LeaksMemoryDatabaseTest, QueryDeviceIds)
{
    auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("0");
    std::set<std::string> deviceIds;
    memoryDatabase->QueryDeviceIds(deviceIds);
    size_t expectSize = 3;
    EXPECT_EQ(deviceIds.size(), expectSize);
}
TEST_F(LeaksMemoryDatabaseTest, QueryDeviceEventTypeMap)
{
    auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("0");
    std::unordered_map<std::string, std::vector<std::string>> resultMap;
    memoryDatabase->QueryMallocOrFreeEventTypeWithDeviceId(resultMap);
    std::vector<std::string> eventTypes = resultMap["0"];
    auto it = std::find(eventTypes.begin(), eventTypes.end(), "HAL");
    EXPECT_TRUE(it != eventTypes.end());
    it = std::find(eventTypes.begin(), eventTypes.end(), "PTA");
    EXPECT_TRUE(it != eventTypes.end());
    eventTypes = resultMap["1"];
    it = std::find(eventTypes.begin(), eventTypes.end(), "HAL");
    EXPECT_TRUE(it != eventTypes.end());
    it = std::find(eventTypes.begin(), eventTypes.end(), "PTA");
    EXPECT_TRUE(it != eventTypes.end());
    eventTypes = resultMap["host"];
    it = std::find(eventTypes.begin(), eventTypes.end(), "HAL");
    EXPECT_TRUE(it != eventTypes.end());
    it = std::find(eventTypes.begin(), eventTypes.end(), "PTA");
    EXPECT_TRUE(it == eventTypes.end());
}
TEST_F(LeaksMemoryDatabaseTest, QueryMinAndMaxTimestamp)
{
    auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("0");
    uint64_t minTimestamp = memoryDatabase->QueryMemoryEventExtremumTimestamp("0", true);
    uint64_t maxTimestamp = memoryDatabase->QueryMemoryEventExtremumTimestamp("0", false);
    const uint64_t expectMin = 1746671067160936;
    const uint64_t expectMax = 1746671087511852;
    EXPECT_EQ(minTimestamp, expectMin);
    EXPECT_EQ(maxTimestamp, expectMax);
}

TEST_F(LeaksMemoryDatabaseTest, QueryMemoryBlockWithNoTimeAndSizeCondition)
{
    std::vector<MemoryEvent> events;
    auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("0");
    LeaksMemoryBlockParams params;
    params.deviceId = "0";
    params.relativeTime = false;
    params.eventType = "PTA";
    std::vector<MemoryBlock> blocks;
    memoryDatabase->QueryMemoryBlocks(params, blocks);
    const size_t expectSize = 2270;
    EXPECT_EQ(blocks.size(), expectSize);
}
TEST_F(LeaksMemoryDatabaseTest, QueryMemoryBlockWithTimeAndSizeConditionAndRelativeTime)
{
    LeaksMemoryBlockParams params;
    params.deviceId = "0";
    params.relativeTime = true;
    const uint64_t endTimestamp = 20 * 1000 * 1000;
    const uint64_t maxSize = 600;
    params.endTimestamp = endTimestamp;
    params.maxSize = maxSize;
    params.eventType = "PTA";
    std::vector<MemoryBlock> blocks;
    auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("0");
    memoryDatabase->QueryMemoryBlocks(params, blocks);
    uint64_t minTimestamp = memoryDatabase->QueryMemoryEventExtremumTimestamp("0", true);
    const size_t expectSize = 956;
    EXPECT_EQ(blocks.size(), expectSize);
    MemoryBlock &firstBlock = blocks[0];
    const uint64_t expectStartTime = 1746671073689685 - minTimestamp;
    const uint64_t expectEndTime = 1746671076330707 - minTimestamp;
    EXPECT_EQ(firstBlock.startTimestamp, expectStartTime);
    EXPECT_EQ(firstBlock.endTimestamp, expectEndTime);
    EXPECT_TRUE(firstBlock.size > 0 and firstBlock.size < params.maxSize);
}
TEST_F(LeaksMemoryDatabaseTest, QueryAllocationWithNoTimeCondition)
{
    LeaksMemoryAllocationParams params;
    params.deviceId = "0";
    params.optimized = false;
    params.eventType = "PTA";
    std::vector<MemoryAllocation> allocations;
    auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("0");
    memoryDatabase->QueryMemoryAllocations(params, allocations);
    size_t expectSize = 4540;
    EXPECT_EQ(allocations.size(), expectSize);
}
TEST_F(LeaksMemoryDatabaseTest, QueryAllocationWithTimeAndRelativeCondition)
{
    LeaksMemoryAllocationParams params;
    params.deviceId = "0";
    params.optimized = false;
    const uint64_t endTimestamp = 20 * 1000 * 1000;
    params.endTimestamp = endTimestamp;
    params.relativeTime = true;
    params.eventType = "PTA";
    std::vector<MemoryAllocation> allocations;
    auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("0");
    memoryDatabase->QueryMemoryAllocations(params, allocations);
    uint64_t minTimestamp = memoryDatabase->QueryMemoryEventExtremumTimestamp("0", true);
    const size_t expectSize = 4331;
    EXPECT_EQ(allocations.size(), expectSize);
    MemoryAllocation &firstAllocation = allocations[0];
    const uint64_t expectTimestamp = 6379153;
    EXPECT_EQ(firstAllocation.timestamp, expectTimestamp);
    uint64_t totalSize = 7168;
    EXPECT_EQ(firstAllocation.totalSize, totalSize);
}

