/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include <gtest/gtest.h>
#include "MemoryDetailRequestHandler.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "LeaksMemoryDatabase.h"
#include "../../TestSuit.cpp"
#include "LeaksMemoryService.h"

using namespace Dic::Module::Timeline;
using namespace Dic::Module::FullDb;
using namespace Dic::Module::MemoryDetail;
using namespace Dic;

class LeaksMemoryServiceTest : public ::testing::Test {
public:
    const static uint64_t SECOND = 1000000000;
    static void SetUpTestSuite()
    {
        std::string currPath = Dic::FileUtil::GetCurrPath();
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        std::string dbPath3 = R"(/src/test/test_data/full_db/)";
        DataBaseManager::Instance().SetDataType(DataType::DB);
        DataBaseManager::Instance().SetFileType(FileType::LEAKS);
        std::string fullDbPath = StringUtil::StrJoin(currPath, dbPath3, "leaks_dump_20250806.dat");
        auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("0");
        ASSERT_TRUE(memoryDatabase->OpenDb(fullDbPath, false));
    }
    static void TearDownTestSuite()
    {
        auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("0");
        memoryDatabase->CloseDb();
        DataBaseManager::Instance().Clear();
    }
};

TEST_F(LeaksMemoryServiceTest, ParseEventsToAllocationsAndBlocks)
{
    auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("0");
    std::vector<MemoryEvent> events;
    memoryDatabase->QueryEntireEventsTable(events);
    const size_t expectEventsSize = 33882;
    EXPECT_EQ(events.size(), expectEventsSize);
    EXPECT_TRUE(memoryDatabase->DropMemoryAllocationAndBlockTable());
    EXPECT_TRUE(memoryDatabase->CreateMemoryAllocationAndBlockTable());
    const size_t expectBlockSize = 3267;
    const size_t expectAllocationSize = 6534;
    auto context = LeaksMemoryService::BuildContext(memoryDatabase);
    EXPECT_TRUE(context.has_value());
    LeaksMemoryService::ParseEventsToBlockAndAllocations(*context);
    // 校验blocks
    LeaksMemoryBlockParams blockParams;
    blockParams.deviceId = "1";
    blockParams.relativeTime = false;
    blockParams.eventType = "PTA";
    std::vector<MemoryBlock> blocks;
    memoryDatabase->QueryMemoryBlocks(blockParams, false, blocks);
    // 校验allocations
    LeaksMemoryAllocationParams allocationParams;
    allocationParams.deviceId = "1";
    allocationParams.optimized = false;
    allocationParams.eventType = "PTA";
    std::vector<MemoryAllocation> allocations;
    memoryDatabase->QueryMemoryAllocations(allocationParams, allocations);
    EXPECT_EQ(blocks.size(), expectBlockSize);
    EXPECT_EQ(allocations.size(), expectAllocationSize);
}

TEST_F(LeaksMemoryServiceTest, BuildBlockEventAttrFromEventWithEmptyAttr)
{
    MemoryEvent event;
    event.attr = "";
    MemoryEventAttrs eventAttr;
    eventAttr.size = 0;
    LeaksMemoryService::BuildEventAttrs(event, eventAttr);
    EXPECT_EQ(eventAttr.size, 0);
    EXPECT_EQ(eventAttr.owner, "");
}

TEST_F(LeaksMemoryServiceTest, BuildBlockEventAttrFromEventWithInvalidJsonAttr)
{
    MemoryEvent event;
    event.attr = "{]";
    MemoryEventAttrs eventAttr;
    eventAttr.size = 0;
    LeaksMemoryService::BuildEventAttrs(event, eventAttr);
    EXPECT_EQ(eventAttr.size, 0);
    EXPECT_EQ(eventAttr.owner, "");
}

TEST_F(LeaksMemoryServiceTest, BuildBlockEventAttrFromEventWithValidJsonAttr)
{
    MemoryEvent event;
    event.attr = R"({"addr": "20617055174656", "size": "7849984", "total": "132120576",
"used": "116313600", "owner": "PTA@init_model"})";
    MemoryEventAttrs eventAttr;
    eventAttr.size = 0;
    LeaksMemoryService::BuildEventAttrs(event, eventAttr);
    const int64_t expectSize = 7849984;
    EXPECT_EQ(eventAttr.size, expectSize);
    EXPECT_EQ(eventAttr.owner, "PTA@init_model");
}

