/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include <gtest/gtest.h>
#include "ConstantDefs.h"
#include "ClusterFileParser.h"
#include "DataBaseManager.h"
#include "ExpertHotspotManager.h"
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
        Dic::Module::FullDb::DataBaseManager::Instance().EraseClusterDb(Dic::COMPARE);
    }
};

TEST_F(ExpertHotspotManagerTest, InitExpertHotspotDataSuccess)
{
    Clear();
    InitParser(filePath, Dic::COMPARE);
    std::string error;
    Dic::Module::Summary::ExpertHotspotManager::InitExpertHotspotData(hotspotPath, "1", error);
    auto res = Dic::Module::Summary::ExpertHotspotManager::QueryExpertHotsSpotData("decode", "1");
    const int exceptSize = 232;
    EXPECT_EQ(res.size(), exceptSize);
    Clear();
}