/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include <gtest/gtest.h>
#include "ConstantDefs.h"
#include "ClusterFileParser.h"
#include "DataBaseManager.h"
#include "ExpertHotspotManager.h"
#include "ClusterDef.h"
#include "TimeUtil.h"
#include "DataBaseManager.h"
#include "TraceFileParser.h"
#include "ParserStatusManager.h"
#include "RepositoryFactory.h"
#include "DataEngine.h"
#include "RenderEngine.h"
#include "ProjectParserFactory.h"

using namespace Dic::Module::FullDb;
class ExpertHotspotManagerTest : public ::testing::Test {
protected:
    std::string filePath;
    std::string hotspotPath;
    std::string deploymentPath;
    std::string heatMapProfilingPath;
    void SetUp() override
    {
        std::string  currPath = Dic::FileUtil::GetCurrPath();
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        filePath = currPath + R"(/src/test/test_data/cluster_analysis_output)";
        hotspotPath = currPath + R"(/src/test/test_data/expert_hotspot)";
        deploymentPath = currPath + R"(/src/test/test_data/expert_deployment)";
        heatMapProfilingPath = currPath + R"(/src/test/test_data/heatMap/)";
    }

    static std::string InitParser(const std::string &dataPath, const std::string &uniqueKey)
    {
        // 创建新的db连接对象
        auto database = Dic::Module::FullDb::DataBaseManager::Instance()
            .CreateClusterDatabase(uniqueKey, Dic::Module::Timeline::DataType::TEXT);
        // 集群解析，如果集群已解析，则只会初始化db，然后结束流程
        Dic::Module::FullDb::ClusterFileParser clusterFileParser(dataPath, database,
                                                                 uniqueKey + Dic::TimeUtil::Instance().NowStr());
        clusterFileParser.ParseClusterFiles();
        clusterFileParser.ParseClusterStep2Files();
        return database->GetDbPath();
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

    static void InitProfiling(const std::string &filePath)
    {
        const std::string dbPath = filePath + "mindstudio_insight_data.db";
        DataBaseManager::Instance().SetDataType(DataType::TEXT, dbPath);
        DataBaseManager::Instance().CreateTraceConnectionPool("100", dbPath);
        TraceFileParser::Instance().Parse({filePath + "trace_view.json"}, "100", "", "");
        WaitParseEnd({"100"});
    }

    static void Clear()
    {
        auto db = Dic::Module::FullDb::DataBaseManager::Instance().GetClusterDatabase(Dic::COMPARE);
        if (db != nullptr) {
            // 清空热点数据
            db->DeleteExpertHotspot("decode", "1");
            db->DeleteExpertHotspot("prefill", "1");
            db->DeleteExpertHotspot("", "profiling");
            // baseinfo内容置0
            std::map<std::string, std::string> baseInfoMap;
            baseInfoMap[Dic::Module::KEY_DENSE_LAYER_LIST] = "";
            baseInfoMap[Dic::Module::KEY_MOE_LAYER] = "0";
            baseInfoMap[Dic::Module::KEY_RANK_NUMBER] = "0";
            baseInfoMap[Dic::Module::KEY_EXPERT_NUMBER] = "0";
            baseInfoMap[Dic::Module::KEY_MODEL_LAYER] = "0";
            db->InsertDuplicateUpdateBaseInfo(baseInfoMap);
        }
        Dic::Module::FullDb::DataBaseManager::Instance().EraseClusterDb(Dic::COMPARE);
        TraceFileParser::Instance().DeleteParseFiles({"100"});
    }
};

TEST_F(ExpertHotspotManagerTest, InitExpertHotspotDataSuccess)
{
    InitParser(filePath, Dic::COMPARE);
    std::string error;
    Dic::Module::Summary::ExpertHotspotManager::InitExpertHotspotData(hotspotPath, "1", error, COMPARE);
    auto res = Dic::Module::Summary::ExpertHotspotManager::QueryExpertHotspotData(COMPARE, "decode", "1");
    const int exceptSize = 232;
    EXPECT_EQ(res.size(), exceptSize);
    Clear();
}

TEST_F(ExpertHotspotManagerTest, UpdateModelInfoSuccess)
{
    InitParser(filePath, Dic::COMPARE);
    std::string error;
    Dic::Module::Summary::ExpertHotspotManager::InitExpertHotspotData(hotspotPath, "1", error, COMPARE);
    Dic::Module::ModelInfo modelInfo{{0, 2}, 0, 0, 4, 61};
    Dic::Module::Summary::ExpertHotspotManager::UpdateModelInfo(COMPARE, modelInfo, error);
    auto res = Dic::Module::Summary::ExpertHotspotManager::QueryExpertHotspotData(COMPARE, "decode", "1");
    const int exceptSize = 244;
    const int numberZero = 0;
    const int exceptVisits = 35661;
    const int numberOne = 1;
    const int numberFive = 5;
    const int numberEight = 8;
    EXPECT_EQ(res.size(), exceptSize);
    EXPECT_EQ(res[numberOne].visits, numberZero);
    EXPECT_EQ(res[numberFive].visits, exceptVisits);
    EXPECT_EQ(res[numberEight].visits, numberZero);
    EXPECT_EQ(res[res.size() - 1].visits, numberZero);
    Clear();
}

TEST_F(ExpertHotspotManagerTest, QueryWithoutHotspotData)
{
    InitParser(filePath, Dic::COMPARE);
    std::string error;
    Dic::Module::ModelInfo modelInfo{{0, 2}, 0, 0, 4, 61};
    Dic::Module::Summary::ExpertHotspotManager::UpdateModelInfo(COMPARE, modelInfo, error);
    auto res = Dic::Module::Summary::ExpertHotspotManager::QueryExpertHotspotData(COMPARE, "decode", "1");
    const int exceptSize = 244;
    const int numberZero = 0;
    EXPECT_EQ(res.size(), exceptSize);
    EXPECT_EQ(res[1].visits, numberZero);
    Clear();
}

TEST_F(ExpertHotspotManagerTest, InitExpertDeploymentDataSuccess)
{
    InitParser(filePath, Dic::COMPARE);
    std::string error;
    Dic::Module::Summary::ExpertHotspotManager::InitExpertHotspotData(deploymentPath, "1", error, COMPARE);
    auto res = Dic::Module::Summary::ExpertHotspotManager::QueryExpertHotspotData(COMPARE, "prefill", "1");
    const int exceptSize = 18560;
    EXPECT_EQ(res.size(), exceptSize);
    Clear();
}

TEST_F(ExpertHotspotManagerTest, ParseHeatMapFromProfilingWithoutTraceDb)
{
    Dic::Module::ParserFactory::Reset();
    InitParser(filePath, Dic::COMPARE);
    auto respotoryFactory = RepositoryFactory::Instance();
    auto dataEngine = DataEngine::Instance();
    dataEngine->SetRepositoryFactory(respotoryFactory);
    auto renderEngine = RenderEngine::Instance();
    renderEngine->SetDataEngineInterface(dataEngine);
    std::string errorMsg;
    bool res = Dic::Module::Summary::ExpertHotspotManager::UpdateHeatMapFromProfiling(errorMsg, Dic::COMPARE, {"0"});
    EXPECT_EQ(res, false);
    Clear();
    Dic::Module::ParserFactory::Reset();
}