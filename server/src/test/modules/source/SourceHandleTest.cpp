/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "QueryApiInstructionsHandler.h"
#include "QueryApiLineHandler.h"
#include "QueryCodeFileHandler.h"
#include "QueryDetailsBaseInfoHandler.h"
#include "QueryDetailsLoadInfoHandler.h"
#include "QueryDetailsMemoryGraphHandler.h"
#include "QueryDetailsMemoryTableHandler.h"
#include "SourceProtocolRequest.h"
#include "WsSessionManager.h"
#include "../../TestSuit.cpp"

using namespace Dic::Server;
using namespace Dic::Protocol;

class SourceHandleTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        Server::WsChannel *ws;
        std::unique_ptr<Server::WsSession> session = std::make_unique<Server::WsSession>(ws);
        Server::WsSessionManager::Instance().AddSession(std::move(session));
    }
    static void TearDownTestCase()
    {
        Server::WsSessionManager::Instance().GetSession()->Stop();
        Server::WsSessionManager::Instance().RemoveSession();
    }
    static int Main(int argc, char** argv)
    {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }
};

TEST_F(SourceHandleTest, QueryApiInstructionsHandlerTest)
{
    Dic::Module::Source::QueryApiInstructionsHandler handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<SourceApiInstrRequest>();
    handler.HandleRequest(std::move(requestPtr));
}

TEST_F(SourceHandleTest, QueryApiLineHandler)
{
    Dic::Module::Source::QueryApiLineHandler handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<SourceApiLineRequest>();
    handler.HandleRequest(std::move(requestPtr));
}

TEST_F(SourceHandleTest, QueryCodeFileHandler)
{
    Dic::Module::Source::QueryCodeFileHandler handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<SourceCodeFileRequest>();
    handler.HandleRequest(std::move(requestPtr));
}

TEST_F(SourceHandleTest, QueryDetailsBaseInfoHandler)
{
    Dic::Module::Source::QueryDetailsBaseInfoHandler handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<SourceDetailBaseInfoRequest>();
    handler.HandleRequest(std::move(requestPtr));
}

TEST_F(SourceHandleTest, QueryDetailsLoadInfoHandler)
{
    Dic::Module::Source::QueryDetailsLoadInfoHandler handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<SourceDetailsLoadInfoRequest>();
    handler.HandleRequest(std::move(requestPtr));
}

TEST_F(SourceHandleTest, QueryDetailsMemoryGraphHandler)
{
    Dic::Module::Source::QueryDetailsMemoryGraphHandler handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<DetailsMemoryGraphRequest>();
    handler.HandleRequest(std::move(requestPtr));
}

TEST_F(SourceHandleTest, QueryDetailsMemoryTableHandler)
{
    Dic::Module::Source::QueryDetailsMemoryTableHandler handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<DetailsMemoryTableRequest>();
    handler.HandleRequest(std::move(requestPtr));
}