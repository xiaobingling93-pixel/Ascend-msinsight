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
#include "ExportOpDetailsHandler.h"
#include "ParamsParser.h"
#include "ProjectExplorerManager.h"
#include "WsSessionImpl.h"
#include "RenderEngine.h"
#include "../../../TestSuit.cpp"


using namespace Dic::Server;
using namespace Dic::Module::Timeline;
using namespace Dic::Module::FullDb;
using namespace Dic::Module::Global;
using namespace Dic::Module::Operator;
using namespace Dic::Module::FullDb;

class OperatorRequestHandlerTest : public TestSuit {
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
        DataBaseManager::Instance().Clear();
        BaselineManagerService::ResetBaseline();
    }

    static void InitDbManager()
    {
        DataBaseManager::Instance().Clear();
        std::string currPath = Dic::FileUtil::GetCurrPath();
        const ParamsOption &option = ParamsParser::Instance().GetOption();
        ServerLog::Initialize(option.logPath, option.logSize, option.logLevel, to_string(option.wsPort));
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        std::string dbPath3 = R"(/src/test/test_data/full_db/)";
        DataBaseManager::Instance().SetDataType(DataType::DB);
        std::string fullDbPath = StringUtil::StrJoin(currPath, dbPath3, "msprof_0.db");
        auto summeryDatabase =
            std::dynamic_pointer_cast<DbSummaryDataBase, Dic::Module::Summary::VirtualSummaryDataBase>(
                DataBaseManager::Instance().CreateSummaryDatabase("2", fullDbPath));
        summeryDatabase->OpenDb(fullDbPath, false);
        auto renderEngine = GetRenderEngine();
        ASSERT_TRUE(renderEngine != nullptr);
        DataBaseManager::Instance().UpdateRankIdToDeviceId(fullDbPath, "2", "2");
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
        baselineInfo.parsedFilePath = filePathText;
        BaselineSettingRequest request;
        request.projectName = "testProject";
        request.params.projectName = "testProject";
        request.params.filePath = filePathText;
        request.params.currentClusterPath = COMPARE;
        bool result = BaselineManagerService::InitBaselineData(request, baselineInfo);
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
    inline static int retry = 2;
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
            auto parseFileInfo = std::make_shared<ParseFileInfo>();
            parseFileInfo->parseFilePath = item;
            parseFileInfo->type = ParseFileType::RANK;
            parseFileInfo->subId = item;
            info.AddSubParseFileInfo(parseFileInfo);
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
        std::for_each(infos.begin(), infos.end(), [](const auto& item) {
            ProjectExplorerManager::Instance().SaveProjectExplorer(item, false);
        });

        std::vector<ProjectExplorerInfo> dbInfos;
        std::vector<std::string> parseDbFileList {filePathDb};
        ProjectExplorerInfo dbInfo = CreateProjectData("testProjectDb", "projectFilePathDb",
                                                       "import", Dic::ProjectTypeEnum::DB, parseDbFileList);
        dbInfos.push_back(dbInfo);
        std::for_each(dbInfos.begin(), dbInfos.end(), [](const auto& item) {
            ProjectExplorerManager::Instance().SaveProjectExplorer(item, false);
        });
    }
};

TEST_F(OperatorRequestHandlerTest, QueryOpCategoryInfoHandlerNormalTest)
{
    Dic::Module::Operator::QueryOpCategoryInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorCategoryInfoRequest>();
    requestPtr->params.rankId = "0";
    requestPtr->params.group = "Operator";
    requestPtr->params.topK = -1;
    requestPtr->fileId = DataBaseManager::Instance().GetFileIdByRankId("0");
    ASSERT_NO_THROW(handler.HandleRequest(std::move(requestPtr)));
}

TEST_F(OperatorRequestHandlerTest, QueryOpCategoryInfoHandleEmptyRankId)
{
    Dic::Module::Operator::QueryOpCategoryInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorCategoryInfoRequest>();
    requestPtr.get()->params.rankId = "";
    requestPtr.get()->params.group = "Operator";
    requestPtr.get()->params.topK = -1;
    requestPtr->fileId = "";
    std::string errMsg;
    EXPECT_EQ(false, requestPtr->params.CommonCheck(errMsg));
}

