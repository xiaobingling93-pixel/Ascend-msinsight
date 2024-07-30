/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include <OperatorProtocol.h>
#include <GlobalDefs.h>
#include "ProtocolDefs.h"
#include "OperatorProtocolUtil.h"
#include "OperatorProtocolResponse.h"

using namespace Dic::Protocol;
class OperatorProtocolUtilTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        operatorProtocol.Register();
    }

    void TearDown() override
    {
        operatorProtocol.UnRegister();
    }

    void CheckResponseBaseStruct(const std::optional<Dic::document_t> &jsonOptional)
    {
        EXPECT_EQ(jsonOptional.has_value(), true);
        EXPECT_EQ(jsonOptional.value().HasMember("body"), true);
        EXPECT_EQ(jsonOptional.value()["body"].HasMember("data"), true);
        EXPECT_EQ(jsonOptional.value()["body"]["data"].IsArray(), true);
    }

    void CheckOperatorDetailInfoResStruct(const Dic::json_t &item, const OperatorDetailInfoRes &res)
    {
        std::vector<std::string> key = {
            "name", "type", "accCore", "startTime", "duration", "waitTime", "blockDim",
            "inputShape", "inputType", "inputFormat", "outputShape", "outputType", "outputFormat"
        };
        for (const auto& it : key) {
            EXPECT_TRUE(item.HasMember(it.c_str()));
        }
        EXPECT_EQ(item["name"].GetString(), res.name);
        EXPECT_EQ(item["type"].GetString(), res.type);
        EXPECT_EQ(item["accCore"].GetString(), res.accCore);
        EXPECT_EQ(item["startTime"].GetString(), res.startTime);
        EXPECT_EQ(item["duration"].GetDouble(), res.duration);
        EXPECT_EQ(item["waitTime"].GetDouble(), res.waitTime);
        EXPECT_EQ(item["blockDim"].GetInt64(), res.blockDim);
        EXPECT_EQ(item["inputShape"].GetString(), res.inputShape);
        EXPECT_EQ(item["inputType"].GetString(), res.inputType);
        EXPECT_EQ(item["inputFormat"].GetString(), res.inputFormat);
        EXPECT_EQ(item["outputShape"].GetString(), res.outputShape);
        EXPECT_EQ(item["outputType"].GetString(), res.outputType);
        EXPECT_EQ(item["outputFormat"].GetString(), res.outputFormat);
        EXPECT_EQ(item.MemberCount(), key.size());
    }

    Dic::Protocol::OperatorProtocol operatorProtocol;
};

TEST_F(OperatorProtocolUtilTest, ToOperatorCategoryInfoResponseTest)
{
    Dic::Protocol::OperatorCategoryInfoResponse response;
    std::string err;
    // empty data
    response.datas = {};
    std::optional<Dic::document_t> jsonOptional = operatorProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
    // valid data
    response.datas = {
        {"FlashAttentionScore", 28179.42},
        {"DSAStatelessGenBitMask", 21827.46}
    };
    jsonOptional = operatorProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
    EXPECT_EQ(jsonOptional.value()["body"]["data"].Size(), response.datas.size());
    int i = 0;
    for (const auto &item : jsonOptional.value()["body"]["data"].GetArray()) {
        EXPECT_EQ(item.HasMember("name"), true);
        EXPECT_EQ(item["name"].GetString(), response.datas[i].name);
        EXPECT_EQ(item.HasMember("duration"), true);
        EXPECT_EQ(item["duration"].GetDouble(), response.datas[i++].duration);
        EXPECT_EQ(item.MemberCount(), 2); // 2, name and duration
    }
}

