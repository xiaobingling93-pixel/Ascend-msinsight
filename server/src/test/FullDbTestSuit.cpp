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
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        std::string dbPath3 = R"(/src/test/test_data/full_db/msprof_0.db)";
        DataBaseManager::Instance().SetDataType(DataType::DB);
        Timeline::DataBaseManager::Instance().CreatConnectionPool(currPath + dbPath3,
                                                                  currPath + dbPath3);
        Timeline::DataBaseManager::Instance().SetDbPathMapping("FullDb", currPath + dbPath3, "");
        Timeline::DataBaseManager::Instance().SetDbPathMapping("2", currPath + dbPath3, "");
        FullDb::FullDbParser::Instance().Parse({"FullDb"}, currPath + dbPath3);
        while (ParserStatusManager::Instance().GetParserStatus("FullDb") != ParserStatus::FINISH_ALL) {
        }
        auto database = std::dynamic_pointer_cast<DbTraceDataBase, Timeline::VirtualTraceDatabase>(
            Timeline::DataBaseManager::Instance().GetTraceDatabase("FullDb"));
        database->UpdateStartTime("FullDb");
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