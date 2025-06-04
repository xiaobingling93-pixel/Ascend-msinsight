/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include <gtest/gtest.h>
#include "MemoryProtocolRequest.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "LeaksMemoryDatabase.h"
#include "../../../TestSuit.cpp"
#include "LeaksMemoryService.h"

using namespace Dic::Module::Timeline;
using namespace Dic::Module::FullDb;
using namespace Dic;

class LeaksMemoryServiceTest : public ::testing::Test {
public:
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
    const size_t expectEventsSize = 15128;
    EXPECT_EQ(events.size(), expectEventsSize);
    EXPECT_TRUE(memoryDatabase->DropMemoryAllocationAndBlockTable());
    EXPECT_TRUE(memoryDatabase->CreateMemoryAllocationAndBlockTable());
    const size_t expectBlockSize = 2270;
    const size_t expectAllocationSize = 4540;
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

TEST_F(LeaksMemoryServiceTest, BuildBlockEventAttrFromEventWithJsonAttrNotContainSizeOrOwner)
{
    MemoryEvent event;
    event.attr = R"({"addr":"123"})";
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
    event.attr = R"({"size":"123","owner":"","addr":"123"})";
    BlockEventAttr eventAttr;
    eventAttr.size = 0;
    LeaksMemoryService::BuildBlockEventAttrFromEvent(event, eventAttr);
    const int64_t expectSize = 123;
    EXPECT_EQ(eventAttr.size, expectSize);
    EXPECT_EQ(eventAttr.addr, "123");
    EXPECT_EQ(eventAttr.owner, "");
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