TEST_F(LeaksMemoryServiceTest, ParserEnd)
{
    EXPECT_NO_THROW(LeaksMemoryService::ParserEnd("0", false));
    EXPECT_NO_THROW(LeaksMemoryService::ParserEnd("0", true));
}

TEST_F(LeaksMemoryServiceTest, ParseCallBack)
{
    EXPECT_NO_THROW(LeaksMemoryService::ParseCallBack("", false, ""));
    EXPECT_NO_THROW(LeaksMemoryService::ParseCallBack("0", false, ""));
    EXPECT_NO_THROW(LeaksMemoryService::ParseCallBack("0", true, ""));
}

TEST_F(LeaksMemoryServiceTest, ParseLeaksDump)
{
    auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("0");
    EXPECT_TRUE(memoryDatabase->DropMemoryAllocationAndBlockTable());
    EXPECT_TRUE(memoryDatabase->CreateMemoryAllocationAndBlockTable());
    memoryDatabase->UpdateParseStatus(NOT_FINISH_STATUS);
    EXPECT_TRUE(LeaksMemoryService::ParseMemoryLeaksDumpEvents("0"));
}

TEST_F(LeaksMemoryServiceTest, ParseMemoryAllocDetailTreeByTimestamp)
{
    auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("0");
    ASSERT_TRUE(memoryDatabase != nullptr);
    LeaksMemoryDetailTreeNode tree;
    const std::string deviceId = "7";
    const std::string eventType = "PTA";
    const uint64_t expectDuration = 1000000;
    LeaksMemoryService::ParseMemoryAllocDetailTreeByTimestamp(deviceId, expectDuration, eventType, tree, true);
    ServerLog::Error(tree.tag);
}

TEST_F(LeaksMemoryServiceTest, ParseThreadPythonTraceInTimeRange)
{
    const uint64_t startTimestamp = 1000000;
    const uint64_t durationSeconds = 15;
    const uint64_t endTimestamp = startTimestamp + durationSeconds * SECOND;
    const uint64_t threadId = 2621226;
    LeaksMemoryThreadPythonTraceParams params;
    params.startTimestamp = startTimestamp;
    params.endTimestamp = endTimestamp;
    params.relativeTime = true;
    params.threadId = threadId;
    params.deviceId = '1';
    LeaksMemoryPythonTrace trace;
    auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("0");
    ASSERT_TRUE(memoryDatabase != nullptr);
    memoryDatabase->QueryPythonTrace(params, trace);
    const size_t expectSize = 50;
    const uint64_t expectMinTimestamp = 9324668200;
    const uint64_t expectMaxTimestamp = 39310611890;
    EXPECT_EQ(trace.slices.size(), expectSize);
    LeaksMemoryService::ParseThreadPythonTrace(trace);
    EXPECT_EQ(trace.minTimestamp, expectMinTimestamp);
    EXPECT_EQ(trace.maxTimestamp, expectMaxTimestamp);
}

/***
 * 测试解析出的内存块数据首末次访问事件是否正确
 */
TEST_F(LeaksMemoryServiceTest, TestMemoryBlockFirstLastAccessTimestamp)
{
    auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("0");
    EXPECT_TRUE(memoryDatabase != nullptr);
    const uint64_t groupId = 1910;
    std::vector<MemoryEvent> events;
    memoryDatabase->QueryEventsByGroupId(groupId, "1", true, events);
    EventGroup eventGroup(events);
    EXPECT_FALSE(events.empty());
    auto firstEvent = events.front();
    // 根据首次事件的deviceId、eventType和timestamp查询出对应解析出的内存块
    std::vector<MemoryBlock> blocks;
    LeaksMemoryBlockParams queryParams;
    queryParams.deviceId = firstEvent.deviceId;
    queryParams.eventType = firstEvent.eventType;
    queryParams.relativeTime = true;
    queryParams.startTimestamp = firstEvent.timestamp - 1;
    queryParams.endTimestamp = firstEvent.timestamp + 1;
    // 仅查询在时间范围内申请或释放的，理应只有一个内存块
    memoryDatabase->QueryMemoryBlocks(queryParams, false, blocks);
    EXPECT_EQ(blocks.size(), 1);
    MemoryBlock block = blocks[0];
    auto blockAttrs = MemoryBlockAttrs::FromJson(block.attrJsonString);
    EXPECT_TRUE(blockAttrs.has_value());
    EXPECT_EQ(blockAttrs->firstAccessTimestamp, eventGroup.accessEvents.front().timestamp);
    EXPECT_EQ(blockAttrs->lastAccessTimestamp, eventGroup.accessEvents.back().timestamp);
}