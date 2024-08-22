/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include <gtest/gtest.h>
#include "vector"
#include "../../TestSuit.cpp"
#include "DataBaseManager.h"
#include "OperatorProtocolRequest.h"
#include "OperatorProtocolResponse.h"

class OperatorTestSuit : public TestSuit {
};

const std::string GROUP_OPERATOR = "Operator";
const std::string GROUP_OPERATOR_TYPE = "Operator Type";
const std::string GROUP_INPUT_SHAPE = "Input Shape";

TEST_F(TestSuit, QueryOperatorDurationInfoByOpType)
{
    DataBaseManager::Instance().SetDataType(DataType::TEXT);
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabase("0");
    Dic::Protocol::OperatorDurationReqParams params = {"0", GROUP_OPERATOR_TYPE, 15};
    std::vector<Dic::Protocol::OperatorDurationRes> datas = {};
    bool result = db->QueryOperatorDurationInfo(params, Dic::Protocol::QueryType::CATEGORY, datas);
    EXPECT_EQ(result, true);
    int size = 8;
    EXPECT_EQ(datas.size(), size);
    datas.clear();
    result = db->QueryOperatorDurationInfo(params, Dic::Protocol::QueryType::COMPUTE_UNIT, datas);
    EXPECT_EQ(result, true);
    int unitSize = 6;
    EXPECT_EQ(datas.size(), unitSize);
}

TEST_F(TestSuit, QueryOperatorDurationInfoByOpTypeAndInputShape)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabase("0");
    Dic::Protocol::OperatorDurationReqParams params = {"0", GROUP_INPUT_SHAPE, 15};
    std::vector<Dic::Protocol::OperatorDurationRes> datas = {};
    bool result = db->QueryOperatorDurationInfo(params, Dic::Protocol::QueryType::CATEGORY, datas);
    EXPECT_EQ(result, true);
    int size = 10;
    EXPECT_EQ(datas.size(), size);
    datas.clear();
    result = db->QueryOperatorDurationInfo(params, Dic::Protocol::QueryType::COMPUTE_UNIT, datas);
    EXPECT_EQ(result, true);
    int unitSize = 6;
    EXPECT_EQ(datas.size(), unitSize);
}

TEST_F(TestSuit, QueryOperatorDurationInfoByOperator)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabase("0");
    Dic::Protocol::OperatorDurationReqParams params = {"0", GROUP_OPERATOR, 15};
    std::vector<Dic::Protocol::OperatorDurationRes> datas = {};
    bool result = db->QueryOperatorDurationInfo(params, Dic::Protocol::QueryType::CATEGORY, datas);
    EXPECT_EQ(result, true);
    int size = 15;
    EXPECT_EQ(datas.size(), size);
    datas.clear();
    result = db->QueryOperatorDurationInfo(params, Dic::Protocol::QueryType::COMPUTE_UNIT, datas);
    EXPECT_EQ(result, true);
    int cnt = 5;
    EXPECT_EQ(datas.size(), cnt);
}

TEST_F(TestSuit, QueryOperatorStatisticInfoByOpType)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabase("0");
    Dic::Protocol::OperatorStatisticReqParams reqParams = {false, "0", GROUP_OPERATOR_TYPE, 15, 1, 10, "", ""};
    Dic::Protocol::OperatorStatisticInfoResponse response = {};
    bool result = db->QueryOperatorStatisticInfo(reqParams, response);
    EXPECT_EQ(result, true);
    int total = 8;
    EXPECT_EQ(response.total, total);
    EXPECT_EQ(response.datas.size(), total);
}

TEST_F(TestSuit, QueryAllOperatorStatisticInfoByOpType)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabase("0");
    Dic::Protocol::OperatorStatisticReqParams reqParams = {true, "0", GROUP_OPERATOR_TYPE, 15, 1, 10, "", ""};
    Dic::Protocol::OperatorStatisticInfoResponse response = {};
    std::vector<Protocol::OperatorStatisticInfoRes> compareRes;
    bool result = db->QueryAllOperatorStatisticInfo(reqParams, compareRes);
    EXPECT_EQ(result, true);
    int total = 8;
    EXPECT_EQ(compareRes.size(), total);
}

TEST_F(TestSuit, QueryAllOperatorStatisticInfoByOpTypeAndInputShape)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabase("0");
    Dic::Protocol::OperatorStatisticReqParams reqParams = {false, "0", GROUP_INPUT_SHAPE, 15, 1, 5, "", ""};
    Dic::Protocol::OperatorStatisticInfoResponse response = {};
    bool result = db->QueryOperatorStatisticInfo(reqParams, response);
    EXPECT_EQ(result, true);
    int total = 10;
    EXPECT_EQ(response.total, total);
    int size = 5;
    EXPECT_EQ(response.datas.size(), size);
}

TEST_F(TestSuit, QueryOperatorDetailInfoByOperator)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabase("0");
    Dic::Protocol::OperatorStatisticReqParams reqParams = {false, "0", GROUP_OPERATOR, 15, 1, 10, "", ""};
    Dic::Protocol::OperatorDetailInfoResponse response = {};
    bool result = db->QueryOperatorDetailInfo(reqParams, response);
    EXPECT_EQ(result, true);
    int total = 15;
    EXPECT_EQ(response.total, total);
    EXPECT_EQ(response.level, "l1");
    int size = 10;
    EXPECT_EQ(response.datas.size(), size);
}

TEST_F(TestSuit, QueryAllOperatorDetailInfoByOperator)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabase("0");
    Dic::Protocol::OperatorStatisticReqParams reqParams = {false, "0", GROUP_OPERATOR, 15, 1, 10, "", ""};
    Dic::Protocol::OperatorDetailInfoResponse response = {};
    std::vector<Protocol::OperatorDetailInfoRes> baselineRes;
    bool result = db->QueryAllOperatorDetailInfo(reqParams, baselineRes, response.level);
    EXPECT_EQ(result, true);
    int total = 17;
    EXPECT_EQ(response.level, "l1");
    EXPECT_EQ(baselineRes.size(), total);
}

TEST_F(TestSuit, QueryOperatorMoreInfoByOpType)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabase("0");
    Dic::Protocol::OperatorMoreInfoReqParams reqParams = {
        "0", GROUP_OPERATOR_TYPE, 15, "Cast", "", "", "AI_CORE", 1, 10, "", ""};
    Dic::Protocol::OperatorMoreInfoResponse response = {};
    bool result = db->QueryOperatorMoreInfo(reqParams, response);
    EXPECT_EQ(result, true);
    int64_t total = 1;
    EXPECT_EQ(response.total, total);
    EXPECT_EQ(response.level, "l1");
    EXPECT_EQ(response.datas.size(), total);
}

TEST_F(TestSuit, QueryOperatorMoreInfoByInputShape)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabase("0");
    Dic::Protocol::OperatorMoreInfoReqParams reqParams = {
        "0", GROUP_INPUT_SHAPE, 15, "", "NonZero", R"("""16""")", "MIX_AIV", 1, 10, "", ""
    };
    Dic::Protocol::OperatorMoreInfoResponse response = {};
    bool result = db->QueryOperatorMoreInfo(reqParams, response);
    EXPECT_EQ(result, true);
    int total = 0;
    EXPECT_EQ(response.total, total);
    EXPECT_EQ(response.level, "l1");
    EXPECT_EQ(response.datas.size(), total);
}