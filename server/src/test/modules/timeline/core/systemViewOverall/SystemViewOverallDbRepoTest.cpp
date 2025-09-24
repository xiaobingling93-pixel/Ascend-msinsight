/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include "DataBaseManager.h"
#include "ParamsParser.h"
#include "FileUtil.h"
#include "FullDbParser.h"
#include "ParserStatusManager.h"
#include "TraceTime.h"
#include "ProjectParserFactory.h"
#include "WsSessionImpl.h"
#include "WsSessionManager.h"
#include "SystemViewOverallRepoFactory.h"
#include "TraceDatabaseSqlConst.h"
#include "DataEngine.h"
#include "RepositoryFactory.h"
#include "RenderEngine.h"

using namespace Dic::Module::Timeline;
using namespace Dic::Module::FullDb;
using namespace Dic::Module;
using namespace Dic;
namespace Dic::Module::Timeline {
class SystemViewOverallDbRepoTest : public ::testing::Test {
public:
    static void InitDataEngine()
    {
        auto respotoryFactory = RepositoryFactory::Instance();
        auto dataEngine = DataEngine::Instance();
        dataEngine->SetRepositoryFactory(respotoryFactory);
        auto renderEngine = RenderEngine::Instance();
        renderEngine->SetDataEngineInterface(dataEngine);
    }

