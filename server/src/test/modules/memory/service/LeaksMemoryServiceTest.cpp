/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include <gtest/gtest.h>
#include "MemoryDetailRequestHandler.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "LeaksMemoryDatabase.h"
#include "../../../TestSuit.cpp"
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
        std::string fullDbPath = StringUtil::StrJoin(currPath, dbPath3, "leaks_dump_2025.dat");
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
    const size_t expectEventsSize = 6306;
    EXPECT_EQ(events.size(), expectEventsSize);
    EXPECT_TRUE(memoryDatabase->DropMemoryAllocationAndBlockTable());
    EXPECT_TRUE(memoryDatabase->CreateMemoryAllocationAndBlockTable());
    const size_t expectBlockSize = 468;
    const size_t expectAllocationSize = 936;
    LeaksMemoryService::ParseEventsToBlockAndAllocations(events, memoryDatabase);
    // 校验blocks
    LeaksMemoryBlockParams blockParams;
    blockParams.deviceId = "0";
    blockParams.relativeTime = false;
    blockParams.eventType = "PTA";
    std::vector<MemoryBlock> blocks;
    memoryDatabase->QueryMemoryBlocks(blockParams, blocks);
    // 校验allocations
    LeaksMemoryAllocationParams allocationParams;
    allocationParams.deviceId = "0";
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
    BlockEventAttr eventAttr;
    eventAttr.size = 0;
    LeaksMemoryService::BuildBlockEventAttrFromEvent(event, eventAttr);
    EXPECT_EQ(eventAttr.size, 0);
    EXPECT_EQ(eventAttr.addr, "");
    EXPECT_EQ(eventAttr.owner, "");
}

TEST_F(LeaksMemoryServiceTest, BuildBlockEventAttrFromEventWithInvalidJsonAttr)
{
    MemoryEvent event;
    event.attr = "{]";
    BlockEventAttr eventAttr;
    eventAttr.size = 0;
    LeaksMemoryService::BuildBlockEventAttrFromEvent(event, eventAttr);
    EXPECT_EQ(eventAttr.size, 0);
    EXPECT_EQ(eventAttr.addr, "");
    EXPECT_EQ(eventAttr.owner, "");
}

TEST_F(LeaksMemoryServiceTest, BuildBlockEventAttrFromEventWithValidJsonAttr)
{
    MemoryEvent event;
    event.attr = R"({"addr": "20617055174656", "size": "7849984", "total": "132120576",
"used": "116313600", "owner": "PTA@init_model"})";
    BlockEventAttr eventAttr;
    eventAttr.size = 0;
    LeaksMemoryService::BuildBlockEventAttrFromEvent(event, eventAttr);
    const int64_t expectSize = 7849984;
    EXPECT_EQ(eventAttr.size, expectSize);
    EXPECT_EQ(eventAttr.addr, "20617055174656");
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
    const uint64_t threadId = 3841316;
    LeaksMemoryThreadPythonTraceParams params;
    params.startTimestamp = startTimestamp;
    params.endTimestamp = endTimestamp;
    params.relativeTime = true;
    params.threadId = threadId;
    params.deviceId = '0';
    LeaksMemoryPythonTrace trace;
    auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("0");
    ASSERT_TRUE(memoryDatabase != nullptr);
    memoryDatabase->QueryPythonTrace(params, trace);
    const size_t expectSize = 2017;
    const uint64_t expectMinTimestamp = 11358527770;
    const uint64_t expectMaxTimestamp = 39780754520;
    EXPECT_EQ(trace.slices.size(), expectSize);
    LeaksMemoryService::ParseThreadPythonTrace(trace);
    EXPECT_EQ(trace.minTimestamp, expectMinTimestamp);
    EXPECT_EQ(trace.maxTimestamp, expectMaxTimestamp);
}
