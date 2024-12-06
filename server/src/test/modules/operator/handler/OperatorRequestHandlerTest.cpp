/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include <WsSessionManager.h>
#include "BaselineManagerService.h"
#include "DataBaseManager.h"
#include "DbSummaryDataBase.h"
#include "FileUtil.h"
#include "OperatorRequestHandler.h"
#include "QueryOpCategoryInfoHandler.h"
#include "QueryOpComputeUnitHandler.h"
#include "QueryOpStatisticInfoHandler.h"
#include "QueryOpDetailInfoHandler.h"
#include "QueryOpMoreInfoHandler.h"
#include "ParamsParser.h"
#include "ProjectExplorerManager.h"
#include "WsSessionImpl.h"
#include "../../../FullDbTestSuit.cpp"

using namespace Dic::Server;
using namespace Dic::Module::Timeline;
using namespace Dic::Module::FullDb;
using namespace Dic::Module::Global;
class OperatorRequestHandlerTest : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        Dic::Server::WsChannel *ws;
        std::unique_ptr<WsSessionImpl> session = std::make_unique<WsSessionImpl>(ws);
        Dic::Server::WsSessionManager::Instance().AddSession(std::move(session));
    }
    static void TearDownTestSuite()
    {
        auto session = Dic::Server::WsSessionManager::Instance().GetSession();
        if (session != nullptr) {
            session->SetStatus(Dic::Server::WsSession::Status::CLOSED);
            session->WaitForExit();
            Dic::Server::WsSessionManager::Instance().RemoveSession();
        }
    }

    static void InitDbManager()
    {
        std::string currPath = Dic::FileUtil::GetCurrPath();
        const ParamsOption &option = ParamsParser::Instance().GetOption();
        ServerLog::Initialize(option.logPath, option.logSize, option.logLevel, to_string(option.wsPort));
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        std::string dbPath3 = R"(/src/test/test_data/full_db/)";
        DataBaseManager::Instance().SetDataType(DataType::DB);
        auto summeryDatabase =
            std::dynamic_pointer_cast<DbSummaryDataBase, Dic::Module::Summary::VirtualSummaryDataBase>(
                DataBaseManager::Instance().GetSummaryDatabase("2"));
        summeryDatabase->OpenDb(currPath + dbPath3 + "msprof_0.db", false);
    }

    static void InitBaseLineManager()
    {
        std::string systemDbPath = currPath.substr(0, index + 1) + R"(/src/test/test_data/)";
        ProjectExplorerManager::Instance().InitSystemMemoryDbPath(systemDbPath);
        InitProjectExplorerData();
    }

    static bool SetBaseLineManager()
    {
        InitBaseLineManager();
        // 创建DB场景的baseline基线manager
        std::string filePathText = currPath.substr(0, index + 1) +
                                   R"(/src/test/test_data/test_rank_0/ASCEND_PROFILER_OUTPUT)";
        BaselineInfo baselineInfo;
        bool result = BaselineManagerService::InitBaselineData("testProject", filePathText, baselineInfo);
        std::string notFinishTask = "";
        int index = 0;
        while (index < retry && !Dic::Module::Timeline::ParserStatusManager::Instance().IsAllFinished(notFinishTask)) {
            const int sleepTime = 2000;
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
            index++;
        }
        return result;
    }

    static void ClearProjectExplorerData()
    {
        ProjectExplorerManager::Instance().DeleteProjectAndFilePath("testProject",
                                                                    std::vector<std::string>());
        ProjectExplorerManager::Instance().DeleteProjectAndFilePath("testProjectDb",
                                                                    std::vector<std::string>());
    }

