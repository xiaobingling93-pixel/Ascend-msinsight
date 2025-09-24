/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "FileUtil.h"
#include "WsSessionImpl.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "AdvisorProtocolRequest.h"
#include "QueryAclnnOpAdvisorHandler.h"
#include "QueryAffinityAPIAdvice.h"
#include "QueryAffinityOptimizerAdvice.h"
#include "QueryAiCpuOpAdviceHandler.h"
#include "QueryFusedOpAdviceHandler.h"
#include "QueryOperatorDispatchHandler.h"

using namespace Dic::Module::Advisor;
using namespace Dic::Protocol;
using namespace Dic::Module::Timeline;
class AdvisorHandlerTest : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        std::string currPath = Dic::FileUtil::GetCurrPath();
        size_t index = currPath.find_last_of("server");
        std::string serverPath = currPath.substr(0, index + 1);
        std::string dbPath = Dic::FileUtil::GetParentPath(serverPath) +
                R"(/test/data/pytorch/db/level1/rank0_ascend_pt/ASCEND_PROFILER_OUTPUT/ascend_pytorch_profiler_0.db)";
        Dic::Server::WsChannel *ws;
        std::unique_ptr<Dic::Server::WsSessionImpl> session = std::make_unique<Dic::Server::WsSessionImpl>(ws);
        Dic::Server::WsSessionManager::Instance().AddSession(std::move(session));
        DataBaseManager::Instance().SetDataType(DataType::DB);
        DataBaseManager::Instance().SetFileType(FileType::PYTORCH);
        DataBaseManager::Instance().CreatTraceConnectionPool("0", dbPath);
    }
    static void TearDownTestSuite()
    {
        auto session = Dic::Server::WsSessionManager::Instance().GetSession();
        if (session != nullptr) {
            session->SetStatus(Dic::Server::WsSession::Status::CLOSED);
            session->WaitForExit();
            Dic::Server::WsSessionManager::Instance().RemoveSession();
        }
        DataBaseManager::Instance().Clear();
    }

    static void SetDefaultAPITypeParamsWithExistRankId(APITypeParams &param)
    {
        param.pageSize = Dic::DEFAULT_PAGESIZE;
        param.currentPage = 1;
        param.rankId = "0";
        param.orderBy = "duration";
        param.orderType = "ascend";
    }

    static void SetDefaultAPITypeParamsWithNotExistRankId(APITypeParams &param)
    {
        SetDefaultAPITypeParamsWithExistRankId(param);
        param.rankId = "1";
    }
};

TEST_F(AdvisorHandlerTest, QueryAclnnOpAdvisorHandlerTestReturnFalseWhenMinPageSize)
{
    auto request = std::make_unique<AclnnOperatorRequest>();
    request->params.pageSize = Dic::MIN_PAGESIZE;
    QueryAclnnOpAdvisorHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandlerTest, QueryAclnnOpAdvisorHandlerTestReturnFalseWhenMaxPageSize)
{
    auto request = std::make_unique<AclnnOperatorRequest>();
    request->params.pageSize = Dic::MAX_PAGESIZE + 1;
    QueryAclnnOpAdvisorHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandlerTest, QueryAclnnOpAdvisorHandlerTestReturnFalseWhenMinCurrentPage)
{
    auto request = std::make_unique<AclnnOperatorRequest>();
    request->params.pageSize = Dic::MAX_PAGESIZE;
    request->params.currentPage = Dic::MIN_CURRENT_PAGE;
    QueryAclnnOpAdvisorHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandlerTest, QueryAclnnOpAdvisorHandlerTestReturnFalseWhenMaxCurrentPage)
{
    auto request = std::make_unique<AclnnOperatorRequest>();
    request->params.pageSize = Dic::MAX_PAGESIZE;
    request->params.currentPage = (uint32_t)Dic::MAX_CURRENT_PAGE + 1;
    QueryAclnnOpAdvisorHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}


TEST_F(AdvisorHandlerTest, QueryAclnnOpAdvisorHandlerTestReturnFalseWhenEmptyRankId)
{
    auto request = std::make_unique<AclnnOperatorRequest>();
    request->params.pageSize = Dic::DEFAULT_PAGESIZE;
    request->params.currentPage = 1;
    request->params.rankId = "";
    QueryAclnnOpAdvisorHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandlerTest, QueryAclnnOpAdvisorHandlerTestReturnFalseWhenInjectRankId)
{
    auto request = std::make_unique<AclnnOperatorRequest>();
    request->params.pageSize = Dic::DEFAULT_PAGESIZE;
    request->params.currentPage = 1;
    request->params.rankId = "0&";
    QueryAclnnOpAdvisorHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandlerTest, QueryAclnnOpAdvisorHandlerTestReturnFalseWhenLongRankId)
{
    auto request = std::make_unique<AclnnOperatorRequest>();
    request->params.pageSize = Dic::DEFAULT_PAGESIZE;
    request->params.currentPage = 1;
    std::string str(501, 'a'); // 501 > 500
    request->params.rankId = str;
    QueryAclnnOpAdvisorHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandlerTest, QueryAclnnOpAdvisorHandlerTestReturnTrue)
{
    auto request = std::make_unique<AclnnOperatorRequest>();
    SetDefaultAPITypeParamsWithExistRankId(request->params);

    QueryAclnnOpAdvisorHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, true);
}

