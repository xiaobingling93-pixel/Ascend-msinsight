/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include <WsSessionManager.h>
#include "WsSessionImpl.h"
#include "MemScopeProtocolRequest.h"
#include "MemScopeService.h"
#include "QueryMemScopeAllocationHandler.h"
#include "QueryMemScopeBlockHandler.h"
#include "QueryMemScopeMemoryDetailHandler.h"
#include "QueryMemScopePythonTraceHandler.h"
#include "QueryMemScopeEventHandler.h"
#include "DataBaseManager.h"
#include "TraceTime.h"

using namespace Dic::Module::Timeline;
using namespace Dic::Module::MemScope;
using namespace Dic::Protocol;
using namespace Dic::Module::FullDb;
using namespace Dic;

class MemScopeRequestHandlerTest : public ::testing::Test {
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
        DataBaseManager::Instance().SetFileType(FileType::MEM_SCOPE);
        auto memoryDatabase = DataBaseManager::Instance().GetMemScopeDatabase("");
        ASSERT_TRUE(memoryDatabase != nullptr);
        ASSERT_TRUE(memoryDatabase->OpenDb(currPath + dbPath3 + "leaks_dump_20250806.dat", false));
        ASSERT_TRUE(memoryDatabase->DropMemoryAllocationAndBlockTable());
        ASSERT_TRUE(MemScopeService::ParseMemoryMemScopeDumpEventsAndPythonTraces("0"));
    }
    static void TearDownTestSuite()
    {
        auto session = Dic::Server::WsSessionManager::Instance().GetSession();
        if (session != nullptr) {
            session->SetStatus(Dic::Server::WsSession::Status::CLOSED);
            session->WaitForExit();
            Dic::Server::WsSessionManager::Instance().RemoveSession();
        }
        auto memoryDatabase = DataBaseManager::Instance().GetMemScopeDatabase("");
        memoryDatabase->CloseDb();
        DataBaseManager::Instance().Clear();
    }
};

