/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "DataBaseManager.h"
#include "ParamsParser.h"
#include "FullDbParser.h"
#include "ParserStatusManager.h"
#include "ProjectParserFactory.h"
#include "WsSessionManager.h"
#include "WsSessionImpl.h"
#include "RepositoryFactory.h"
#include "DataEngine.h"
#include "RenderEngine.h"

using namespace Dic::Module::Timeline;
using namespace Dic::Module::FullDb;
using namespace Dic::Module;
using namespace Dic;

class FullDbTestSuit : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        FullDb::FullDbParser::Instance().Reset();
        std::string currPath = Dic::FileUtil::GetCurrPath();
        const ParamsOption &option = ParamsParser::Instance().GetOption();
        ServerLog::Initialize(option.logPath, option.logSize, option.logLevel, to_string(option.wsPort));
        std::unique_ptr<Dic::Server::WsSession> session = std::make_unique<Dic::Server::WsSessionImpl>(nullptr);
        WsSessionManager::Instance().AddSession(std::move(session));
        auto respotoryFactory = RepositoryFactory::Instance();
        auto dataEngine = DataEngine::Instance();
        dataEngine->SetRepositoryFactory(respotoryFactory);
        auto renderEngine = RenderEngine::Instance();
        renderEngine->SetDataEngineInterface(dataEngine);
        const std::string server = "server";
        int index = currPath.find_last_of(server);
        currPath = currPath.substr(0, index - server.length()); // 取 server 前的文件路径
        std::string dbPath3 = currPath + R"(/server/src/test/test_data/full_db/msprof_0.db)";

        DataBaseManager::Instance().SetDataType(DataType::DB);
        std::pair<std::string, ParserType> parserType = std::make_pair(dbPath3, ParserType::DB);
        ParserType allocType = parserType.second;
        std::shared_ptr<ProjectParserBase> factory = ParserFactory::GetProjectParser(allocType);
        ProjectTypeEnum projectType = factory->GetProjectType(dbPath3);
        std::vector<Global::ProjectExplorerInfo> projectExplorerInfoList;

        std::string warn;
        std::vector<std::string> parseFileList = factory->GetParseFileByImportFile(dbPath3, warn);
        Global::ProjectExplorerInfo projectExplorerInfo;
        projectExplorerInfo.fileName = dbPath3;
        projectExplorerInfo.projectName = dbPath3;
        projectExplorerInfo.projectType = static_cast<int64_t>(projectType);
        projectExplorerInfo.importType = "import";
        auto parseFileInfo = std::make_shared<ParseFileInfo>();
        parseFileInfo->parseFilePath = dbPath3;
        projectExplorerInfo.subParseFileInfo.push_back(parseFileInfo);
        projectExplorerInfoList.push_back(projectExplorerInfo);

        if (allocType != ParserType::JSON) {
            ParserFactory::Reset();
        }
        ImportActionRequest request;
        // 从ImportActionHandler可以看出，Parser方法的第一个参数是vector，但永远只有一个元素，所以DT中不要传多个元素的vector
        factory->Parser(projectExplorerInfoList, request);

        Timeline::DataBaseManager::Instance().SetDbPathMapping("FullDb", dbPath3, "");
        Timeline::DataBaseManager::Instance().SetDbPathMapping("2", dbPath3, "");
        while (ParserStatusManager::Instance().GetParserStatus("2") != ParserStatus::FINISH_ALL) {
        }
    }

    static void TearDownTestSuite()
    {
        FullDbParser::Instance().Reset();
        auto session = Dic::Server::WsSessionManager::Instance().GetSession();
        if (session != nullptr) {
            session->SetStatus(Dic::Server::WsSession::Status::CLOSED);
            session->WaitForExit();
            Dic::Server::WsSessionManager::Instance().RemoveSession();
        }
    }
};