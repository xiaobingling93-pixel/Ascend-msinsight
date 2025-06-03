/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include "FileUtil.h"
#include "ParamsParser.h"
#include "TraceFileParser.h"
#include "DataBaseManager.h"
#include "KernelParse.h"
#include "ParserStatusManager.h"
#include "TraceTime.h"
#include "SystemViewOverallRepoFactory.h"

using namespace Dic::Module::Summary;
using namespace Dic::Server;
using namespace Dic;
namespace Dic::Module::Timeline {
class SystemViewOverallTextRepoTest : public ::testing::Test  {
public:
    static void SetUpTestSuite()
    {
        // Repo Factory
        SystemViewOverallRepoFactory::Instance();
        // database
        std::string currPath = Dic::FileUtil::GetCurrPath();
        const ParamsOption &option = ParamsParser::Instance().GetOption();
        ServerLog::Initialize(option.logPath, option.logSize, option.logLevel, to_string(option.wsPort));
        std::string subStr = "server";
        size_t index = currPath.rfind(subStr);
        currPath = currPath.substr(0, index - 1);
        std::string refPath0 = R"(/test/data/pytorch/text/level1/rank0_ascend_pt/ASCEND_PROFILER_OUTPUT/)";
        std::string refPathWithoutPMU = R"(/test/data/pytorch/text/level0/rank1_ascend_pt/ASCEND_PROFILER_OUTPUT/)";
        DataBaseManager::Instance().SetDataType(DataType::TEXT);
        DataBaseManager::Instance().CreatConnectionPool("0", currPath + refPath0 + "mindstudio_insight_data.db");
        DataBaseManager::Instance().CreatConnectionPool("1", currPath + refPathWithoutPMU +
            "mindstudio_insight_data.db");
        TraceFileParser::Instance().Parse({currPath + refPath0 + "trace_view.json"}, "0", "");
        WaitParseEnd({"0"});
        TraceFileParser::Instance().Parse({currPath + refPath0 + "trace_view.json"}, "1", "");
        WaitParseEnd({"1"});
        std::string testDataPath0 = currPath + R"(/test/data/pytorch/text/level1/rank0_ascend_pt)";
        KernelParse::Instance().Parse({testDataPath0});
        WaitParseEnd({KERNEL_PREFIX + "0"});
        std::string testDataPathWithoutPmu = currPath + R"(/test/data/pytorch/text/level0/rank1_ascend_pt)";
        KernelParse::Instance().Parse({testDataPathWithoutPmu});
        WaitParseEnd({KERNEL_PREFIX + "1"});
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

    static void TearDownTestSuite()
    {
        SystemViewOverallRepoFactory::Instance()->Reset();
        KernelParse::Instance().Reset();
        TraceFileParser::Instance().DeleteParseFiles({"0"});
        TraceFileParser::Instance().DeleteParseFiles({"1"});
    }

    static int Main(int argc, char** argv)
    {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }
};

TEST_F(SystemViewOverallTextRepoTest, QueryOverlapAnalysisDataForOverallMetricTest)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabaseByRankId("0");
    auto repoPtr = SystemViewOverallRepoFactory::Instance()->GetSystemViewOverallRepo(
        DataBaseManager::Instance().GetDataType());
    if (repoPtr == nullptr) {
        // GetSystemViewOverallRepo中已有日志报错
        return;
    }
    Dic::Protocol::SystemViewOverallReqParam requestParams;
    requestParams.rankId = "0";
    std::vector<OverallTmpInfo> overlapInfos;
    overlapInfos = repoPtr->QueryOverlapAnalysisDataForOverallMetric(requestParams, database);
    ASSERT_EQ(overlapInfos.size(), 3);  // 3
    const double toleranceThreshold = 0.01;
    const std::vector<std::string> EXPECT_OVERLAP_NAME = {"Communication(Not Overlapped)", "Computing", "Free"};
    const std::vector<double> EXPECT_OVERLAP_TINE = {101963.2, 8873.76, 25744.05};
    for (size_t index = 0; index < EXPECT_OVERLAP_TINE.size(); index++) {
        ASSERT_FALSE(overlapInfos[index].categoryList.empty());
        EXPECT_EQ(overlapInfos[index].categoryList[0], EXPECT_OVERLAP_NAME[index]);
        EXPECT_NEAR(overlapInfos[index].duration, EXPECT_OVERLAP_TINE[index], toleranceThreshold);
    }
}

// System View Overall: 查询Computing拆解所需数据（有PMU数据，能正常查询）
TEST_F(SystemViewOverallTextRepoTest, QueryDataForComputingOverallMetricTestWithPmu)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabaseByRankId("0");
    auto repoPtr = SystemViewOverallRepoFactory::Instance()->GetSystemViewOverallRepo(
        DataBaseManager::Instance().GetDataType());
    if (repoPtr == nullptr) {
        // GetSystemViewOverallRepo中已有日志报错
        return;
    }
    Dic::Protocol::SystemViewOverallReqParam requestParams;
    std::vector<SystemViewOverallRes> details;
    Protocol::SystemViewOverallRes tmpRes = { .totalTime = 8873.76, .ratio = 6.5, // Computing Time = 8873.76, 6.5%
        .nums = 0, .avg = 0, .max = 0, .min = UINT32_MAX, .name = COMPUTING_TIME, .children = {}, .level = 1};
    details.emplace_back(tmpRes);
    tmpRes = { .totalTime = 101963.2, .ratio = 74.65, // Comm(Not Overlapped) = 101963.2, 74.65%
        .nums = 0, .avg = 0, .max = 0, .min = 0, .name = COMMUNICATION_NOT_OVERLAP_TIME, .children = {}, .level = 1};
    details.emplace_back(tmpRes);
    tmpRes = { .totalTime = 25744.05, .ratio = 18.85, // Free = 25744.05, 18.85%
        .nums = 0, .avg = 0, .max = 0, .min = 0, .name = FREE_TIME, .children = {}, .level = 1};
    details.emplace_back(tmpRes);
    tmpRes = { .totalTime = 136581.01, .ratio = 100, // E2E = 136581.01, 100%
        .nums = 0, .avg = 0, .max = 0, .min = 0, .name = E2E_TIME, .children = {}, .level = 1};
    details.emplace_back(tmpRes);
    requestParams.rankId = "0";
    SystemViewOverallHelper computeHelper;
    bool result = repoPtr->QueryDataForComputingOverallMetric(requestParams, computeHelper, database);
    EXPECT_EQ(result, true);
    EXPECT_EQ(computeHelper.cpuCubeOps.size(), 24);  // 24
    EXPECT_EQ(computeHelper.kernelEvents.size(), 185);  // 185
    EXPECT_EQ(computeHelper.bwdTrackId, 2); // 2
    computeHelper.CategorizeComputingEvents();
    computeHelper.AggregateComputingOverallMetrics(details);
    EXPECT_EQ(details.size(), 4); // 4
    EXPECT_EQ(details[0].children.size(), 5); // 5
    // more details
    std::vector<std::string> tempList = {"Conv"};
    uint64_t minTimestamp = TraceTime::Instance().GetStartTime();
    std::vector<SameOperatorsDetails> filteredEvents =
        computeHelper.FilterComputingEventsByCategory(tempList, minTimestamp, "");
    EXPECT_EQ(filteredEvents.size(), 66); // 66个Conv类算子
}

