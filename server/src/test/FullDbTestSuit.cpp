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
#include "modules/full_db/Mock/MockSession.h"

using namespace Dic::Module::Timeline;
using namespace Dic::Module::FullDb;
using namespace Dic::Module;

class FullDbTestSuit : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        FullDb::FullDbParser::Instance().Reset();
        std::string currPath = Dic::FileUtil::GetCurrPath();
        const ParamsOption &option = ParamsParser::Instance().GetOption();
        ServerLog::Initialize(option.logPath, option.logSize, option.logLevel, to_string(option.wsPort));
        std::unique_ptr<MockSession> session = std::make_unique<MockSession>();
        WsSessionManager::Instance().AddSession(std::move(session));
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        std::string dbPath3 = currPath + R"(/src/test/test_data/full_db/msprof_0.db)";
        DataBaseManager::Instance().SetDataType(DataType::DB);
        std::pair<std::string, ParserType> parserType = std::make_pair(dbPath3, ParserType::DB);
        ParserType allocType = parserType.second;
        std::shared_ptr<ParserAlloc> factory = ParserFactory::ParserImport(allocType);
        // 路径列表不为空，需要进行文件目录的新增、覆盖
        ProjectTypeEnum projectType = factory->GetProjectType({ dbPath3 });
        std::vector<Global::ProjectExplorerInfo> projectExplorerInfoList;

        std::string warn;
        // 获取文件列表
        std::vector<std::string> parseFileList = factory->GetParseFileByImportFile(dbPath3, projectType, warn);
        Global::ProjectExplorerInfo projectExplorerInfo;
        projectExplorerInfo.fileName = dbPath3;
        projectExplorerInfo.projectName = dbPath3;
        projectExplorerInfo.projectType = static_cast<int64_t>(projectType);
        projectExplorerInfo.importType = "import";
        Global::ParseFileInfo parseFileInfo;
        parseFileInfo.parseFilePath = dbPath3;
        projectExplorerInfo.parseFilePathInfos.push_back(parseFileInfo);
        projectExplorerInfoList.push_back(projectExplorerInfo);

        if (allocType != ParserType::JSON) {
            ParserFactory::Reset();
        }
        ImportActionRequest request;
        factory->Parser(projectExplorerInfoList, request);
        Timeline::DataBaseManager::Instance().SetDbPathMapping("FullDb", dbPath3, "");
        Timeline::DataBaseManager::Instance().SetDbPathMapping("2", dbPath3, "");
        while (ParserStatusManager::Instance().GetParserStatus("2") != ParserStatus::FINISH_ALL) {
        }
    }

    static void TearDownTestCase()
    {
        auto connList = Timeline::DataBaseManager::Instance().GetAllTraceDatabase();
        for (auto &conn : connList) {
            conn->Stop();
        }
        Timeline::DataBaseManager::Instance().Clear();
        Timeline::TraceTime::Instance().Reset();
        Timeline::ParserStatusManager::Instance().ClearAllParserStatus();
    }
};