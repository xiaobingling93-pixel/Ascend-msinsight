/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include <WsSessionManager.h>
#include "WsSessionImpl.h"
#include "MemoryDetailProtocolRequest.h"
#include "LeaksMemoryService.h"
#include "QueryLeaksMemoryAllocationHandler.h"
#include "QueryLeaksMemoryBlockHandler.h"
#include "QueryLeaksMemoryDetailHandler.h"
#include "QueryLeaksMemoryPythonTraceHandler.h"
#include "QueryLeaksMemoryEventHandler.h"
#include "DataBaseManager.h"
#include "TraceTime.h"

using namespace Dic::Module::Timeline;
using namespace Dic::Module::MemoryDetail;
using namespace Dic::Protocol;
using namespace Dic::Module::FullDb;
using namespace Dic;

class LeaksMemoryRequestHandlerTest : public ::testing::Test {
public:
    static const uint64_t SECOND = 1000000000;
    static const uint64_t INT64MAX = INT64_MAX;
    static void SetUpTestSuite()
    {
        Dic::Server::WsChannel *ws;
        std::unique_ptr<Dic::Server::WsSession> session = std::make_unique<Dic::Server::WsSessionImpl>(ws);
        Dic::Server::WsSessionManager::Instance().AddSession(std::move(session));
        std::string currPath = Dic::FileUtil::GetCurrPath();
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        std::string dbPath3 = R"(/src/test/test_data/full_db/)";
        DataBaseManager::Instance().SetDataType(DataType::DB);
        DataBaseManager::Instance().SetFileType(FileType::LEAKS);
        auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("");
        ASSERT_TRUE(memoryDatabase != nullptr);
        ASSERT_TRUE(memoryDatabase->OpenDb(currPath + dbPath3 + "leaks_dump_20250806.dat", false));
        ASSERT_TRUE(memoryDatabase->DropMemoryAllocationAndBlockTable());
        ASSERT_TRUE(LeaksMemoryService::ParseMemoryLeaksDumpEventsAndPythonTraces("0"));
    }
    static void TearDownTestSuite()
    {
        auto session = Dic::Server::WsSessionManager::Instance().GetSession();
        if (session != nullptr) {
            session->SetStatus(Dic::Server::WsSession::Status::CLOSED);
            session->WaitForExit();
            Dic::Server::WsSessionManager::Instance().RemoveSession();
        }
        auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("");
        memoryDatabase->CloseDb();
        DataBaseManager::Instance().Clear();
    }
};

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryAllocationsUseInvalidParamsDeviceId)
{
    QueryLeaksMemoryAllocationHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryAllocationRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryAllocationRequest>();
    requestPtr->params.deviceId = "@:1:;";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryAllocationsUseInvalidParamsEventType)
{
    QueryLeaksMemoryAllocationHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryAllocationRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryAllocationRequest>();
    requestPtr->params.deviceId = "1";
    requestPtr->params.eventType = "";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryAllocationUseInvalidParamsTimestamp)
{
    QueryLeaksMemoryAllocationHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryAllocationRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryAllocationRequest>();
    requestPtr->params.startTimestamp = INT64_MAX;
    requestPtr->params.endTimestamp = 0;
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryAllocationsUseNonExistsDeviceId)
{
    QueryLeaksMemoryAllocationHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryAllocationRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryAllocationRequest>();
    requestPtr->params.deviceId = "-1";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    requestPtr->params.eventType = "PTA";
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryAllocationsUseValidParamsWithoutTimeCondition)
{
    QueryLeaksMemoryAllocationHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryAllocationRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryAllocationRequest>();
    requestPtr->params.deviceId = "1";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    requestPtr->params.eventType = "PTA";
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryAllocationsUseValidParamsWithTimeAndRelativeCondition)
{
    QueryLeaksMemoryAllocationHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryAllocationRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryAllocationRequest>();
    requestPtr->params.deviceId = "1";
    requestPtr->params.relativeTime = true;
    requestPtr->params.eventType = "PTA";
    const uint64_t endTimestamp = 10000000000;
    requestPtr->params.endTimestamp = endTimestamp;
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryBlocksUseInvalidParamsDeviceId)
{
    QueryLeaksMemoryBlockHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryBlockRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryBlockRequest>();
    requestPtr->params.deviceId = "@:1:;";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    requestPtr->params.eventType = "PTA";
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryBlocksUseInvalidParamsTimestamp)
{
    QueryLeaksMemoryBlockHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryBlockRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryBlockRequest>();
    requestPtr->params.endTimestamp = 0;
    requestPtr->params.startTimestamp = INT64_MAX;
    requestPtr->params.deviceId = "1";
    requestPtr->params.eventType = "PTA";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryBlocksUseInvalidParamsSize)
{
    QueryLeaksMemoryBlockHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryBlockRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryBlockRequest>();
    requestPtr->params.maxSize = 0;
    requestPtr->params.minSize = INT64_MAX;
    requestPtr->params.deviceId = "1";
    requestPtr->params.eventType = "PTA";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryBlocksUseInvalidParamsExceedSize)
{
    QueryLeaksMemoryBlockHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryBlockRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryBlockRequest>();
    requestPtr->params.maxSize = INT64_MAX;
    requestPtr->params.deviceId = "1";
    requestPtr->params.eventType = "PTA";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryBlocksUseValidParamsWithoutTimeAndSizeCondition)
{
    QueryLeaksMemoryBlockHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryBlockRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryBlockRequest>();
    requestPtr->params.deviceId = "1";
    requestPtr->params.eventType = "PTA";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryBlocksUseValidParamsWithTimeCondition)
{
    QueryLeaksMemoryBlockHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryBlockRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryBlockRequest>();
    const uint64_t endTimestamp = 10000000;
    requestPtr->params.endTimestamp = endTimestamp;
    requestPtr->params.relativeTime = true;
    requestPtr->params.deviceId = "1";
    requestPtr->params.eventType = "PTA";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryBlocksUseValidParamsWithSizeCondition)
{
    QueryLeaksMemoryBlockHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryBlockRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryBlockRequest>();
    const uint64_t maxSize = 100000;
    requestPtr->params.maxSize = maxSize;
    requestPtr->params.deviceId = "0";
    requestPtr->params.eventType = "PTA";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryDetailUseInvalidParamsWithInjectDeviceId)
{
    QueryLeaksMemoryDetailHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryDetailRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryDetailRequest>();
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    requestPtr->params.deviceId = "&";
    requestPtr->params.timestamp = 0;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryDetailUseInvalidParamsWithNonExistsDeviceId)
{
    QueryLeaksMemoryDetailHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryDetailRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryDetailRequest>();
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    requestPtr->params.deviceId = "-1";
    const uint64_t durationSecond = 15;
    requestPtr->params.timestamp = durationSecond * SECOND;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryDetailUseInvalidParamsWithBigRelativeTimestamp)
{
    QueryLeaksMemoryDetailHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryDetailRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryDetailRequest>();
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    requestPtr->params.deviceId = "1";
    requestPtr->params.timestamp = INT64MAX + 1;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryDetailUseValidParams)
{
    QueryLeaksMemoryDetailHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryDetailRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryDetailRequest>();
    const uint64_t durationSecond = 15;
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    requestPtr->params.deviceId = "1";
    requestPtr->params.eventType = "PTA";
    requestPtr->params.timestamp = durationSecond * SECOND;
    requestPtr->params.relativeTime = true;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryTraceUseInvalidParamsWithInjectDeviceId)
{
    QueryLeaksMemoryPythonTraceHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryTraceRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryTraceRequest>();
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    requestPtr->params.deviceId = "&";
    requestPtr->params.relativeTime = true;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryTraceUseInvalidParamsWithInvalidThreadId)
{
    QueryLeaksMemoryPythonTraceHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryTraceRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryTraceRequest>();
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    requestPtr->params.deviceId = "";
    requestPtr->params.threadId = INT64MAX + 1;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryTraceUseInvalidParamsWithInvalidTimestamp)
{
    QueryLeaksMemoryPythonTraceHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryTraceRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryTraceRequest>();
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    requestPtr->params.deviceId = "0";
    requestPtr->params.threadId = 1;
    requestPtr->params.startTimestamp = INT64MAX + 1;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryTraceUseValidParams)
{
    QueryLeaksMemoryPythonTraceHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryTraceRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryTraceRequest>();
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    const uint64_t threadId = 3841316;
    requestPtr->params.deviceId = "1";
    requestPtr->params.threadId = 3841316;
    requestPtr->params.relativeTime = true;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryBlockTable)
{
    QueryLeaksMemoryBlockHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryBlockRequest> requestPtr =
        std::make_unique<Dic::Protocol::LeaksMemoryBlockRequest>();
    const uint64_t endTimestamp = 10000000;
    requestPtr->isTable = true;
    requestPtr->params.endTimestamp = endTimestamp;
    requestPtr->params.relativeTime = true;
    requestPtr->params.deviceId = "1";
    requestPtr->params.eventType = "PTA";
    requestPtr->moduleName = Protocol::MODULE_LEAKS;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryEventTable)
{
    QueryLeaksMemoryEventHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryEventRequest> requestPtr =
        std::make_unique<Dic::Protocol::LeaksMemoryEventRequest>();
    const uint64_t endTimestamp = 1000000000;
    requestPtr->params.endTimestamp = endTimestamp;
    requestPtr->params.relativeTime = true;
    requestPtr->params.deviceId = "1";
    requestPtr->moduleName = Protocol::MODULE_LEAKS;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}