TEST_F(OperatorRequestHandlerTest, QueryOpComputeUnitHandlerNormalTest)
{
    Dic::Module::Operator::QueryOpComputeUnitHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorComputeUnitInfoRequest>();
    requestPtr->params.rankId = "0";
    requestPtr->fileId = DataBaseManager::Instance().GetFileIdByRankId("0");
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
    requestPtr->params.rankId = "0";
    requestPtr->params.group = "Operator Type";
    requestPtr->params.topK = -1;
    requestPtr->params.isCompare = true;
    // 10 表示分页最小是10条
    requestPtr->params.pageSize = 10;
    requestPtr->params.current = 1;
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
    requestPtr->params.rankId = "2";
    requestPtr->params.group = "Operator Type";
    requestPtr->params.topK = -1; // -1 表示topK取全部数据
    requestPtr->params.isCompare = true;
    // 10 表示分页最小是10条
    requestPtr->params.pageSize = 10;
    requestPtr->params.current = 1;
    requestPtr->fileId = DataBaseManager::Instance().GetFileIdByRankId("2");
    EXPECT_TRUE(SetBaseLineManager());
    EXPECT_TRUE(handler.HandleRequest(std::move(requestPtr)));
    ClearProjectExplorerData();
}

TEST_F(OperatorRequestHandlerTest, QueryOpStatisticInfoHandlerSuccessAndOrderByOpTypeAndTopKIs15GroupByInputShape)
{
    InitDbManager();
    Dic::Module::Operator::QueryOpStatisticInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorStatisticInfoRequest>();
    requestPtr->params.rankId = "2";
    requestPtr->params.group = "Input Shape";
    requestPtr->params.topK = 15;  // 15表示topK取15条数据
    requestPtr->params.isCompare = true;
    requestPtr->params.orderBy = "opType";
    // 10 表示分页最小是10条
    requestPtr->params.pageSize = 10;
    requestPtr->params.current = 1;

    EXPECT_TRUE(SetBaseLineManager());
    EXPECT_TRUE(handler.HandleRequest(std::move(requestPtr)));
    ClearProjectExplorerData();
}

TEST_F(OperatorRequestHandlerTest, QueryOpStatisticInfoHandlerSuccessWhenBaselineIsDbGroupByHCCLOperatorType)
{
    InitDbManager();
    Dic::Module::Operator::QueryOpStatisticInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorStatisticInfoRequest>();
    requestPtr->params.rankId = "2";
    requestPtr->params.group = "Communication Operator Type";
    // topK给一个极大值
    requestPtr->params.topK = 10000000; // 10000000表示topK是一个极大值
    requestPtr->params.isCompare = true;
    requestPtr->params.orderBy = "count";
    requestPtr->params.order = "ascend";
    // 10 表示分页最小是10条
    requestPtr->params.pageSize = 10;
    requestPtr->params.current = 1;

    EXPECT_TRUE(SetBaseLineManager());
    EXPECT_TRUE(handler.HandleRequest(std::move(requestPtr)));
    ClearProjectExplorerData();
}

TEST_F(OperatorRequestHandlerTest, QueryOpStatisticInfoHandlerSuccessGroupByHCCLOperatorTypeOrderDesc)
{
    InitDbManager();
    Dic::Module::Operator::QueryOpStatisticInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorStatisticInfoRequest>();
    requestPtr->params.rankId = "2";
    requestPtr->params.group = "Communication Operator Type";
    // topK给一个极大值
    requestPtr->params.topK = 10000000; // 10000000表示topK是一个极大值
    requestPtr->params.isCompare = true;
    requestPtr->params.orderBy = "count";
    requestPtr->params.order = "descend";
    // 10 表示分页最小是10条
    requestPtr->params.pageSize = 10;
    requestPtr->params.current = 1;

    EXPECT_TRUE(SetBaseLineManager());
    EXPECT_TRUE(handler.HandleRequest(std::move(requestPtr)));
    ClearProjectExplorerData();
}

// QueryOpDetailInfoHandler 测试
TEST_F(OperatorRequestHandlerTest, QueryOpDetailInfoHandlerFailedWhenBsesLineIsNotSet)
{
    Dic::Module::Operator::QueryOpDetailInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorDetailInfoRequest>();
    requestPtr->params.rankId = "2";
    requestPtr->params.group = "Operator";
    // topK给一个极大值
    requestPtr->params.topK = 10000000; // 10000000表示topK是一个极大值
    requestPtr->params.isCompare = true;
    requestPtr->params.orderBy = "count";
    requestPtr->params.order = "descend";
    // 10 表示分页最小是10条
    requestPtr->params.pageSize = 10;
    requestPtr->params.current = 1;

    EXPECT_FALSE(handler.HandleRequest(std::move(requestPtr)));
}

// ExportOpDetailsHandler 测试
TEST_F(OperatorRequestHandlerTest, ExportOpDetailsHandlerSuccessGroupByOperatorIsNotCompare)
{
    InitDbManager();
    Dic::Module::Operator::ExportOpDetailsHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorExportDetailsRequest>();
    requestPtr->params.rankId = "2";
    requestPtr->params.group = "Operator";
    // topK给一个极大值
    requestPtr->params.topK = INT64_MAX;

    EXPECT_TRUE(SetBaseLineManager());
    EXPECT_TRUE(handler.HandleRequest(std::move(requestPtr)));
    ClearProjectExplorerData();
}