    static void SetUpTestCase()
    {
        FullDb::FullDbParser::Instance().Reset();
        InitDataEngine();
        std::string currPath = Dic::FileUtil::GetCurrPath();
        const ParamsOption &option = ParamsParser::Instance().GetOption();
        ServerLog::Initialize(option.logPath, option.logSize, option.logLevel, to_string(option.wsPort));
        std::unique_ptr<Dic::Server::WsSession> session = std::make_unique<Dic::Server::WsSessionImpl>(nullptr);
        WsSessionManager::Instance().AddSession(std::move(session));
        std::string subStr = "server";
        size_t index = currPath.rfind(subStr);
        currPath = currPath.substr(0, index - 1);
        std::string dbPath3 = currPath +
            R"(/test/data/pytorch/db/level1/rank0_ascend_pt/ASCEND_PROFILER_OUTPUT/ascend_pytorch_profiler_0.db)";
        DataBaseManager::Instance().SetDataType(DataType::DB);
        DataBaseManager::Instance().SetFileType(FileType::PYTORCH);
        DataBaseManager::Instance().CreateTraceConnectionPool("0", dbPath3);
        std::pair<std::string, ParserType> parserType = std::make_pair(dbPath3, ParserType::DB);
        ParserType allocType = parserType.second;
        std::shared_ptr<ProjectParserBase> factory = ParserFactory::GetProjectParser(allocType);
        // 路径列表不为空，需要进行文件目录的新增、覆盖
        ProjectTypeEnum projectType = factory->GetProjectType({ dbPath3 });
        std::vector<Global::ProjectExplorerInfo> projectExplorerInfoList;

        std::string warn;
        // 获取文件列表
        std::vector<std::string> parseFileList = factory->GetParseFileByImportFile(dbPath3, warn);
        Global::ProjectExplorerInfo projectExplorerInfo;
        projectExplorerInfo.fileName = dbPath3;
        projectExplorerInfo.projectName = dbPath3;
        projectExplorerInfo.projectType = static_cast<int64_t>(projectType);
        projectExplorerInfo.importType = "import";
        auto parseFileInfo = std::make_shared<ParseFileInfo>();
        parseFileInfo->parseFilePath = dbPath3;
        projectExplorerInfo.subParseFileInfo.push_back(parseFileInfo);
        projectExplorerInfoList.push_back(projectExplorerInfo);

        if (allocType != ParserType::JSON) {
            ParserFactory::Reset();
        }
        ImportActionRequest request;
        factory->Parser(projectExplorerInfoList, request);
        Timeline::DataBaseManager::Instance().SetDbPathMapping("FullDb", dbPath3, "ubuntu3538958389648580163_0");
        Timeline::DataBaseManager::Instance().SetDbPathMapping("0", dbPath3, "ubuntu3538958389648580163_0");
        while (ParserStatusManager::Instance().GetParserStatus("ubuntu3538958389648580163_0 0") !=
               ParserStatus::FINISH_ALL) {
        }
        auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabaseByRankId("0");
        while (!database->CheckValueFromStatusInfoTable(OVERLAP_ANALYSIS_UNIT, FINISH_STATUS)) {}
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

TEST_F(SystemViewOverallDbRepoTest, QueryOverlapAnalysisDataForOverallMetricTest)
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
    const std::vector<double> EXPECT_OVERLAP_TINE = {98071.95, 8903.43, 25258.14};
    for (size_t index = 0; index < EXPECT_OVERLAP_TINE.size(); index++) {
        ASSERT_FALSE(overlapInfos[index].categoryList.empty());
        EXPECT_EQ(overlapInfos[index].categoryList[0], EXPECT_OVERLAP_NAME[index]);
        EXPECT_NEAR(overlapInfos[index].duration, EXPECT_OVERLAP_TINE[index], toleranceThreshold);
    }
}

// System View Overall: 查询Computing拆解所需数据（有PMU数据，能正常查询）
TEST_F(SystemViewOverallDbRepoTest, QueryDataForComputingOverallMetricTestWithPmu)
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
    Protocol::SystemViewOverallRes tmpRes = { .totalTime = 8903.43, .ratio = 6.73, // Computing Time = 8903.43, 6.73%
        .nums = 0, .avg = 0, .max = 0, .min = UINT32_MAX, .name = COMPUTING_TIME, .children = {}, .level = 1};
    details.emplace_back(tmpRes);
    tmpRes = { .totalTime = 98071.95, .ratio = 74.17, // Comm(Not Overlapped) = 98071.95, 74.17%
        .nums = 0, .avg = 0, .max = 0, .min = 0, .name = COMMUNICATION_NOT_OVERLAP_TIME, .children = {}, .level = 1};
    details.emplace_back(tmpRes);
    tmpRes = { .totalTime = 25258.14, .ratio = 19.1, // Free = 25258.14, 19.1%
        .nums = 0, .avg = 0, .max = 0, .min = 0, .name = FREE_TIME, .children = {}, .level = 1};
    details.emplace_back(tmpRes);
    tmpRes = { .totalTime = 132233.52, .ratio = 100, // E2E = 132233.52, 100%
        .nums = 0, .avg = 0, .max = 0, .min = 0, .name = E2E_TIME, .children = {}, .level = 1};
    details.emplace_back(tmpRes);
    requestParams.rankId = "0";
    SystemViewOverallHelper computeHelper;
    bool result = repoPtr->QueryDataForComputingOverallMetric(requestParams, computeHelper, database);
    EXPECT_EQ(result, true);
    EXPECT_EQ(computeHelper.cpuCubeOps.size(), 24);  // 24
    EXPECT_EQ(computeHelper.kernelEvents.size(), 185);  // 185
    EXPECT_EQ(computeHelper.bwdTrackId, 13471134862279280); // globalTid in PYTORCH_API = 13471134862279280
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

TEST_F(SystemViewOverallDbRepoTest, QueryCommunicationOverlapOverallInfosTestWhenSuccess)
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
    computeHelper.e2eTime = 151888.75; // 151888.75
    std::vector<Protocol::SystemViewOverallRes> responseBody;
    repoPtr->QueryCommunicationOverlapOverallInfos(requestParams, computeHelper, responseBody, database);
    EXPECT_EQ(responseBody.size(), 1);
    EXPECT_DOUBLE_EQ(responseBody[0].totalTime, 98071.947); // 98071.947
    EXPECT_DOUBLE_EQ(responseBody[0].ratio, 64.57); // 64.57
    EXPECT_EQ(responseBody[0].children.size(), 1); // Group 0 Communication
    EXPECT_DOUBLE_EQ(responseBody[0].children[0].totalTime, 98071.95); // 98071.95
    EXPECT_EQ(responseBody[0].children[0].children.size(), 2); // 2 for "Wait Time" and "Transmit Time"
    EXPECT_DOUBLE_EQ(responseBody[0].children[0].children[0].totalTime, 0); // 0 for Wait Time
    EXPECT_DOUBLE_EQ(responseBody[0].children[0].children[1].totalTime, 98071.95); // 98071.95 for Transmit Time
}

TEST_F(SystemViewOverallDbRepoTest, QueryCommunicationOpsTimeDataByGroupNameTestWhenSuccess)
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
    std::string deviceId = Dic::Module::Timeline::DataBaseManager::Instance().GetDeviceIdFromRankId("0");
    requestParams.deviceId = deviceId;
    int deviceIdInt = StringUtil::StringToInt(deviceId);
    ParamsForOAData paramsForOaData = { sql, type, minTimestamp };
    bool result = database->QueryOverlapAnalysisData(paramsForOaData, deviceIdInt, notOverlapData, totalTime);
    EXPECT_TRUE(result);
    requestParams.categoryList = {"", "Group 0 Communication"};
    repoPtr->QueryCommunicationOpsTimeDataByGroupName(requestParams, minTimestamp, notOverlapData,
                                                      response.body.sameOperatorsDetails, database);
    EXPECT_EQ(response.body.sameOperatorsDetails.size(), 4); // 4
    int index = 0;
    EXPECT_EQ(response.body.sameOperatorsDetails[index++].name, "hcom_allReduce__210_0_1");
    EXPECT_EQ(response.body.sameOperatorsDetails[index++].name, "hcom_allReduce__210_1_1");
    EXPECT_EQ(response.body.sameOperatorsDetails[index++].name, "hcom_allReduce__210_2_1");
    EXPECT_EQ(response.body.sameOperatorsDetails[index++].name, "hcom_allReduce__210_3_1");
}
}