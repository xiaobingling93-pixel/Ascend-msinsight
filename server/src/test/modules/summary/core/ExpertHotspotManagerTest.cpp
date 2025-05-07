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

class ExpertHotspotManagerTest : public ::testing::Test {
protected:
    std::string filePath;
    std::string hotspotPath;
    void SetUp() override
    {
        std::string  currPath = Dic::FileUtil::GetCurrPath();
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        filePath = currPath + R"(/src/test/test_data/cluster_analysis_output)";
        hotspotPath = currPath + R"(/src/test/test_data/expert_hotspot)";
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

    static void Clear()
    {
        auto db = Dic::Module::FullDb::DataBaseManager::Instance().GetClusterDatabase(Dic::COMPARE);
        if (db != nullptr) {
            // 清空热点数据
            db->DeleteExpertHotspot("decode", "1");
            db->DeleteExpertHotspot("prefill", "1");
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
    }
};

TEST_F(ExpertHotspotManagerTest, InitExpertHotspotDataSuccess)
{
    InitParser(filePath, Dic::COMPARE);
    std::string error;
    Dic::Module::Summary::ExpertHotspotManager::InitExpertHotspotData(hotspotPath, "1", error);
    auto res = Dic::Module::Summary::ExpertHotspotManager::QueryExpertHotspotData("decode", "1");
    const int exceptSize = 232;
    EXPECT_EQ(res.size(), exceptSize);
    Clear();
}

TEST_F(ExpertHotspotManagerTest, UpdateModelInfoSuccess)
{
    InitParser(filePath, Dic::COMPARE);
    std::string error;
    Dic::Module::Summary::ExpertHotspotManager::InitExpertHotspotData(hotspotPath, "1", error);
    Dic::Module::ModelInfo modelInfo{{0, 2}, 0, 0, 4, 61};
    Dic::Module::Summary::ExpertHotspotManager::UpdateModelInfo(modelInfo, error);
    auto res = Dic::Module::Summary::ExpertHotspotManager::QueryExpertHotspotData("decode", "1");
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
    Dic::Module::Summary::ExpertHotspotManager::UpdateModelInfo(modelInfo, error);
    auto res = Dic::Module::Summary::ExpertHotspotManager::QueryExpertHotspotData("decode", "1");
    const int exceptSize = 244;
    const int numberZero = 0;
    EXPECT_EQ(res.size(), exceptSize);
    EXPECT_EQ(res[1].visits, numberZero);
    Clear();
}