TEST_F(OperatorProtocolUtilTest, ToOperatorComputeUnitInfoResponseTest)
{
    Dic::Protocol::OperatorComputeUnitInfoResponse response;
    std::string err;
    // empty data
    response.datas = {};
    std::optional<Dic::document_t> jsonOptional = operatorProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
    // valid data
    response.datas = {
            {"AI_CORE", 28179.42},
            {"AI_CPU", 21827.46},
            {"VECTOR_CORE", 2000.46}
    };
    jsonOptional = operatorProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
    EXPECT_EQ(jsonOptional.value()["body"]["data"].Size(), response.datas.size());
    int i = 0;
    for (const auto &item : jsonOptional.value()["body"]["data"].GetArray()) {
        EXPECT_EQ(item.HasMember("name"), true);
        EXPECT_EQ(item["name"].GetString(), response.datas[i].name);
        EXPECT_EQ(item.HasMember("duration"), true);
        EXPECT_EQ(item["duration"].GetDouble(), response.datas[i++].duration);
        EXPECT_EQ(item.MemberCount(), 2); // 2, name and duration
    }
}

TEST_F(OperatorProtocolUtilTest, ToOperatorStatisticInfoResponseTest)
{
    Dic::Protocol::OperatorStatisticInfoResponse response;
    std::string err;
    // empty data
    response.total = 0;
    response.datas = {};
    std::optional<Dic::document_t> jsonOptional = operatorProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
    EXPECT_EQ(jsonOptional.value()["body"].HasMember("total"), true);
    EXPECT_EQ(jsonOptional.value()["body"]["total"], response.total);
    EXPECT_EQ(jsonOptional.value()["body"]["data"].Size(), response.datas.size());
    // valid data
    response.datas = {
            {"FlashAttentionScore", "FlashAttentionScore", "1;;1;2", "MIX_AIC", 28179.4, 4, 7044.8, 7111.4, 6982.4},
            {"MatMul", "MatMul", "16384,2560;5120,2560", "AI_CORE", 5992.7, 4, 1498.1, 1505.9, 1486.9}
    };
    response.total = response.datas.size();
    jsonOptional = operatorProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
    EXPECT_EQ(jsonOptional.value()["body"]["total"], response.total);
    EXPECT_EQ(jsonOptional.value()["body"]["data"].Size(), response.datas.size());
    int i = 0;
    for (const auto &item : jsonOptional.value()["body"]["data"].GetArray()) {
        EXPECT_EQ(item.HasMember("opType"), true);
        EXPECT_EQ(item["opType"].GetString(), response.datas[i].opType);
        EXPECT_EQ(item.HasMember("accCore"), true);
        EXPECT_EQ(item["accCore"].GetString(), response.datas[i].accCore);
        EXPECT_EQ(item.HasMember("totalTime"), true);
        EXPECT_EQ(item["totalTime"].GetDouble(), response.datas[i].totalTime);
        EXPECT_EQ(item.HasMember("maxTime"), true);
        EXPECT_EQ(item["maxTime"].GetDouble(), response.datas[i++].maxTime);
        EXPECT_EQ(item.MemberCount(), 9); // 9, name and duration
    }
}

TEST_F(OperatorProtocolUtilTest, ToOperatorMoreInfoResponseTest)
{
    Dic::Protocol::OperatorMoreInfoResponse response;
    std::string err;
    // empty data
    response.total = 0;
    response.level = "";
    response.datas = {};
    std::optional<Dic::document_t> jsonOptional = operatorProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
    EXPECT_EQ(jsonOptional.value()["body"].HasMember("total"), true);
    EXPECT_EQ(jsonOptional.value()["body"]["total"], response.total);
    EXPECT_EQ(jsonOptional.value()["body"].HasMember("level"), true);
    EXPECT_EQ(jsonOptional.value()["body"]["level"].GetString(), response.level);
    EXPECT_EQ(jsonOptional.value()["body"]["data"].Size(), response.datas.size());
    // valid data
    response.datas = {
        {
            "0", "1", "FlashAttentionScore", "FlashAttentionScore", "MIX_AIC", "131.2", 7111.4, 1.8, 24,
            "8192,2,640;8192,2,640", "DT_BF16;DT_BF16", "FORMAT_ND;FORMAT_ND",
            "655360;655360", "FLOAT;FLOAT", "FORMAT_ND;FORMAT_ND"
        },
        {
            "1", "2", "MatMul", "MatMul", "AI_CORE", "108.09", 1505.92, 0.05, 24,
            "16384,2560;5120,2560", "DT_BF16;DT_BF16","FORMAT_ND;FORMAT_ND","16384,5120","DT_BF16","FORMAT_ND"
        }
    };
    response.total = response.datas.size();
    response.level = "l2";
    jsonOptional = operatorProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
    EXPECT_EQ(jsonOptional.value()["body"]["total"], response.total);
    EXPECT_EQ(jsonOptional.value()["body"]["level"].GetString(), response.level);
    EXPECT_EQ(jsonOptional.value()["body"]["data"].Size(), response.datas.size());
    int i = 0;
    for (const auto &item : jsonOptional.value()["body"]["data"].GetArray()) {
        CheckOperatorDetailInfoResStruct(item, response.datas[i++]);
    }
}

