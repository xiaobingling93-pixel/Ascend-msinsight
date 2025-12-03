/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "vector"
#include "DataBaseManager.h"
#include "OperatorProtocolRequest.h"
#include "OperatorProtocolResponse.h"
#include "DbSummaryDataBase.h"
#include "ParamsParser.h"
#include "FileUtil.h"
#include "../../FullDbTestSuit.cpp"

using namespace Dic::Module::Timeline;
using namespace Dic::Module::FullDb;

class DbOperatorTestSuit : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        std::string currPath = Dic::FileUtil::GetCurrPath();
        const ParamsOption &option = ParamsParser::Instance().GetOption();
        ServerLog::Initialize(option.logPath, option.logSize, option.logLevel, to_string(option.wsPort));
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        std::string dbPath3 = R"(/src/test/test_data/full_db/)";
        std::string dbPath = StringUtil::StrJoin(currPath, dbPath3, "msprof_0.db");
        DataBaseManager::Instance().SetDataType(DataType::DB, dbPath);
        auto summeryDatabase =
            std::dynamic_pointer_cast<DbSummaryDataBase, Dic::Module::Summary::VirtualSummaryDataBase>(
                DataBaseManager::Instance().CreateSummaryDatabase("2", dbPath));
        summeryDatabase->OpenDb(dbPath, false);
    }

    static void TearDownTestSuite() {}
};

const std::string GROUP_OPERATOR = "Operator";
const std::string GROUP_OPERATOR_TYPE = "Operator Type";
const std::string GROUP_INPUT_SHAPE = "Input Shape";

TEST_F(DbOperatorTestSuit, FullDb_of_QueryOperatorDurationInfoByOpType)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabaseByRankId("2");
    Dic::Protocol::OperatorDurationReqParams params = {"2", "2", GROUP_OPERATOR_TYPE, 15};
    std::vector<Dic::Protocol::OperatorDurationRes> datas = {};
    bool result = db->QueryOperatorDurationInfo(params, Dic::Protocol::QueryType::CATEGORY, datas);
    EXPECT_EQ(result, true);
    int size = 8;
    EXPECT_EQ(datas.size(), size);
    datas.clear();
    result = db->QueryOperatorDurationInfo(params, Dic::Protocol::QueryType::COMPUTE_UNIT, datas);
    EXPECT_EQ(result, true);
    int unitSize = 1;
    EXPECT_EQ(datas.size(), unitSize);
}

TEST_F(DbOperatorTestSuit, FullDb_of_QueryOperatorDurationInfoByOpTypeAndInputShape)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabaseByRankId("2");
    Dic::Protocol::OperatorDurationReqParams params = {"2", "2", GROUP_INPUT_SHAPE, 15};
    std::vector<Dic::Protocol::OperatorDurationRes> datas = {};
    bool result = db->QueryOperatorDurationInfo(params, Dic::Protocol::QueryType::CATEGORY, datas);
    EXPECT_EQ(result, true);
    int size = 11;
    EXPECT_EQ(datas.size(), size);
    datas.clear();
    result = db->QueryOperatorDurationInfo(params, Dic::Protocol::QueryType::COMPUTE_UNIT, datas);
    EXPECT_EQ(result, true);
    int unitSize = 1;
    EXPECT_EQ(datas.size(), unitSize);
}

TEST_F(DbOperatorTestSuit, FullDb_of_QueryOperatorDurationInfoByOperator)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabaseByRankId("2");
    Dic::Protocol::OperatorDurationReqParams params = {"2", "2", GROUP_OPERATOR, 15};
    std::vector<Dic::Protocol::OperatorDurationRes> datas = {};
    bool result = db->QueryOperatorDurationInfo(params, Dic::Protocol::QueryType::CATEGORY, datas);
    EXPECT_EQ(result, true);
    int size = 11;
    EXPECT_EQ(datas.size(), size);
    datas.clear();
    result = db->QueryOperatorDurationInfo(params, Dic::Protocol::QueryType::COMPUTE_UNIT, datas);
    EXPECT_EQ(result, true);
    int cnt = 1;
    EXPECT_EQ(datas.size(), cnt);
}

TEST_F(DbOperatorTestSuit, FullDb_of_QueryOperatorStatisticInfoByOpType)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabaseByRankId("2");
    Dic::Protocol::OperatorStatisticReqParams reqParams = {false, "2", "2", GROUP_OPERATOR_TYPE, 15, 0, 10, "", ""};
    Dic::Protocol::OperatorStatisticInfoResponse response = {};
    bool result = db->QueryOperatorStatisticInfo(reqParams, response);
    EXPECT_EQ(result, true);
    int total = 8;
    int size = 8;
    EXPECT_EQ(response.total, total);
    EXPECT_EQ(response.datas.size(), size);
}

TEST_F(DbOperatorTestSuit, FullDb_of_QueryOperatorStatisticInfoByOpTypeAndInputShape)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabaseByRankId("2");
    Dic::Protocol::OperatorStatisticReqParams reqParams = {false, "2", "2", GROUP_INPUT_SHAPE, 15, 0, 5, "", ""};
    Dic::Protocol::OperatorStatisticInfoResponse response = {};
    bool result = db->QueryOperatorStatisticInfo(reqParams, response);
    EXPECT_EQ(result, true);
    int total = 11;
    EXPECT_EQ(response.total, total);
    int size = 5;
    EXPECT_EQ(response.datas.size(), size);
}

TEST_F(DbOperatorTestSuit, FullDb_of_QueryAllOperatorStatisticInfoByOpTypeAndInputShape)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabaseByRankId("2");
    Dic::Protocol::OperatorStatisticReqParams reqParams = {true, "2", "2", GROUP_INPUT_SHAPE, 15, 0, 5, "", ""};
    Dic::Protocol::OperatorStatisticInfoResponse response = {};
    std::vector<Protocol::OperatorStatisticInfoRes> compareRes;
    bool result = db->QueryAllOperatorStatisticInfo(reqParams, compareRes);
    EXPECT_EQ(result, true);
    int size = 11;
    EXPECT_EQ(compareRes.size(), size);
}

TEST_F(DbOperatorTestSuit, FullDb_of_QueryAllOperatorDetailInfoWhenPmuDataNotExist)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabaseByRankId("2");
    Dic::Protocol::OperatorStatisticReqParams reqParams = {false, "2", "2", GROUP_OPERATOR, 15, 0, 5, "", ""};
    Dic::Protocol::OperatorDetailInfoResponse response = {};
    bool result = db->QueryOperatorDetailInfo(reqParams, response);
    EXPECT_EQ(result, true);
}


TEST_F(DbOperatorTestSuit, QueryBandwidthContentionMatMulDataTest)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabaseByRankId("2");
    std::vector<Dic::Module::BandwidthContentionMatMulInfo> info;
    bool result = db->QueryBandwidthContentionMatMulData(info);
    ASSERT_TRUE(result);
    ASSERT_EQ(info.size(), 0);
}