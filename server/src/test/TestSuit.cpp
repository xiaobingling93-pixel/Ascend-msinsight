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
#include "KernelParse.h"
#include "MemoryParse.h"
#include "FullDbParser.h"
#include "DbMemoryDataBase.h"
#include "DbSummaryDataBase.h"


using namespace Dic::Module::Timeline;
using namespace Dic::Module::Summary;
using namespace Dic::Module::Memory;

class TestSuit : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        std::string currPath = Dic::FileUtil::GetCurrPath();
        const ParamsOption &option = ParamsParser::Instance().GetOption();
        ServerLog::Initialize(option.logPath, option.logSize, option.logLevel, to_string(option.wsPort));
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        std::string refPath0 = R"(/src/test/test_data/test_rank_0/ASCEND_PROFILER_OUTPUT/)";
        std::string refPath1 = R"(/src/test/test_data/test_rank_1/ASCEND_PROFILER_OUTPUT/)";
        DataBaseManager::Instance().SetDataType(DataType::JSON);
        DataBaseManager::Instance().CreatConnectionPool("0", currPath + refPath0 + "mindstudio_insight_data.db");
        DataBaseManager::Instance().CreatConnectionPool("1", currPath + refPath1 + "mindstudio_insight_data.db");
        TraceFileParser::Instance().Parse({currPath + refPath0 + "trace_view.json"}, "0", "");
        WaitParseEnd({"0"});
        TraceFileParser::Instance().Parse({currPath + refPath1 + "trace_view.json"}, "1", "");
        WaitParseEnd({"1"});
        std::string testDataPath = currPath + R"(/src/test/test_data)";
        KernelParse::Instance().Parse({testDataPath}, "");
        WaitParseEnd({KERNEL_PREFIX + "0", KERNEL_PREFIX + "1"});
        MemoryParse::Instance().Parse({testDataPath}, "");
        WaitParseEnd({MEMORY_PREFIX + "0", MEMORY_PREFIX + "1"});

        ClusterFileParser clusterFileParser;
        clusterFileParser.ParseClusterFiles(testDataPath);
        clusterFileParser.ParseClusterStep2Files(testDataPath);
        WaitParseEnd({"0", "1", KERNEL_PREFIX + "0",
                     KERNEL_PREFIX + "1", MEMORY_PREFIX + "0", MEMORY_PREFIX + "1"});
    }

    static void WaitParseEnd(std::vector<std::string> statusList)
    {
        while (true) {
            int i = 0;
            for (const auto& tmp : statusList) {
                if (ParserStatusManager::Instance().GetParserStatus(tmp) != ParserStatus::FINISH) {
                    break;
                } else {
                    i++;
                }
            }
            if (i < statusList.size()) {
                continue;
            } else {
                Dic::Server::ServerLog::Info("parse end");
                return;
            }
        }
    }

    static void TearDownTestCase()
    {
        KernelParse::Instance().Reset();
        TraceFileParser::Instance().DeleteParseFiles({"0", "1"});
        DataBaseManager::Instance().ClearClusterDb();
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
