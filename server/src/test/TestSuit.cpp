/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

#include "TestSuit.h"

using namespace Dic::Module::Timeline;
using namespace Dic::Module::Summary;
using namespace Dic::Module::Memory;
using namespace Dic;

std::string TestSuit::clusterPath;
void TestSuit::SetUpTestSuite()
{
    std::string currPath = Dic::FileUtil::GetCurrPath();
    const ParamsOption &option = ParamsParser::Instance().GetOption();
    ServerLog::Initialize(option.logPath, option.logSize, option.logLevel, to_string(option.wsPort));
    int index = currPath.find_last_of("server");
    currPath = currPath.substr(0, index + 1);
    std::string refPath0 = R"(/src/test/test_data/test_rank_0/ASCEND_PROFILER_OUTPUT/)";
    std::string refPath1 = R"(/src/test/test_data/test_rank_1/ASCEND_PROFILER_OUTPUT/)";
    std::string dbPath0 = currPath + refPath0 + "mindstudio_insight_data.db";
    std::string dbPath1 = currPath + refPath1 + "mindstudio_insight_data.db";
    DataBaseManager::Instance().SetDataType(DataType::TEXT, dbPath0);
    DataBaseManager::Instance().SetFileType(FileType::PYTORCH, dbPath0);
    DataBaseManager::Instance().SetDataType(DataType::TEXT, dbPath1);
    DataBaseManager::Instance().SetFileType(FileType::PYTORCH, dbPath1);
    DataBaseManager::Instance().CreateTraceConnectionPool("0", dbPath0);
    DataBaseManager::Instance().CreateTraceConnectionPool("1", dbPath1);
    DataBaseManager::Instance().UpdateRankIdToDeviceId(dbPath0, "0", "0");
    DataBaseManager::Instance().UpdateRankIdToDeviceId(dbPath1, "1", "1");
    TraceFileParser::Instance().Parse({currPath + refPath0 + "trace_view.json"}, "0", "",
                                      dbPath0);
    WaitParseEnd({"0"});
    TraceFileParser::Instance().Parse({currPath + refPath1 + "trace_view.json"}, "1", "",
                                      dbPath1);
    WaitParseEnd({"1"});
    std::string testDataPath = currPath + R"(/src/test/test_data)";
    KernelParse::Instance().Parse({dbPath0, "0", currPath + refPath0});
    KernelParse::Instance().Parse({dbPath1, "1", currPath + refPath1});
    WaitParseEnd({KERNEL_PREFIX + "0", KERNEL_PREFIX + "1"});
    MemoryParse::Instance().Parse({dbPath0, "0", currPath + refPath0});
    MemoryParse::Instance().Parse({dbPath1, "1", currPath + refPath1});
    WaitParseEnd({MEMORY_PREFIX + "0", MEMORY_PREFIX + "1"});

    clusterPath = testDataPath + R"(/cluster_analysis_output)";
    // database先传空指针，等完成mstt分析之后再对该指针赋值
    ClusterFileParser clusterFileParser(clusterPath, nullptr, COMPARE + TimeUtil::Instance().NowStr());
    clusterFileParser.ParseClusterFiles();
    clusterFileParser.ParseClusterStep2Files();
    WaitParseEnd({"0", "1", KERNEL_PREFIX + "0",
                  KERNEL_PREFIX + "1", MEMORY_PREFIX + "0", MEMORY_PREFIX + "1"});
}

void TestSuit::WaitParseEnd(std::vector<std::string> statusList)
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

void TestSuit::TearDownTestSuite()
{
    KernelParse::Instance().Reset();
    TraceFileParser::Instance().DeleteParseFiles({"0", "1"});
    DataBaseManager::Instance().EraseClusterDb(COMPARE);
    std::string tempPath = Dic::FileUtil::GetCurrPath();
    int index = tempPath.find_last_of("server");
    tempPath = tempPath.substr(0, index + 1);
    if (std::remove((tempPath + R"(/src/test/test_data)" + "/cluster.db").c_str()) != 0) {
        Dic::Server::ServerLog::Info("Delete file failed");
    } else {
        Dic::Server::ServerLog::Info("Delete file success");
    }
}

int TestSuit::Main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

std::shared_ptr<RenderEngine> TestSuit::GetRenderEngine()
{
    auto respotoryFactory = RepositoryFactory::Instance();
    auto dataEngine = DataEngine::Instance();
    dataEngine->SetRepositoryFactory(respotoryFactory);
    auto renderEngine = RenderEngine::Instance();
    renderEngine->SetDataEngineInterface(dataEngine);
    return renderEngine;
}
