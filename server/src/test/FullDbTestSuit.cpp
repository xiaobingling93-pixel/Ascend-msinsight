/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "DataBaseManager.h"
#include "OperatorProtocolRequest.h"
#include "ParamsParser.h"
#include "FileUtil.h"
#include "FullDbParser.h"
#include "ParserStatusManager.h"
#include "TraceTime.h"
#include "ParserFactory.h"
#include "WsSessionManager.h"
#include "WsSessionImpl.h"

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
        const std::string server = "server";
        int index = currPath.find_last_of(server);
        currPath = currPath.substr(0, index - server.length()); // 取 server 前的文件路径
        std::string dbPath3 = currPath + R"(/server/src/test/test_data/full_db/msprof_0.db)";
        std::string dbPath4 = currPath +
            R"(/test/data/pytorch/db/level2/rank0_ascend_pt/ASCEND_PROFILER_OUTPUT/ascend_pytorch_profiler_0.db)";
        std::vector<std::string> dbPathes = { dbPath3, dbPath4 };

        DataBaseManager::Instance().SetDataType(DataType::DB);
        std::pair<std::string, ParserType> parserType = std::make_pair(dbPath3, ParserType::DB);
        ParserType allocType = parserType.second;
        std::shared_ptr<ParserAlloc> factory = ParserFactory::ParserImport(allocType);
        // 路径列表不为空，需要进行文件目录的新增、覆盖
        ProjectTypeEnum projectType = factory->GetProjectType(dbPathes);
        std::vector<Global::ProjectExplorerInfo> projectExplorerInfoList;

        for (const auto& path : dbPathes) {
            std::string warn;
            // 获取文件列表
            std::vector<std::string> parseFileList = factory->GetParseFileByImportFile(path, projectType, warn);
            Global::ProjectExplorerInfo projectExplorerInfo;
            projectExplorerInfo.fileName = path;
            projectExplorerInfo.projectName = path;
            projectExplorerInfo.projectType = static_cast<int64_t>(projectType);
            projectExplorerInfo.importType = "import";
            Global::ParseFileInfo parseFileInfo;
            parseFileInfo.parseFilePath = path;
            projectExplorerInfo.parseFilePathInfos.push_back(parseFileInfo);
            projectExplorerInfoList.push_back(projectExplorerInfo);
        }

        if (allocType != ParserType::JSON) {
            ParserFactory::Reset();
        }
        ImportActionRequest request;
        factory->Parser(projectExplorerInfoList, request);

        Timeline::DataBaseManager::Instance().SetDbPathMapping("FullDb", dbPath3, "");
        Timeline::DataBaseManager::Instance().SetDbPathMapping("FullDbNew", dbPath4, "");
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