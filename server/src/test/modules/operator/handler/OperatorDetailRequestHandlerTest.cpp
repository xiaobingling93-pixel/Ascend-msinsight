/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include <WsSessionManager.h>
#include "BaselineManagerService.h"
#include "DataBaseManager.h"
#include "QueryOpCategoryInfoHandler.h"
#include "QueryOpDetailInfoHandler.h"
#include "ParamsParser.h"
#include "ProjectExplorerManager.h"
#include "WsSessionImpl.h"
#include "../../../FullDbTestSuit.cpp"
#include "../../../TestSuit.cpp"

using namespace Dic::Server;
using namespace Dic::Module::Timeline;
using namespace Dic::Module::FullDb;
using namespace Dic::Module::Global;
class OperatorDetailRequestHandlerTest : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        Dic::Server::WsChannel *ws;
        std::unique_ptr<WsSessionImpl> session = std::make_unique<WsSessionImpl>(ws);
        Dic::Server::WsSessionManager::Instance().AddSession(std::move(session));
        SetBaseLineManager();
    }

    static void TearDownTestSuite()
    {
        auto session = Dic::Server::WsSessionManager::Instance().GetSession();
        if (session != nullptr) {
            session->SetStatus(Dic::Server::WsSession::Status::CLOSED);
            session->WaitForExit();
            Dic::Server::WsSessionManager::Instance().RemoveSession();
        }
        ClearProjectExplorerData();
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
        bool result = BaselineManagerService::InitBaselineData("testProject", filePathText, baselineInfo,
                                                               COMPARE);
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
            parseFileInfo->subId = item;
            parseFileInfo->type = ParseFileType::RANK;
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

TEST_F(OperatorDetailRequestHandlerTest, QueryOpDetailInfoHandlerFailedWhenBsesLineIsSetAndCompareIsTrue)
{
    Dic::Module::Operator::QueryOpDetailInfoHandler handler;
    auto requestPtr = std::make_unique<Dic::Protocol::OperatorDetailInfoRequest>();
    requestPtr.get()->params.rankId = "1";
    requestPtr.get()->params.group = "Operator";
    // topK给一个极大值
    requestPtr.get()->params.topK = 10000000; // 10000000表示topK是一个极大值
    requestPtr.get()->params.isCompare = true;
    requestPtr.get()->params.orderBy = "count";
    requestPtr.get()->params.order = "descend";
    // 10 表示分页最小是10条
    requestPtr.get()->params.pageSize = 10;
    requestPtr.get()->params.current = 1;

    EXPECT_TRUE(SetBaseLineManager());
    EXPECT_FALSE(handler.HandleRequest(std::move(requestPtr)));
}
