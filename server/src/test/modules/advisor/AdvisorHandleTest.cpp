/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "QueryAclnnOpAdvisorHandler.h"
#include "QueryAffinityAPIAdvice.h"
#include "QueryAffinityOptimizerAdvice.h"
#include "QueryAiCpuOpAdviceHandler.h"
#include "QueryFusedOpAdviceHandler.h"
#include "AdvisorProtocolRequest.h"
#include "WsSessionManager.h"
#include "WsSession.h"
#include "../../TestSuit.cpp"

using namespace Dic::Server;
using namespace Dic::Protocol;
using namespace Dic::Module::Advisor;

class AdvisorHandleTest : public ::testing::Test {
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

TEST_F(AdvisorHandleTest, QueryAclnnOpAdvisorHandler)
{
    Server::WsSession &session = *Server::WsSessionManager::Instance().GetSession();
    EXPECT_NE(session.GetStatus(), WsSession::Status::CLOSED);
    QueryAclnnOpAdvisorHandler handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<AclnnOperatorRequest>();
    handler.HandleRequest(std::move(requestPtr));
}

TEST_F(AdvisorHandleTest, QueryAffinityAPIAdvice)
{
    Server::WsSession &session = *Server::WsSessionManager::Instance().GetSession();
    EXPECT_NE(session.GetStatus(), WsSession::Status::CLOSED);
    QueryAffinityAPIAdvice handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<AffinityAPIRequest>();
    handler.HandleRequest(std::move(requestPtr));
}

TEST_F(AdvisorHandleTest, QueryAffinityOptimizerAdvice)
{
    Server::WsSession &session = *Server::WsSessionManager::Instance().GetSession();
    EXPECT_NE(session.GetStatus(), WsSession::Status::CLOSED);
    QueryAffinityOptimizerAdvice handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<AffinityOptimizerRequest>();
    handler.HandleRequest(std::move(requestPtr));
}

TEST_F(AdvisorHandleTest, QueryAiCpuOpAdviceHandler)
{
    Server::WsSession &session = *Server::WsSessionManager::Instance().GetSession();
    EXPECT_NE(session.GetStatus(), WsSession::Status::CLOSED);
    QueryAiCpuOpAdviceHandler handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<AICpuOperatorRequest>();
    handler.HandleRequest(std::move(requestPtr));
}

TEST_F(AdvisorHandleTest, QueryFusedOpAdviceHandler)
{
    Server::WsSession &session = *Server::WsSessionManager::Instance().GetSession();
    EXPECT_NE(session.GetStatus(), WsSession::Status::CLOSED);
    QueryFusedOpAdviceHandler handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<OperatorFusionRequest>();
    handler.HandleRequest(std::move(requestPtr));
}
