/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include <gtest/gtest.h>
#include "TraceFileParser.h"
#include "ClusterFileParser.h"
#include "FileUtil.h"
#include "DataBaseManager.h"
#include "ServerLog.h"
#include "ParamsParser.h"
#include "ParserStatusManager.h"

using namespace Dic::Module::Timeline;
class TestSuit : public ::testing::Test {
protected:
    static void SetUpTestCase()
    {
        std::string currPath = Dic::FileUtil::GetCurrPath();
        const ParamsOption &option = ParamsParser::Instance().GetOption();
        ServerLog::Initialize(option.logPath, option.logSize, option.logLevel, to_string(option.wsPort));
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        Dic::Module::Timeline::DataBaseManager::Instance()
        .CreatConnectionPool("0", currPath + R"(/src/test/test_data/test_rank_0/ASCEND_PROFILER_OUTPUT/trace_view.db)");
        Dic::Module::Timeline::DataBaseManager::Instance()
        .CreatConnectionPool("1", currPath + R"(/src/test/test_data/test_rank_1/ASCEND_PROFILER_OUTPUT/trace_view.db)");
        Dic::Module::Timeline::TraceFileParser::Instance().Parse(
            {currPath + R"(/src/test/test_data/test_rank_0/ASCEND_PROFILER_OUTPUT/trace_view.json)"}, "0", "");
        Dic::Module::Timeline::TraceFileParser::Instance().Parse(
            {currPath + R"(/src/test/test_data/test_rank_1/ASCEND_PROFILER_OUTPUT/trace_view.json)"}, "1", "");
        Dic::Module::Timeline::ClusterFileParser clusterFileParser;
        clusterFileParser.ParseClusterFiles(currPath + R"(/src/test/test_data)");
        int interval = 2000;
        while (true) {
            ParserStatus status0 = ParserStatusManager::Instance().GetParserStatus("0");
            ParserStatus status1 = ParserStatusManager::Instance().GetParserStatus("1");
            if (status0 == ParserStatus::FINISH_ALL && status1 == ParserStatus::FINISH_ALL) {
                Dic::Server::ServerLog::Info("parse end");
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                break;
            }
        }
    }

    static void TearDownTestCase()
    {
        Dic::Module::Timeline::TraceFileParser::Instance().DeleteParseFile("0");
        Dic::Module::Timeline::TraceFileParser::Instance().DeleteParseFile("1");
        Dic::Module::Timeline::DataBaseManager::Instance().ClearClusterDb();
        std::string tempPath = Dic::FileUtil::GetCurrPath();
        int index = tempPath.find_last_of("server");
        tempPath = tempPath.substr(0, index + 1);
        if (std::remove((tempPath + R"(/src/test/test_data)" + "/cluster.db").c_str()) != 0) {
            Dic::Server::ServerLog::Info("无法删除文件");
        } else {
            Dic::Server::ServerLog::Info("文件已成功删除");
        }
    }

    static int Main(int argc, char** argv)
    {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }
};
