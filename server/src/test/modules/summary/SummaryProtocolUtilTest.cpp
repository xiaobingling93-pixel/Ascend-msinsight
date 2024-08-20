/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include <GlobalDefs.h>
#include "ProtocolDefs.h"
#include "SummaryProtocol.h"
#include "SummaryProtocolUtil.h"
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"

using namespace Dic::Protocol;
class SummaryProtocolUtilTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        protocol.Register();
    }

    void TearDown() override
    {
        protocol.UnRegister();
    }

    Dic::Protocol::SummaryProtocol protocol;
};

TEST_F(SummaryProtocolUtilTest, ToQueryParallelStrategyRequestTest)
{
    std::string reqJson = R"({"id": 2, "moduleName": "summary", "type": "request",
        "command": "summary/query/parallelStrategy", "resultCallbackId": 0, "params": {}})";
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    auto result = dynamic_cast<QueryParallelStrategyRequest &>(*(protocol.FromJson(json, err)));
    EXPECT_EQ(result.command, REQ_RES_SUMMARY_QUERY_PARALLEL_STRATEGY);
}


TEST_F(SummaryProtocolUtilTest, ToSetParallelStrategyRequestTest)
{
    std::string reqJson = R"({"id": 2, "moduleName": "summary", "type": "request",
        "command": "summary/set/parallelStrategy", "resultCallbackId": 0, "params": {}})";
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    auto request = protocol.FromJson(json, err);
    EXPECT_TRUE(request == nullptr);
    reqJson = R"({"id": 2, "moduleName": "summary", "type": "request",
        "command": "summary/set/parallelStrategy", "resultCallbackId": 0, "params":
        {"algorithm": "test", "tpSize": 2, "ppSize": 3, "dpSize": 4}})";
    json.Parse(reqJson.c_str());
    auto result = dynamic_cast<SetParallelStrategyRequest &>(*(protocol.FromJson(json, err)));
    EXPECT_EQ(result.command, REQ_RES_SUMMARY_SET_PARALLEL_STRATEGY);
    EXPECT_EQ(result.config.algorithm, "test");
    EXPECT_EQ(result.config.tpSize, 2); // tp = 2
    EXPECT_EQ(result.config.ppSize, 3); // pp = 3
    EXPECT_EQ(result.config.dpSize, 4); // dp = 4
}

TEST_F(SummaryProtocolUtilTest, ToQueryParallelStrategyResponseTest)
{
    Dic::Protocol::QueryParallelStrategyResponse response;
    std::string err;
    response.config.algorithm = "megatron-lm";
    response.config.tpSize = 8; // tp = 8
    response.config.ppSize = 4; // pp = 4
    response.config.dpSize = 2; // dp = 2
    std::optional<Dic::document_t> jsonOptional = protocol.ToJson(response, err);
    EXPECT_EQ(jsonOptional.value()["body"][KEY_ALGORITHM.c_str()], response.config.algorithm.c_str());
    EXPECT_EQ(jsonOptional.value()["body"][KEY_TP_SIZE.c_str()], response.config.tpSize);
    EXPECT_EQ(jsonOptional.value()["body"][KEY_PP_SIZE.c_str()], response.config.ppSize);
    EXPECT_EQ(jsonOptional.value()["body"][KEY_DP_SIZE.c_str()], response.config.dpSize);
}

TEST_F(SummaryProtocolUtilTest, ToSetParallelStrategyResponseTest)
{
    Dic::Protocol::SetParallelStrategyResponse response;
    std::string err;
    response.result = false;
    response.msg = "test";
    std::optional<Dic::document_t> jsonOptional = protocol.ToJson(response, err);
    EXPECT_EQ(jsonOptional.value()["body"][KEY_RESULT.c_str()], response.result);
    EXPECT_EQ(jsonOptional.value()["body"][KEY_MSG.c_str()], response.msg.c_str());
}