TEST_F(AdvisorHandlerTest, QueryAclnnOpAdvisorHandlerTestReturnFalseWhenNotExistRankId)
{
    auto request = std::make_unique<AclnnOperatorRequest>();
    SetDefaultAPITypeParamsWithNotExistRankId(request->params);
    QueryAclnnOpAdvisorHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandlerTest, QueryAffinityAPIAdviceTestReturnTrue)
{
    auto request = std::make_unique<AffinityAPIRequest>();
    SetDefaultAPITypeParamsWithExistRankId(request->params);
    QueryAffinityAPIAdvice handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, true);
}

TEST_F(AdvisorHandlerTest, QueryAffinityAPIAdviceTestReturnFalseWhenNotExistRankId)
{
    auto request = std::make_unique<AffinityAPIRequest>();
    SetDefaultAPITypeParamsWithNotExistRankId(request->params);
    QueryAffinityAPIAdvice handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandlerTest, QueryAffinityOptimizerAdviceTestReturnTrue)
{
    auto request = std::make_unique<AffinityOptimizerRequest>();
    SetDefaultAPITypeParamsWithExistRankId(request->params);
    QueryAffinityOptimizerAdvice handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, true);
}

TEST_F(AdvisorHandlerTest, QueryAffinityOptimizerAdviceReturnFalseWhenNotExistRankId)
{
    auto request = std::make_unique<AffinityOptimizerRequest>();
    SetDefaultAPITypeParamsWithNotExistRankId(request->params);
    QueryAffinityOptimizerAdvice handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandlerTest, QueryAiCpuOpAdviceHandlerTestReturnTrue)
{
    auto request = std::make_unique<AICpuOperatorRequest>();
    SetDefaultAPITypeParamsWithExistRankId(request->params);
    QueryAiCpuOpAdviceHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, true);
}

TEST_F(AdvisorHandlerTest, QueryAiCpuOpAdviceHandlerReturnFalseWhenNotExistRankId)
{
    auto request = std::make_unique<AICpuOperatorRequest>();
    SetDefaultAPITypeParamsWithNotExistRankId(request->params);
    QueryAiCpuOpAdviceHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandlerTest, QueryFusedOpAdviceHandlerTestReturnTrue)
{
    auto request = std::make_unique<OperatorFusionRequest>();
    SetDefaultAPITypeParamsWithExistRankId(request->params);
    QueryFusedOpAdviceHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, true);
}

TEST_F(AdvisorHandlerTest, QueryFusedOpAdviceHandlerReturnFalseWhenNotExistRankId)
{
    auto request = std::make_unique<OperatorFusionRequest>();
    SetDefaultAPITypeParamsWithNotExistRankId(request->params);
    QueryFusedOpAdviceHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandlerTest, QueryOperatorDispatchAdvisorHandlerTestReturnFalseWhenMinPageSize)
{
    auto request = std::make_unique<OperatorDispatchRequest>();
    request->params.pageSize = Dic::MIN_PAGESIZE;
    QueryOperatorDispatchHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandlerTest, QueryOperatorDispatchAdvisorHandlerTestReturnFalseWhenMaxPageSize)
{
    auto request = std::make_unique<OperatorDispatchRequest>();
    request->params.pageSize = Dic::MAX_PAGESIZE + 1;
    QueryOperatorDispatchHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandlerTest, QueryOperatorDispatchAdvisorHandlerTestReturnFalseWhenMinCurrentPage)
{
    auto request = std::make_unique<OperatorDispatchRequest>();
    request->params.pageSize = Dic::MAX_PAGESIZE;
    request->params.currentPage = Dic::MIN_CURRENT_PAGE;
    QueryOperatorDispatchHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandlerTest, QueryOperatorDispatchAdvisorHandlerTestReturnFalseWhenMaxCurrentPage)
{
    auto request = std::make_unique<OperatorDispatchRequest>();
    request->params.pageSize = Dic::MAX_PAGESIZE;
    request->params.currentPage = (uint32_t)Dic::MAX_CURRENT_PAGE + 1;
    QueryOperatorDispatchHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandlerTest, QueryOperatorDispatchAdvisorHandlerTestReturnFalseWhenEmptyRankId)
{
    auto request = std::make_unique<OperatorDispatchRequest>();
    request->params.pageSize = Dic::DEFAULT_PAGESIZE;
    request->params.currentPage = 1;
    request->params.rankId = "";
    QueryOperatorDispatchHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandlerTest, QueryOperatorDispatchAdvisorHandlerTestReturnFalseWhenInjectRankId)
{
    auto request = std::make_unique<OperatorDispatchRequest>();
    request->params.pageSize = Dic::DEFAULT_PAGESIZE;
    request->params.currentPage = 1;
    request->params.rankId = "0&";
    QueryOperatorDispatchHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandlerTest, QueryOperatorDispatchAdvisorHandlerTestReturnFalseWhenLongRankId)
{
    auto request = std::make_unique<OperatorDispatchRequest>();
    request->params.pageSize = Dic::DEFAULT_PAGESIZE;
    request->params.currentPage = 1;
    std::string str(501, 'a'); // 501 > 500
    request->params.rankId = str;
    QueryOperatorDispatchHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(AdvisorHandlerTest, QueryOperatorDispatchAdvisorHandlerTestReturnFalseWhenNotExistRankId)
{
    auto request = std::make_unique<OperatorDispatchRequest>();
    SetDefaultAPITypeParamsWithNotExistRankId(request->params);
    QueryOperatorDispatchHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}