protected:
    inline static std::string currPath = Dic::FileUtil::GetCurrPath();
    inline static int index = currPath.find_last_of("server");
    inline static int retry = 5;
    static ProjectExplorerInfo CreateProjectData(const std::string &projectName, const std::string &fileName,
                                                 const std::string &importType, Dic::ProjectTypeEnum projectType,
                                                 const std::vector<std::string> parseFileList)
    {
        ProjectExplorerInfo info;
        info.projectName = projectName;
        info.fileName = fileName;
        info.importType = importType;
        info.projectType = static_cast<int64_t>(projectType);
        for (const auto &item: parseFileList) {
            ParseFileInfo parseFileInfo;
            parseFileInfo.parseFilePath = item;
            info.parseFilePathInfos.push_back(parseFileInfo);
        }
        return info;
    }

    static void InitProjectExplorerData()
    {
        std::string filePathText = currPath.substr(0, index + 1) +
                                   R"(/src/test/test_data/test_rank_0/ASCEND_PROFILER_OUTPUT)";
        std::string filePathDb = currPath.substr(0, index + 1) +
                                 R"(/src/test/test_data/full_db/ascend_pytorch_profiler.db)";
        std::vector<ProjectExplorerInfo> infos;
        std::vector<std::string> parseFileList {filePathText};
        ProjectExplorerInfo info = CreateProjectData("testProject", "projectFilePath",
                                                     "import", Dic::ProjectTypeEnum::TEXT_CLUSTER, parseFileList);
        infos.push_back(info);
        ProjectExplorerManager::Instance().SaveProjectExplorer(infos, false);

        std::vector<ProjectExplorerInfo> dbInfos;
        std::vector<std::string> parseDbFileList {filePathDb};
        ProjectExplorerInfo dbInfo = CreateProjectData("testProjectDb", "projectFilePathDb",
                                                       "import", Dic::ProjectTypeEnum::DB, parseDbFileList);
        dbInfos.push_back(dbInfo);
        ProjectExplorerManager::Instance().SaveProjectExplorer(dbInfos, false);
    }
};

TEST_F(OperatorRequestHandlerTest, QueryOpCategoryInfoHandlerNormalTest)
{
    Dic::Module::Operator::QueryOpCategoryInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorCategoryInfoRequest>();
    requestPtr.get()->params.rankId = "0";
    requestPtr.get()->params.group = "Operator";
    requestPtr.get()->params.topK = -1;
    ASSERT_NO_THROW(handler.HandleRequest(std::move(requestPtr)));
}

TEST_F(OperatorRequestHandlerTest, QueryOpComputeUnitHandlerNormalTest)
{
    Dic::Module::Operator::QueryOpComputeUnitHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorComputeUnitInfoRequest>();
    requestPtr.get()->params.rankId = "0";
    ASSERT_NO_THROW(handler.HandleRequest(std::move(requestPtr)));
}

TEST_F(OperatorRequestHandlerTest, QueryOpStatisticInfoHandlerNormalTest)
{
    Dic::Module::Operator::QueryOpStatisticInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorStatisticInfoRequest>();
    ASSERT_NO_THROW(handler.HandleRequest(std::move(requestPtr)));
}

TEST_F(OperatorRequestHandlerTest, QueryOpStatisticInfoHandlerCmplTest)
{
    Dic::Module::Operator::QueryOpStatisticInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorStatisticInfoRequest>();
    requestPtr.get()->params.rankId = "0";
    requestPtr.get()->params.group = "Operator Type";
    requestPtr.get()->params.topK = -1;
    requestPtr.get()->params.isCompare = true;
    // 10 表示分页最小是10条
    requestPtr.get()->params.pageSize = 10;
    requestPtr.get()->params.current = 1;
    ASSERT_NO_THROW(handler.HandleRequest(std::move(requestPtr)));
}

TEST_F(OperatorRequestHandlerTest, QueryOpDetailInfoHandlerNormalTest)
{
    Dic::Module::Operator::QueryOpDetailInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorDetailInfoRequest>();
    ASSERT_NO_THROW(handler.HandleRequest(std::move(requestPtr)));
}

TEST_F(OperatorRequestHandlerTest, QueryOpDetailInfoHandlerNormal2Test)
{
    Dic::Module::Operator::QueryOpDetailInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorDetailInfoRequest>();
    const uint64_t ten = 10;
    const uint64_t oneOneOne = 100;
    const uint64_t one = 1;
    requestPtr->params.topK = ten;
    requestPtr->params.pageSize = oneOneOne;
    requestPtr->params.current = one;
    requestPtr->params.rankId = "1";
    ASSERT_NO_THROW(handler.HandleRequest(std::move(requestPtr)));
}

TEST_F(OperatorRequestHandlerTest, QueryOpDetailInfoHandlerNormal3Test)
{
    Dic::Module::Operator::QueryOpDetailInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorDetailInfoRequest>();
    const uint64_t ten = 10;
    const uint64_t oneOneOne = 100;
    const uint64_t one = 1;
    requestPtr->params.topK = ten;
    requestPtr->params.pageSize = oneOneOne;
    requestPtr->params.current = one;
    requestPtr->params.rankId = "1";
    requestPtr->params.isCompare = true;
    ASSERT_NO_THROW(handler.HandleRequest(std::move(requestPtr)));
}

TEST_F(OperatorRequestHandlerTest, QueryOpMoreInfoHandlerNormalTest)
{
    Dic::Module::Operator::QueryOpMoreInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorMoreInfoRequest>();
    ASSERT_NO_THROW(handler.HandleRequest(std::move(requestPtr)));
}