TEST_F(OperatorRequestHandlerTest, ExportOpDetailsHandlerSuccessGroupByOperatorTypeIsNotCompare)
{
    InitDbManager();
    Dic::Module::Operator::ExportOpDetailsHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorExportDetailsRequest>();
    requestPtr->params.rankId = "2";
    requestPtr->params.group = "Operator Type";
    // topK给一个极大值
    requestPtr->params.topK = INT64_MAX;

    EXPECT_TRUE(SetBaseLineManager());
    EXPECT_TRUE(handler.HandleRequest(std::move(requestPtr)));
    ClearProjectExplorerData();
}

TEST_F(OperatorRequestHandlerTest, ExportOpDetailsHandlerSuccessGroupByInputShapeIsNotCompare)
{
    InitDbManager();
    Dic::Module::Operator::ExportOpDetailsHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorExportDetailsRequest>();
    requestPtr->params.rankId = "2";
    requestPtr->params.group = "Input Shape";
    // topK给一个极大值
    requestPtr->params.topK = INT64_MAX;

    EXPECT_TRUE(SetBaseLineManager());
    EXPECT_TRUE(handler.HandleRequest(std::move(requestPtr)));
    ClearProjectExplorerData();
}

TEST_F(OperatorRequestHandlerTest, ExportOpDetailsHandlerSuccessGroupByCommunicationOperatorIsNotCompare)
{
    InitDbManager();
    Dic::Module::Operator::ExportOpDetailsHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorExportDetailsRequest>();
    requestPtr->params.rankId = "2";
    requestPtr->params.group = "Communication Operator";
    // topK给一个极大值
    requestPtr->params.topK = INT64_MAX;

    EXPECT_TRUE(SetBaseLineManager());
    EXPECT_TRUE(handler.HandleRequest(std::move(requestPtr)));
    ClearProjectExplorerData();
}

TEST_F(OperatorRequestHandlerTest, ExportOpDetailsHandlerSuccessGroupByCommunicationOperatorTypeIsNotCompare)
{
    InitDbManager();
    Dic::Module::Operator::ExportOpDetailsHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorExportDetailsRequest>();
    requestPtr->params.rankId = "2";
    requestPtr->params.group = "Communication Operator Type";
    // topK给一个极大值
    requestPtr->params.topK = INT64_MAX;

    EXPECT_TRUE(SetBaseLineManager());
    EXPECT_TRUE(handler.HandleRequest(std::move(requestPtr)));
    ClearProjectExplorerData();
}


TEST_F(OperatorRequestHandlerTest, ExportOpDetailsHandlerSuccessGroupByOperatorTypeIsCompare)
{
    InitDbManager();
    Dic::Module::Operator::ExportOpDetailsHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorExportDetailsRequest>();
    requestPtr->params.isCompare = true;
    requestPtr->params.rankId = "2";
    requestPtr->params.group = "Operator Type";
    // topK给一个极大值
    requestPtr->params.topK = INT64_MAX;

    EXPECT_TRUE(SetBaseLineManager());
    EXPECT_TRUE(handler.HandleRequest(std::move(requestPtr)));
    ClearProjectExplorerData();
}

TEST_F(OperatorRequestHandlerTest, ExportOpDetailsHandlerSuccessGroupByInputShapeIsCompare)
{
    InitDbManager();
    Dic::Module::Operator::ExportOpDetailsHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorExportDetailsRequest>();
    requestPtr->params.isCompare = true;
    requestPtr->params.rankId = "2";
    requestPtr->params.group = "Input Shape";
    // topK给一个极大值
    requestPtr->params.topK = INT64_MAX;

    EXPECT_TRUE(SetBaseLineManager());
    EXPECT_TRUE(handler.HandleRequest(std::move(requestPtr)));
    ClearProjectExplorerData();
}

TEST_F(OperatorRequestHandlerTest, ExportOpDetailsHandlerSuccessGroupByCommunicationOperatorIsCompare)
{
    InitDbManager();
    Dic::Module::Operator::ExportOpDetailsHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorExportDetailsRequest>();
    requestPtr->params.isCompare = true;
    requestPtr->params.rankId = "2";
    requestPtr->params.group = "Communication Operator";
    // topK给一个极大值
    requestPtr->params.topK = INT64_MAX;

    EXPECT_TRUE(SetBaseLineManager());
    EXPECT_TRUE(handler.HandleRequest(std::move(requestPtr)));
    ClearProjectExplorerData();
}

TEST_F(OperatorRequestHandlerTest, ExportOpDetailsHandlerSuccessGroupByCommunicationOperatorTypeIsCompare)
{
    InitDbManager();
    Dic::Module::Operator::ExportOpDetailsHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorExportDetailsRequest>();
    requestPtr->params.isCompare = true;
    requestPtr->params.rankId = "2";
    requestPtr->params.group = "Communication Operator Type";
    // topK给一个极大值
    requestPtr->params.topK = INT64_MAX;

    EXPECT_TRUE(SetBaseLineManager());
    EXPECT_TRUE(handler.HandleRequest(std::move(requestPtr)));
    ClearProjectExplorerData();
}

