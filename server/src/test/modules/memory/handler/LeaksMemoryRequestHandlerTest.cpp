/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include <WsSessionManager.h>
#include "WsSessionImpl.h"
#include "MemoryProtocolRequest.h"
#include "LeaksMemoryDatabase.h"
#include "QueryLeaksMemoryAllocationHandler.h"
#include "QueryLeaksMemoryBlockHandler.h"
#include "DataBaseManager.h"
#include "TraceTime.h"

using namespace Dic::Module::Timeline;
using namespace Dic::Module::Memory;
using namespace Dic::Protocol;
using namespace Dic::Module::FullDb;
using namespace Dic;

class LeaksMemoryRequestHandlerTest : public ::testing::Test {
public:
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
        auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("0");
        memoryDatabase->OpenDb(currPath + dbPath3 + "leaks_dump_2025.dat", false);
    }
    static void TearDownTestSuite()
    {
        auto session = Dic::Server::WsSessionManager::Instance().GetSession();
        if (session != nullptr) {
            session->SetStatus(Dic::Server::WsSession::Status::CLOSED);
            session->WaitForExit();
            Dic::Server::WsSessionManager::Instance().RemoveSession();
        }
        auto memoryDatabase = DataBaseManager::Instance().GetLeaksMemoryDatabase("0");
        memoryDatabase->CloseDb();
        DataBaseManager::Instance().Clear();
    }
};

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryAllocationsUseInvalidParamsDeviceId)
{
    Dic::Module::Memory::QueryLeaksMemoryAllocationHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryAllocationRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryAllocationRequest>();
    requestPtr->params.deviceId = "@:1:;";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryAllocationsUseInvalidParamsEventType)
{
    Dic::Module::Memory::QueryLeaksMemoryAllocationHandler handler;
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
    Dic::Module::Memory::QueryLeaksMemoryAllocationHandler handler;
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
    Dic::Module::Memory::QueryLeaksMemoryAllocationHandler handler;
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
    Dic::Module::Memory::QueryLeaksMemoryAllocationHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryAllocationRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryAllocationRequest>();
    requestPtr->params.deviceId = "0";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    requestPtr->params.eventType = "PTA";
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryAllocationsUseValidParamsWithTimeAndRelativeCondition)
{
    Dic::Module::Memory::QueryLeaksMemoryAllocationHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryAllocationRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryAllocationRequest>();
    requestPtr->params.deviceId = "0";
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
    Dic::Module::Memory::QueryLeaksMemoryBlockHandler handler;
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
    Dic::Module::Memory::QueryLeaksMemoryBlockHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryBlockRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryBlockRequest>();
    requestPtr->params.endTimestamp = 0;
    requestPtr->params.startTimestamp = INT64_MAX;
    requestPtr->params.deviceId = "0";
    requestPtr->params.eventType = "PTA";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryBlocksUseInvalidParamsSize)
{
    Dic::Module::Memory::QueryLeaksMemoryBlockHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryBlockRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryBlockRequest>();
    requestPtr->params.maxSize = 0;
    requestPtr->params.minSize = INT64_MAX;
    requestPtr->params.deviceId = "0";
    requestPtr->params.eventType = "PTA";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryBlocksUseInvalidParamsExceedSize)
{
    Dic::Module::Memory::QueryLeaksMemoryBlockHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryBlockRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryBlockRequest>();
    requestPtr->params.maxSize = INT64_MAX;
    requestPtr->params.deviceId = "0";
    requestPtr->params.eventType = "PTA";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_FALSE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryBlocksUseValidParamsWithoutTimeAndSizeCondition)
{
    Dic::Module::Memory::QueryLeaksMemoryBlockHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryBlockRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryBlockRequest>();
    requestPtr->params.deviceId = "0";
    requestPtr->params.eventType = "PTA";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryBlocksUseValidParamsWithTimeCondition)
{
    Dic::Module::Memory::QueryLeaksMemoryBlockHandler handler;
    std::unique_ptr<Dic::Protocol::LeaksMemoryBlockRequest> requestPtr =
            std::make_unique<Dic::Protocol::LeaksMemoryBlockRequest>();
    const uint64_t endTimestamp = 10000000;
    requestPtr->params.endTimestamp = endTimestamp;
    requestPtr->params.relativeTime = true;
    requestPtr->params.deviceId = "0";
    requestPtr->params.eventType = "PTA";
    requestPtr->moduleName = Protocol::MODULE_MEMORY;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_TRUE(result);
}

TEST_F(LeaksMemoryRequestHandlerTest, QueryMemoryBlocksUseValidParamsWithSizeCondition)
{
    Dic::Module::Memory::QueryLeaksMemoryBlockHandler handler;
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