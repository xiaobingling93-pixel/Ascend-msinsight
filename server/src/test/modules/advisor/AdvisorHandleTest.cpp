/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "QueryAclnnOpAdvisorHandler.h"
#include "QueryAffinityAPIAdvice.h"
#include "QueryAffinityOptimizerAdvice.h"
#include "QueryAiCpuOpAdviceHandler.h"
#include "QueryFusedOpAdviceHandler.h"
#include "QueryOperatorDispatchHandler.h"
#include "AdvisorProtocolRequest.h"
#include "WsSessionManager.h"
#include "WsSessionImpl.h"
#include "../../TestSuit.cpp"

using namespace Dic::Server;
using namespace Dic::Protocol;
using namespace Dic::Module::Advisor;

class AdvisorHandleTest : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        Server::WsChannel *ws;
        std::unique_ptr<Server::WsSessionImpl> session = std::make_unique<Server::WsSessionImpl>(ws);
        Server::WsSessionManager::Instance().AddSession(std::move(session));
    }
    static void TearDownTestSuite()
    {
        auto session = Server::WsSessionManager::Instance().GetSession();
        if (session != nullptr) {
            session->SetStatus(WsSession::Status::CLOSED);
            session->WaitForExit();
            Server::WsSessionManager::Instance().RemoveSession();
        }
    }
    static int Main(int argc, char** argv)
    {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }
};

TEST_F(AdvisorHandleTest, QueryAclnnOpAdvisorHandlerTestReturnFalseWhenInvalidPageSize)
{
    QueryAclnnOpAdvisorHandler handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<AclnnOperatorRequest>();
    auto &request = dynamic_cast<AclnnOperatorRequest &>(*requestPtr);
    request.params.pageSize = MAX_PAGESIZE + 1;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandleTest, QueryAclnnOpAdvisorHandlerTestReturnFalseWhenInvalidCurrentPage)
{
    QueryAclnnOpAdvisorHandler handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<AclnnOperatorRequest>();
    auto &request = dynamic_cast<AclnnOperatorRequest &>(*requestPtr);
    request.params.pageSize = MAX_PAGESIZE + 1;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandleTest, QueryAclnnOpAdvisorHandlerTestReturnFalseWhenEmptyRankId)
{
    QueryAclnnOpAdvisorHandler handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<AclnnOperatorRequest>();
    auto &request = dynamic_cast<AclnnOperatorRequest &>(*requestPtr);
    request.params.pageSize = MAX_PAGESIZE;
    request.params.currentPage = 1;
    request.params.rankId = "";
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandleTest, QueryAclnnOpAdvisorHandlerTestReturnFalseWhenRankIdContainsIllegalChars)
{
    QueryAclnnOpAdvisorHandler handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<AclnnOperatorRequest>();
    auto &request = dynamic_cast<AclnnOperatorRequest &>(*requestPtr);
    request.params.pageSize = MAX_PAGESIZE - 1;
    request.params.currentPage = 1;
    request.params.rankId = "0&";
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandleTest, QueryAclnnOpAdvisorHandlerTestReturnFalseWhenOrderByContainsIllegalChars)
{
    QueryAclnnOpAdvisorHandler handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<AclnnOperatorRequest>();
    auto &request = dynamic_cast<AclnnOperatorRequest &>(*requestPtr);
    request.params.pageSize = MAX_PAGESIZE - 1;
    request.params.currentPage = 1;
    request.params.rankId = "0";
    request.params.orderBy = "&";
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandleTest, QueryAclnnOpAdvisorHandlerTestReturnFalseWhenOrderTypeIsTooLong)
{
    QueryAclnnOpAdvisorHandler handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<AclnnOperatorRequest>();
    auto &request = dynamic_cast<AclnnOperatorRequest &>(*requestPtr);
    request.params.pageSize = MAX_PAGESIZE - 1;
    request.params.currentPage = 1;
    request.params.rankId = "0";
    request.params.orderBy = "step";
    std::string str(MAX_PAGESIZE, 'a'); // ref 500
    request.params.orderBy = str;
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandleTest, QueryAffinityAPIAdvice)
{
    QueryAffinityAPIAdvice handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<AffinityAPIRequest>();
    handler.HandleRequest(std::move(requestPtr));
}

TEST_F(AdvisorHandleTest, QueryAffinityOptimizerAdvice)
{
    QueryAffinityOptimizerAdvice handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<AffinityOptimizerRequest>();
    handler.HandleRequest(std::move(requestPtr));
}

TEST_F(AdvisorHandleTest, QueryAiCpuOpAdviceHandler)
{
    QueryAiCpuOpAdviceHandler handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<AICpuOperatorRequest>();
    handler.HandleRequest(std::move(requestPtr));
}

TEST_F(AdvisorHandleTest, QueryFusedOpAdviceHandler)
{
    QueryFusedOpAdviceHandler handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<OperatorFusionRequest>();
    handler.HandleRequest(std::move(requestPtr));
}

TEST_F(AdvisorHandleTest, QueryOperatorDispatchHandler)
{
    QueryOperatorDispatchHandler handler;
    std::unique_ptr<Request> requestPtr = std::make_unique<OperatorDispatchRequest>();
    handler.HandleRequest(std::move(requestPtr));
}