TEST_F(OperatorRequestHandlerTest, QueryOpStatisticInfoHandlerSuccessWhenBaselineIsDbGroupByOperatorType)
{
    InitDbManager();
    Dic::Module::Operator::QueryOpStatisticInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorStatisticInfoRequest>();
    requestPtr.get()->params.rankId = "2";
    requestPtr.get()->params.group = "Operator Type";
    requestPtr.get()->params.topK = -1; // -1 表示topK取全部数据
    requestPtr.get()->params.isCompare = true;
    // 10 表示分页最小是10条
    requestPtr.get()->params.pageSize = 10;
    requestPtr.get()->params.current = 1;

    EXPECT_TRUE(SetBaseLineManager());
    EXPECT_TRUE(handler.HandleRequest(std::move(requestPtr)));
    ClearProjectExplorerData();
}

TEST_F(OperatorRequestHandlerTest, QueryOpStatisticInfoHandlerSuccessAndOrderByOpTypeAndTopKIs15GroupByInputShape)
{
    InitDbManager();
    Dic::Module::Operator::QueryOpStatisticInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorStatisticInfoRequest>();
    requestPtr.get()->params.rankId = "2";
    requestPtr.get()->params.group = "Input Shape";
    requestPtr.get()->params.topK = 15;  // 15表示topK取15条数据
    requestPtr.get()->params.isCompare = true;
    requestPtr.get()->params.orderBy = "op_type";
    // 10 表示分页最小是10条
    requestPtr.get()->params.pageSize = 10;
    requestPtr.get()->params.current = 1;

    EXPECT_TRUE(SetBaseLineManager());
    EXPECT_TRUE(handler.HandleRequest(std::move(requestPtr)));
    ClearProjectExplorerData();
}

TEST_F(OperatorRequestHandlerTest, QueryOpStatisticInfoHandlerSuccessWhenBaselineIsDbGroupByHCCLOperatorType)
{
    InitDbManager();
    Dic::Module::Operator::QueryOpStatisticInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorStatisticInfoRequest>();
    requestPtr.get()->params.rankId = "2";
    requestPtr.get()->params.group = "HCCL Operator Type";
    // topK给一个极大值
    requestPtr.get()->params.topK = 10000000; // 10000000表示topK是一个极大值
    requestPtr.get()->params.isCompare = true;
    requestPtr.get()->params.orderBy = "count";
    requestPtr.get()->params.order = "ascend";
    // 10 表示分页最小是10条
    requestPtr.get()->params.pageSize = 10;
    requestPtr.get()->params.current = 1;

    EXPECT_TRUE(SetBaseLineManager());
    EXPECT_TRUE(handler.HandleRequest(std::move(requestPtr)));
    ClearProjectExplorerData();
}

TEST_F(OperatorRequestHandlerTest, QueryOpStatisticInfoHandlerSuccessGroupByHCCLOperatorTypeOrderDesc)
{
    InitDbManager();
    Dic::Module::Operator::QueryOpStatisticInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorStatisticInfoRequest>();
    requestPtr.get()->params.rankId = "2";
    requestPtr.get()->params.group = "HCCL Operator Type";
    // topK给一个极大值
    requestPtr.get()->params.topK = 10000000; // 10000000表示topK是一个极大值
    requestPtr.get()->params.isCompare = true;
    requestPtr.get()->params.orderBy = "count";
    requestPtr.get()->params.order = "descend";
    // 10 表示分页最小是10条
    requestPtr.get()->params.pageSize = 10;
    requestPtr.get()->params.current = 1;

    EXPECT_TRUE(SetBaseLineManager());
    EXPECT_TRUE(handler.HandleRequest(std::move(requestPtr)));
    ClearProjectExplorerData();
}

// QueryOpDetailInfoHandler 测试
TEST_F(OperatorRequestHandlerTest, QueryOpDetailInfoHandlerFailedWhenBsesLineIsNotSet)
{
    Dic::Module::Operator::QueryOpDetailInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorDetailInfoRequest>();
    requestPtr.get()->params.rankId = "2";
    requestPtr.get()->params.group = "Operator";
    // topK给一个极大值
    requestPtr.get()->params.topK = 10000000; // 10000000表示topK是一个极大值
    requestPtr.get()->params.isCompare = true;
    requestPtr.get()->params.orderBy = "count";
    requestPtr.get()->params.order = "descend";
    // 10 表示分页最小是10条
    requestPtr.get()->params.pageSize = 10;
    requestPtr.get()->params.current = 1;

    EXPECT_FALSE(handler.HandleRequest(std::move(requestPtr)));
}