//  System View Overall: 查询Computing拆解所需数据（无PMU数据，无法进行Computing拆解）
TEST_F(SystemViewOverallTextRepoTest, QueryDataForComputingOverallMetricTestWithoutPmu)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabaseByRankId("1");
    auto repoPtr = SystemViewOverallRepoFactory::Instance()->GetSystemViewOverallRepo(
        DataBaseManager::Instance().GetDataType());
    if (repoPtr == nullptr) {
        // GetSystemViewOverallRepo中已有日志报错
        return;
    }
    Dic::Protocol::SystemViewOverallReqParam requestParams;
    requestParams.rankId = "1";
    SystemViewOverallHelper computeHelper;
    bool result = repoPtr->QueryDataForComputingOverallMetric(requestParams, computeHelper, database);
    EXPECT_EQ(result, true);
    EXPECT_EQ(computeHelper.cpuCubeOps.size(), 0);
    EXPECT_EQ(computeHelper.kernelEvents.size(), 0);
    EXPECT_EQ(computeHelper.bwdTrackId, 0);
}

TEST_F(SystemViewOverallTextRepoTest, QueryCommunicationOverlapOverallInfosTestWhenSuccess)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabaseByRankId("0");
    auto repoPtr = SystemViewOverallRepoFactory::Instance()->GetSystemViewOverallRepo(
        DataBaseManager::Instance().GetDataType());
    if (repoPtr == nullptr) {
        // GetSystemViewOverallRepo中已有日志报错
        return;
    }
    Dic::Protocol::SystemViewOverallReqParam requestParams;
    requestParams.rankId = "0";
    const double e2eTime = 136581.01;
    SystemViewOverallHelper computeHelper;
    std::vector<Protocol::SystemViewOverallRes> responseBody;
    repoPtr->QueryCommunicationOverlapOverallInfos(requestParams, e2eTime, responseBody, database);
    EXPECT_EQ(responseBody.size(), 1);
    EXPECT_DOUBLE_EQ(responseBody[0].totalTime, 101963.203); // 101963.203
    EXPECT_DOUBLE_EQ(responseBody[0].ratio, 74.65); // 74.65
    EXPECT_EQ(responseBody[0].children.size(), 1); // Group 0 Communication
    EXPECT_DOUBLE_EQ(responseBody[0].children[0].totalTime, 101963.2); // 101963.2
    EXPECT_EQ(responseBody[0].children[0].children.size(), 2); // 2 for "Wait Time" and "Transmit Time"
    EXPECT_DOUBLE_EQ(responseBody[0].children[0].children[0].totalTime, 0); // 0 for Wait Time
    EXPECT_DOUBLE_EQ(responseBody[0].children[0].children[1].totalTime, 101963.2); // 101963.2 for Transmit Time
}

