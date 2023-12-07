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
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabase("0");
    Dic::Protocol::OperatorDurationReqParams params = {"0", GROUP_OPERATOR_TYPE, 15};
    std::vector<Protocol::OperatorDurationRes> datas = {};
    bool result = db->QueryOperatorDurationInfo(params, Protocol::QueryType::CATEGORY, datas);
    EXPECT_EQ(result, true);
    int size = 7;
    EXPECT_EQ(datas.size(), size);
    datas.clear();
    result = db->QueryOperatorDurationInfo(params, Protocol::QueryType::COMPUTE_UNIT, datas);
    EXPECT_EQ(result, true);
    int unitSize = 5;
    EXPECT_EQ(datas.size(), unitSize);
}

TEST_F(TestSuit, QueryOperatorDurationInfoByOpTypeAndInputShape)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabase("0");
    Dic::Protocol::OperatorDurationReqParams params = {"0", GROUP_INPUT_SHAPE, 15};
    std::vector<Protocol::OperatorDurationRes> datas = {};
    bool result = db->QueryOperatorDurationInfo(params, Protocol::QueryType::CATEGORY, datas);
    EXPECT_EQ(result, true);
    int size = 9;
    EXPECT_EQ(datas.size(), size);
    datas.clear();
    result = db->QueryOperatorDurationInfo(params, Protocol::QueryType::COMPUTE_UNIT, datas);
    EXPECT_EQ(result, true);
    int unitSize = 6;
    EXPECT_EQ(datas.size(), unitSize);
}

TEST_F(TestSuit, QueryOperatorDurationInfoByOperator)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabase("0");
    Dic::Protocol::OperatorDurationReqParams params = {"0", GROUP_OPERATOR, 15};
    std::vector<Protocol::OperatorDurationRes> datas = {};
    bool result = db->QueryOperatorDurationInfo(params, Protocol::QueryType::CATEGORY, datas);
    EXPECT_EQ(result, true);
    int size = 9;
    EXPECT_EQ(datas.size(), size);
    datas.clear();
    result = db->QueryOperatorDurationInfo(params, Protocol::QueryType::COMPUTE_UNIT, datas);
    EXPECT_EQ(result, true);
    int cnt = 6;
    EXPECT_EQ(datas.size(), cnt);
}

TEST_F(TestSuit, QueryOperatorStatisticInfoByOpType)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabase("0");
    Protocol::OperatorStatisticReqParams reqParams = {"0", GROUP_OPERATOR_TYPE, 15, 0, 10, "", ""};
    Dic::Protocol::OperatorStatisticInfoResponse response = {};
    bool result = db->QueryOperatorStatisticInfo(reqParams, response);
    EXPECT_EQ(result, true);
    int total = 7;
    EXPECT_EQ(response.total, total);
    EXPECT_EQ(response.datas.size(), total);
}

TEST_F(TestSuit, QueryOperatorStatisticInfoByOpTypeAndInputShape)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabase("0");
    Protocol::OperatorStatisticReqParams reqParams = {"0", GROUP_INPUT_SHAPE, 15, 0, 5, "", ""};
    Dic::Protocol::OperatorStatisticInfoResponse response = {};
    bool result = db->QueryOperatorStatisticInfo(reqParams, response);
    EXPECT_EQ(result, true);
    int total = 9;
    EXPECT_EQ(response.total, total);
    int size = 5;
    EXPECT_EQ(response.datas.size(), size);
}

TEST_F(TestSuit, QueryOperatorDetailInfoByOperator)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabase("0");
    Protocol::OperatorStatisticReqParams reqParams = {"0", GROUP_OPERATOR, 15, 0, 10, "", ""};
    Protocol::OperatorDetailInfoResponse response = {};
    bool result = db->QueryOperatorDetailInfo(reqParams, response);
    EXPECT_EQ(result, true);
    int total = 15;
    EXPECT_EQ(response.total, total);
    EXPECT_EQ(response.level, "l1");
    int size = 10;
    EXPECT_EQ(response.datas.size(), size);
}

TEST_F(TestSuit, QueryOperatorMoreInfoByOpType)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabase("0");
    Protocol::OperatorMoreInfoReqParams reqParams = {"0", GROUP_OPERATOR_TYPE, 15, "Cast", "", "", 0, 10, "", ""};
    Protocol::OperatorMoreInfoResponse response = {};
    bool result = db->QueryOperatorMoreInfo(reqParams, response);
    EXPECT_EQ(result, true);
    int64_t total = 3;
    EXPECT_EQ(response.total, total);
    EXPECT_EQ(response.level, "l1");
    EXPECT_EQ(response.datas.size(), total);
}

TEST_F(TestSuit, QueryOperatorMoreInfoByInputShape)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetSummaryDatabase("0");
    Protocol::OperatorMoreInfoReqParams reqParams = {
        "0", GROUP_INPUT_SHAPE, 15, "", "NonZero", R"("""16""")", 0, 10, "", ""
    };
    Protocol::OperatorMoreInfoResponse response = {};
    bool result = db->QueryOperatorMoreInfo(reqParams, response);
    EXPECT_EQ(result, true);
    int total = 3;
    EXPECT_EQ(response.total, total);
    EXPECT_EQ(response.level, "l1");
    EXPECT_EQ(response.datas.size(), total);
}