TEST_F(OperatorProtocolUtilTest, ToOperatorDetailInfoResponseTest)
{
    Dic::Protocol::OperatorDetailInfoResponse response;
    std::string err;
    // empty data
    response.total = 0;
    response.level = "";
    response.datas = {};
    std::optional<Dic::document_t> jsonOptional = operatorProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
    EXPECT_EQ(jsonOptional.value()["body"].HasMember("total"), true);
    EXPECT_EQ(jsonOptional.value()["body"]["total"], response.total);
    EXPECT_EQ(jsonOptional.value()["body"].HasMember("level"), true);
    EXPECT_EQ(jsonOptional.value()["body"]["level"].GetString(), response.level);
    EXPECT_EQ(jsonOptional.value()["body"]["data"].Size(), response.datas.size());
    // valid data
    response.datas = {
        {"0", "1", "ApplyAdamW", "NA", "NA", "131.2", 7111.4, 1.8, 24, "", "", "", "", "", ""},
        {"1", "2", "MatMul", "NA", "NA", "108.09", 1505.92, 0.05, 24,     "", "", "", "", "", ""}
    };
    response.total = response.datas.size();
    response.level = "l0";
    jsonOptional = operatorProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
    EXPECT_EQ(jsonOptional.value()["body"]["total"], response.total);
    EXPECT_EQ(jsonOptional.value()["body"]["level"].GetString(), response.level);
    EXPECT_EQ(jsonOptional.value()["body"]["data"].Size(), response.datas.size());
    int i = 0;
    for (const auto &item : jsonOptional.value()["body"]["data"].GetArray()) {
        CheckOperatorDetailInfoResStruct(item, response.datas[i++]);
    }
}

TEST_F(OperatorProtocolUtilTest, ToOperatorParseStatusEventTest)
{
    Dic::Protocol::OperatorParseStatusEvent event;
    std::string err;
    event.data = {.rankId = "1", .status = true, .error = "error"};
    std::optional<Dic::document_t> jsonOptional = operatorProtocol.ToJson(event, err);
    EXPECT_EQ(jsonOptional.has_value(), true);
    EXPECT_EQ(jsonOptional.value().HasMember("body"), true);
    EXPECT_EQ(jsonOptional.value()["body"].HasMember("rankId"), true);
    EXPECT_EQ(jsonOptional.value()["body"]["rankId"].GetString(), event.data.rankId);
    EXPECT_EQ(jsonOptional.value()["body"].HasMember("status"), true);
    EXPECT_EQ(jsonOptional.value()["body"]["status"].GetBool(), event.data.status);
    EXPECT_EQ(jsonOptional.value()["body"].HasMember("error"), true);
    EXPECT_EQ(jsonOptional.value()["body"]["error"].GetString(), event.data.error);
}

TEST_F(OperatorProtocolUtilTest, ToOperatorParseClearEventTest)
{
    Dic::Protocol::OperatorParseClearEvent event;
    std::string err;
    std::optional<Dic::document_t> jsonOptional = operatorProtocol.ToJson(event, err);
    EXPECT_EQ(jsonOptional.has_value(), true);
}