TEST_F(SystemViewOverallTextRepoTest, QueryCommunicationOpsTimeDataByGroupNameTestWhenSuccess)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabaseByRankId("0");
    auto repoPtr = SystemViewOverallRepoFactory::Instance()->GetSystemViewOverallRepo(
        DataBaseManager::Instance().GetDataType());
    if (repoPtr == nullptr) {
        // GetSystemViewOverallRepo中已有日志报错
        return;
    }
    Dic::Protocol::SystemViewOverallReqParam requestParams;
    requestParams.rankId = "0";
    SystemViewOverallHelper computeHelper;
    UnitThreadsOperatorsResponse response;
    uint64_t minTimestamp = TraceTime::Instance().GetStartTime();
    std::string sql = DataBaseManager::Instance().GetDataType() == DataType::TEXT ?
                      QUERY_OVERLAP_ANALYSIS_BY_TYPE_TEXT_SQL : QUERY_OVERLAP_ANALYSIS_BY_TYPE_DB_SQL;
    std::string type = DataBaseManager::Instance().GetDataType() == DataType::TEXT ?
                       "Communication(Not Overlapped)" : "2";
    uint64_t totalTime = 0;
    std::vector<Protocol::ThreadTraces> notOverlapData{};
    bool result = database->QueryOverlapAnalysisData(sql, type, minTimestamp, notOverlapData, totalTime);
    EXPECT_TRUE(result);
    repoPtr->QueryCommunicationOpsTimeDataByGroupName("Group 0 Communication", minTimestamp, notOverlapData,
                                                      response.body.sameOperatorsDetails, database);
    EXPECT_EQ(response.body.sameOperatorsDetails.size(), 4); // 4
    int index = 0;
    EXPECT_EQ(response.body.sameOperatorsDetails[index++].name, "hcom_allReduce__586_0_1");
    EXPECT_EQ(response.body.sameOperatorsDetails[index++].name, "hcom_allReduce__586_1_1");
    EXPECT_EQ(response.body.sameOperatorsDetails[index++].name, "hcom_allReduce__586_2_1");
    EXPECT_EQ(response.body.sameOperatorsDetails[index++].name, "hcom_allReduce__586_3_1");
}
}