TEST_F(OperatorRequestHandlerTest, ExportOpDetailsHandlerFailTopKIsIllegal)
{
    InitDbManager();
    Dic::Module::Operator::ExportOpDetailsHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorExportDetailsRequest>();
    requestPtr->params.isCompare = true;
    requestPtr->params.rankId = "2";
    requestPtr->params.group = "Communication Operator Type";
    // topK给一个极大值
    requestPtr->params.topK = INT64_MIN;

    EXPECT_FALSE(handler.HandleRequest(std::move(requestPtr)));
    ClearProjectExplorerData();
}

TEST_F(OperatorRequestHandlerTest, ExportOpDetailsHandlerGroupByIsIllegal)
{
    InitDbManager();
    Dic::Module::Operator::ExportOpDetailsHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorExportDetailsRequest>();
    requestPtr->params.isCompare = true;
    requestPtr->params.rankId = "2";
    requestPtr->params.group = "UnKnow";
    // topK给一个极大值
    requestPtr->params.topK = INT64_MAX;

    EXPECT_FALSE(handler.HandleRequest(std::move(requestPtr)));
    ClearProjectExplorerData();
}

TEST_F(OperatorRequestHandlerTest, QueryOpDetailInfoHandlerFailedWithabnormalTopK)
{
    Dic::Module::Operator::QueryOpDetailInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorDetailInfoRequest>();
    requestPtr->params.group = "Operator";
    // abnormal topK -5
    requestPtr->params.topK = -5;
    EXPECT_FALSE(handler.HandleRequest(std::move(requestPtr)));
}

TEST_F(OperatorRequestHandlerTest, QueryOpDetailInfoHandlerFailedWithabnormalPagesize)
{
    Dic::Module::Operator::QueryOpDetailInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorDetailInfoRequest>();
    requestPtr->params.group = "Operator";
    // normal topK 10
    requestPtr->params.topK = 10;
    requestPtr->params.pageSize = MAX_PAGESIZE + 1;
    EXPECT_FALSE(handler.HandleRequest(std::move(requestPtr)));
}

TEST_F(OperatorRequestHandlerTest, QueryOpDetailInfoHandlerFailedWithabnormalCurrent)
{
    Dic::Module::Operator::QueryOpDetailInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorDetailInfoRequest>();
    requestPtr->params.group = "Operator";
    // normal topK 10
    requestPtr->params.topK = 10;
    // normal pageSize 10
    requestPtr->params.pageSize = 10;
    requestPtr->params.current = MIN_CURRENT_PAGE;
    EXPECT_FALSE(handler.HandleRequest(std::move(requestPtr)));
}

TEST_F(OperatorRequestHandlerTest, QueryOpDetailInfoHandlerFailedWithabnormalRankid)
{
    Dic::Module::Operator::QueryOpDetailInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorDetailInfoRequest>();
    requestPtr->params.group = "Operator";
    // normal topK 10
    requestPtr->params.topK = 10;
    // normal pageSize 10
    requestPtr->params.pageSize = 10;
    requestPtr->params.current = 1;
    requestPtr->params.rankId = "";
    EXPECT_FALSE(handler.HandleRequest(std::move(requestPtr)));
}

TEST_F(OperatorRequestHandlerTest, QueryOpDetailInfoHandlerFailedWithabnormalOrder)
{
    Dic::Module::Operator::QueryOpDetailInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorDetailInfoRequest>();
    requestPtr->params.group = "Operator";
    // normal topK 10
    requestPtr->params.topK = 10;
    // normal pageSize 10
    requestPtr->params.pageSize = 10;
    requestPtr->params.current = 1;
    requestPtr->params.rankId = "2";
    requestPtr->params.orderBy = "";
    requestPtr->params.order = "";
    ASSERT_NO_THROW(handler.HandleRequest(std::move(requestPtr)));
}

TEST_F(OperatorRequestHandlerTest, QueryOpDetailInfoHandlerFailedWithabnormalQuery)
{
    Dic::Module::Operator::QueryOpDetailInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorDetailInfoRequest>();
    requestPtr->params.group = "Operator";
    // normal topK 10
    requestPtr->params.topK = 10;
    // normal pageSize 10
    requestPtr->params.pageSize = 10;
    requestPtr->params.current = 1;
    requestPtr->params.rankId = "0";
    requestPtr->params.orderBy = "count";
    requestPtr->params.order = "descend";
    requestPtr->params.isCompare = false;
    EXPECT_FALSE(handler.HandleRequest(std::move(requestPtr)));
}
