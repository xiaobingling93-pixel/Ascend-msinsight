/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "SummaryService.h"
#include "ConstantDefs.h"
#include "ClusterFileParser.h"
#include "DataBaseManager.h"
#include "TimeUtil.h"

using namespace Dic::Module::Summary;
const int NUMBER_ZERO = 0;
const int NUMBER_SIXTEEN = 16;

class SummaryServiceTest : public ::testing::Test {
protected:
    std::string filePath;

    void SetUp() override
    {
        std::string  currPath = Dic::FileUtil::GetCurrPath();
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        filePath = currPath + R"(/src/test/test_data/cluster_analysis_output)";
    }

    static void InitParser(const std::string &dataPath, const std::string &uniqueKey)
    {
        // 创建新的db连接对象
        auto database = Dic::Module::FullDb::DataBaseManager::Instance().CreateClusterDatabase(uniqueKey,
            Dic::Module::Timeline::DataType::TEXT);
        // 集群解析，如果集群已解析，则只会初始化db，然后结束流程
        Dic::Module::FullDb::ClusterFileParser clusterFileParser(dataPath, database,
                                                                 uniqueKey + Dic::TimeUtil::Instance().NowStr());
        clusterFileParser.ParseClusterFiles();
        clusterFileParser.ParseClusterStep2Files();
    }

    static void Clear()
    {
        Dic::Module::FullDb::DataBaseManager::Instance().EraseClusterDb(Dic::COMPARE);
        Dic::Module::FullDb::DataBaseManager::Instance().EraseClusterDb(Dic::BASELINE);
    }
};

TEST_F(SummaryServiceTest, QueryCompareSummaryBaseInfoAllFail)
{
    Clear();
    SummaryTopRankRequest request;
    request.params.isCompare = false;
    SummaryTopRankResponse response;
    SummaryService::QueryCompareSummaryBaseInfo(request, response);
    EXPECT_EQ(response.body.baseInfo.compare.filePath, "");
    EXPECT_EQ(response.body.baseInfo.baseline.filePath, "");
}

TEST_F(SummaryServiceTest, QueryCompareSummaryBaseInfoAllSuccess)
{
    Clear();
    InitParser(filePath, Dic::COMPARE);
    InitParser(filePath, Dic::BASELINE);
    SummaryTopRankRequest request;
    request.params.isCompare = true;
    SummaryTopRankResponse response;
    SummaryService::QueryCompareSummaryBaseInfo(request, response);
    EXPECT_EQ(response.body.baseInfo.compare.rankCount, NUMBER_SIXTEEN);
    EXPECT_EQ(response.body.baseInfo.baseline.rankCount, NUMBER_SIXTEEN);
    Clear();
}

TEST_F(SummaryServiceTest, QueryCompareSummaryBaseInfoOnlyCompareSuccess)
{
    Clear();
    InitParser(filePath, Dic::COMPARE);
    SummaryTopRankRequest request;
    request.params.isCompare = true;
    SummaryTopRankResponse response;
    SummaryService::QueryCompareSummaryBaseInfo(request, response);
    EXPECT_EQ(response.body.baseInfo.compare.rankCount, NUMBER_SIXTEEN);
    EXPECT_EQ(response.body.baseInfo.baseline.rankCount, NUMBER_ZERO);
    Clear();
}

TEST_F(SummaryServiceTest, QueryCompareSummaryBaseInfoOnlyBaselineSuccess)
{
    Clear();
    InitParser(filePath, Dic::BASELINE);
    SummaryTopRankRequest request;
    request.params.isCompare = true;
    SummaryTopRankResponse response;
    SummaryService::QueryCompareSummaryBaseInfo(request, response);
    EXPECT_EQ(response.body.baseInfo.compare.rankCount, NUMBER_ZERO);
    EXPECT_EQ(response.body.baseInfo.baseline.rankCount, NUMBER_SIXTEEN);
    Clear();
}