TEST_F(MemScopeRequestHandlerTest, QueryMemoryAllocationsUseInvalidParamsDeviceId)
{
    QueryMemScopeAllocationHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopeMemoryAllocationRequest> requestPtr =
            std::make_unique<Dic::Protocol::MemScopeMemoryAllocationRequest>();
    requestPtr->params.deviceId = "@:1:;";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryAllocationsUseInvalidParamsEventType)
{
    QueryMemScopeAllocationHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopeMemoryAllocationRequest> requestPtr =
            std::make_unique<Dic::Protocol::MemScopeMemoryAllocationRequest>();
    requestPtr->params.deviceId = "1";
    requestPtr->params.eventType = "";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryAllocationUseInvalidParamsTimestamp)
{
    QueryMemScopeAllocationHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopeMemoryAllocationRequest> requestPtr =
            std::make_unique<Dic::Protocol::MemScopeMemoryAllocationRequest>();
    requestPtr->params.startTimestamp = INT64_MAX;
    requestPtr->params.endTimestamp = 0;
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryAllocationsUseNonExistsDeviceId)
{
    QueryMemScopeAllocationHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopeMemoryAllocationRequest> requestPtr =
            std::make_unique<Dic::Protocol::MemScopeMemoryAllocationRequest>();
    requestPtr->params.deviceId = "-1";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    requestPtr->params.eventType = "PTA";
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryAllocationsUseValidParamsWithoutTimeCondition)
{
    QueryMemScopeAllocationHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopeMemoryAllocationRequest> requestPtr =
            std::make_unique<Dic::Protocol::MemScopeMemoryAllocationRequest>();
    requestPtr->params.deviceId = "1";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    requestPtr->params.eventType = "PTA";
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryAllocationsUseValidParamsWithTimeAndRelativeCondition)
{
    QueryMemScopeAllocationHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopeMemoryAllocationRequest> requestPtr =
            std::make_unique<Dic::Protocol::MemScopeMemoryAllocationRequest>();
    requestPtr->params.deviceId = "1";
    requestPtr->params.relativeTime = true;
    requestPtr->params.eventType = "PTA";
    const uint64_t endTimestamp = 10000000000;
    requestPtr->params.endTimestamp = endTimestamp;
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryBlocksUseInvalidParamsDeviceId)
{
    QueryMemScopeBlockHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopeMemoryBlockRequest> requestPtr =
            std::make_unique<Dic::Protocol::MemScopeMemoryBlockRequest>();
    requestPtr->params.deviceId = "@:1:;";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    requestPtr->params.eventType = "PTA";
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryBlocksUseInvalidParamsTimestamp)
{
    QueryMemScopeBlockHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopeMemoryBlockRequest> requestPtr =
            std::make_unique<Dic::Protocol::MemScopeMemoryBlockRequest>();
    requestPtr->params.endTimestamp = 0;
    requestPtr->params.startTimestamp = INT64_MAX;
    requestPtr->params.deviceId = "1";
    requestPtr->params.eventType = "PTA";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryBlocksUseInvalidParamsSize)
{
    QueryMemScopeBlockHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopeMemoryBlockRequest> requestPtr =
            std::make_unique<Dic::Protocol::MemScopeMemoryBlockRequest>();
    requestPtr->params.maxSize = 0;
    requestPtr->params.minSize = INT64_MAX;
    requestPtr->params.deviceId = "1";
    requestPtr->params.eventType = "PTA";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryBlocksUseInvalidParamsExceedSize)
{
    QueryMemScopeBlockHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopeMemoryBlockRequest> requestPtr =
            std::make_unique<Dic::Protocol::MemScopeMemoryBlockRequest>();
    requestPtr->params.maxSize = INT64_MAX;
    requestPtr->params.deviceId = "1";
    requestPtr->params.eventType = "PTA";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryBlocksUseValidParamsWithoutTimeAndSizeCondition)
{
    QueryMemScopeBlockHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopeMemoryBlockRequest> requestPtr =
            std::make_unique<Dic::Protocol::MemScopeMemoryBlockRequest>();
    requestPtr->params.deviceId = "1";
    requestPtr->params.eventType = "PTA";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryBlocksUseValidParamsWithTimeCondition)
{
    QueryMemScopeBlockHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopeMemoryBlockRequest> requestPtr =
            std::make_unique<Dic::Protocol::MemScopeMemoryBlockRequest>();
    const uint64_t endTimestamp = 10000000;
    requestPtr->params.endTimestamp = endTimestamp;
    requestPtr->params.relativeTime = true;
    requestPtr->params.deviceId = "1";
    requestPtr->params.eventType = "PTA";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryBlocksUseValidParamsWithSizeCondition)
{
    QueryMemScopeBlockHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopeMemoryBlockRequest> requestPtr =
            std::make_unique<Dic::Protocol::MemScopeMemoryBlockRequest>();
    const uint64_t maxSize = 100000;
    requestPtr->params.maxSize = maxSize;
    requestPtr->params.deviceId = "0";
    requestPtr->params.eventType = "PTA";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryDetailUseInvalidParamsWithInjectDeviceId)
{
    QueryMemScopeMemoryDetailHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopeMemoryDetailRequest> requestPtr =
            std::make_unique<Dic::Protocol::MemScopeMemoryDetailRequest>();
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    requestPtr->params.deviceId = "&";
    requestPtr->params.timestamp = 0;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryDetailUseInvalidParamsWithNonExistsDeviceId)
{
    QueryMemScopeMemoryDetailHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopeMemoryDetailRequest> requestPtr =
            std::make_unique<Dic::Protocol::MemScopeMemoryDetailRequest>();
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    requestPtr->params.deviceId = "-1";
    const uint64_t durationSecond = 15;
    requestPtr->params.timestamp = durationSecond * SECOND;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryDetailUseInvalidParamsWithBigRelativeTimestamp)
{
    QueryMemScopeMemoryDetailHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopeMemoryDetailRequest> requestPtr =
            std::make_unique<Dic::Protocol::MemScopeMemoryDetailRequest>();
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    requestPtr->params.deviceId = "1";
    requestPtr->params.timestamp = INT64MAX + 1;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryDetailUseValidParams)
{
    QueryMemScopeMemoryDetailHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopeMemoryDetailRequest> requestPtr =
            std::make_unique<Dic::Protocol::MemScopeMemoryDetailRequest>();
    const uint64_t durationSecond = 15;
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    requestPtr->params.deviceId = "1";
    requestPtr->params.eventType = "PTA";
    requestPtr->params.timestamp = durationSecond * SECOND;
    requestPtr->params.relativeTime = true;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryTraceUseInvalidParamsWithInjectDeviceId)
{
    QueryMemScopePythonTraceHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopePythonTraceRequest> requestPtr =
            std::make_unique<Dic::Protocol::MemScopePythonTraceRequest>();
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    requestPtr->params.deviceId = "&";
    requestPtr->params.relativeTime = true;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryTraceUseInvalidParamsWithInvalidThreadId)
{
    QueryMemScopePythonTraceHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopePythonTraceRequest> requestPtr =
            std::make_unique<Dic::Protocol::MemScopePythonTraceRequest>();
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    requestPtr->params.deviceId = "";
    requestPtr->params.threadId = INT64MAX + 1;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryTraceUseInvalidParamsWithInvalidTimestamp)
{
    QueryMemScopePythonTraceHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopePythonTraceRequest> requestPtr =
            std::make_unique<Dic::Protocol::MemScopePythonTraceRequest>();
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    requestPtr->params.deviceId = "0";
    requestPtr->params.threadId = 1;
    requestPtr->params.startTimestamp = INT64MAX + 1;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryTraceUseValidParams)
{
    QueryMemScopePythonTraceHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopePythonTraceRequest> requestPtr =
            std::make_unique<Dic::Protocol::MemScopePythonTraceRequest>();
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    const uint64_t threadId = 3841316;
    requestPtr->params.deviceId = "1";
    requestPtr->params.threadId = 3841316;
    requestPtr->params.relativeTime = true;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryBlockTable)
{
    QueryMemScopeBlockHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopeMemoryBlockRequest> requestPtr =
        std::make_unique<Dic::Protocol::MemScopeMemoryBlockRequest>();
    const uint64_t endTimestamp = 10000000;
    requestPtr->isTable = true;
    requestPtr->params.endTimestamp = endTimestamp;
    requestPtr->params.relativeTime = true;
    requestPtr->params.deviceId = "1";
    requestPtr->params.eventType = "PTA";
    requestPtr->moduleName = Protocol::MODULE_MEM_SCOPE;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}

TEST_F(MemScopeRequestHandlerTest, QueryMemoryEventTable)
{
    QueryMemScopeEventHandler handler;
    std::unique_ptr<Dic::Protocol::MemScopeEventRequest> requestPtr =
        std::make_unique<Dic::Protocol::MemScopeEventRequest>();
    const uint64_t endTimestamp = 1000000000;
    requestPtr->params.endTimestamp = endTimestamp;
    requestPtr->params.relativeTime = true;
    requestPtr->params.deviceId = "1";
    requestPtr->moduleName = Protocol::MODULE_MEM_SCOPE;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}