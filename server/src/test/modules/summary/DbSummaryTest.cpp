/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "SummaryProtocolResponse.h"
#include "DataBaseManager.h"
#include "DbSummaryDataBase.h"
#include "ParamsParser.h"
#include "FileUtil.h"
#include "TraceTime.h"

using namespace Dic::Module::Timeline;
using namespace Dic::Module::FullDb;

class DbSummaryTest : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        std::string currPath = Dic::FileUtil::GetCurrPath();
        const ParamsOption &option = ParamsParser::Instance().GetOption();
        ServerLog::Initialize(option.logPath, option.logSize, option.logLevel, to_string(option.wsPort));
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        std::string dbPath3 = R"(/src/test/test_data/full_db/)";
        DataBaseManager::Instance().SetDataType(DataType::DB);
        DataBaseManager::Instance().SetFileType(FileType::MS_PROF);
        DataBaseManager::Instance().CreatConnectionPool("0", currPath + dbPath3 + "msprof_0.db");
        auto database = std::dynamic_pointer_cast<DbTraceDataBase, VirtualTraceDatabase>(
            DataBaseManager::Instance().GetTraceDatabase("0"));
        database->UpdateStartTime("0");
        auto summaryDatabase =
                dynamic_cast<DbSummaryDataBase *>(DataBaseManager::Instance().GetSummaryDatabase("0"));
        summaryDatabase->OpenDb(currPath + dbPath3 + "msprof_0.db", false);
    }
    static void TearDownTestSuite() {}
};

TEST_F(DbSummaryTest, QueryComputeStatisticsData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::SummaryStatisticParams requestParams;
    requestParams.rankId = "2";
    Dic::Protocol::SummaryStatisticsBody responseBody;
    database->QueryComputeStatisticsData(requestParams, responseBody);
    int expectSize = 1;
    EXPECT_EQ(responseBody.summaryStatisticsItemList.size(), expectSize);
}

TEST_F(DbSummaryTest, QueryComputeStatisticsDataWithEmptyParamReturnExpectSize)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::SummaryStatisticParams requestParams;
    requestParams.stepId = "";
    Dic::Protocol::SummaryStatisticsBody responseBody;
    auto res = database->QueryComputeStatisticsData(requestParams, responseBody);
    EXPECT_EQ(res, true);
    const int expectSize = 1;
    EXPECT_EQ(responseBody.summaryStatisticsItemList.size(), expectSize);
}

TEST_F(DbSummaryTest, QueryComputeStatisticsData2)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::SummaryStatisticParams requestParams;
    requestParams.rankId = "2";
    requestParams.stepId = "16";
    Dic::Protocol::SummaryStatisticsBody responseBody;
    database->QueryComputeStatisticsData(requestParams, responseBody);
    int expectSize = 1;
    EXPECT_EQ(responseBody.summaryStatisticsItemList.size(), expectSize);
}

TEST_F(DbSummaryTest, QueryCommunicationDetailData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabase("0");
    Dic::Protocol::CommunicationDetailParams requestParams;
    requestParams.rankId = "2";
    requestParams.currentPage = 0;
    requestParams.pageSize = 10; // page size = 10
    requestParams.timeFlag = "AI_VECTOR_CORE";
    Dic::Protocol::CommunicationDetailResponse responseBody;
    database->QueryCommunicationOpDetail(requestParams, responseBody.commDetails);
    int expectSize = 10;
    EXPECT_EQ(responseBody.commDetails.size(), expectSize);
}

TEST_F(DbSummaryTest, QueryGetTotalNumData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabase("0");
    Dic::Protocol::CommunicationDetailParams requestParams;
    requestParams.rankId = "2";
    requestParams.currentPage = 0;
    requestParams.pageSize = 10; // page size = 10
    requestParams.timeFlag = "AI_VECTOR_CORE";
    Dic::Protocol::CommunicationDetailResponse responseBody;
    database->QueryTotalNumByAcceleratorCore(requestParams.timeFlag, responseBody.totalNum);
    int expectSize = 11;
    EXPECT_EQ(responseBody.totalNum, expectSize);
}

TEST_F(DbSummaryTest, QueryComputeDetailData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabase("0");
    Dic::Protocol::ComputeDetailParams requestParams;
    requestParams.rankId = "2";
    requestParams.currentPage = 0;
    requestParams.pageSize = 10; // page size = 10
    requestParams.timeFlag = "AI_VECTOR_CORE";
    std::vector<Dic::Protocol::ComputeDetail> responseBody;
    database->QueryComputeOpDetail(requestParams, responseBody);
    int expectSize = 10;
    EXPECT_EQ(responseBody.size(), expectSize);
}