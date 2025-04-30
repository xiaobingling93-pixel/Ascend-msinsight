/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include <GlobalDefs.h>
#include "ProtocolDefs.h"
#include "CommunicationProtocol.h"
#include "CommunicationProtocolResponse.h"

using namespace Dic::Protocol;
const uint64_t NUMBER_HUNDRED = 100;
const uint64_t NUMBER_THOUSAND = 100;
class CommunicationProtocolUtilTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        protocol.Register();
    }

    void TearDown() override
    {
        protocol.UnRegister();
    }

    void CheckResponseBaseStruct(const std::optional<Dic::document_t> &jsonOptional)
    {
        ASSERT_TRUE(jsonOptional.has_value());
        ASSERT_TRUE(jsonOptional.value().HasMember("body"));
    }

    Dic::Protocol::CommunicationProtocol protocol;
};

TEST_F(CommunicationProtocolUtilTest, ToCommunicationAdvisorRequestNormalTest)
{
    std::string reqJson = R"({"id": 1, "moduleName": "communication", "type": "request", "resultCallbackId": 0,
        "command": "communication/advisor", "params": {}})";
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    auto result = protocol.FromJson(json, err);
    EXPECT_EQ(result->id, 1);
    EXPECT_EQ(result->command, "communication/advisor");
    EXPECT_EQ(result->type, ProtocolMessage::Type::REQUEST);
    EXPECT_EQ(result->moduleName, MODULE_COMMUNICATION);
}

TEST_F(CommunicationProtocolUtilTest, ToAdvisorRequestLackIdTestReturnNull)
{
    std::string reqJson = R"({"moduleName": "communication", "type": "request", "resultCallbackId": 0,
        "command": "communication/advisor", "params": {}})";
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    auto result = protocol.FromJson(json, err);
    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(err, "Failed to set request base info of communication advisor request.");
}

TEST_F(CommunicationProtocolUtilTest, ToOperatorListResponseTest)
{
    const std::string KEY_MIN_TIME = "minTime";
    const std::string KEY_MAX_TIME = "maxTime";
    Dic::Protocol::OperatorListsResponse response;
    std::string err;
    OperatorListsResponseBody body;
    response.body.rankLists.emplace_back("1");
    response.body.minTime = NUMBER_HUNDRED;
    response.body.maxTime = NUMBER_THOUSAND;
    OperatorTimeItem op = {"op1", 100, 900};
    CompareData<std::vector<OperatorTimeItem>> compareData;
    compareData.compare.push_back(op);
    response.body.opLists.push_back(compareData);
    std::optional<Dic::document_t> jsonOptional = protocol.ToJson(response, err);
    EXPECT_EQ(jsonOptional.value()["body"][KEY_MIN_TIME.c_str()], response.body.minTime);
    EXPECT_EQ(jsonOptional.value()["body"][KEY_MAX_TIME.c_str()], response.body.maxTime);
}

TEST_F(CommunicationProtocolUtilTest, ToCommunicationAdvisorResponseEmptyDataTest)
{
    CommunicationAdvisorResponse response;
    std::string err;
    response.body.items.clear();
    std::optional<Dic::document_t> jsonOptional = protocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
}

TEST_F(CommunicationProtocolUtilTest, ToCommunicationAdvisorResponseNormalDataTest)
{
    CommunicationAdvisorResponse response;
    CommunicationAdvisorInfo info{"Packet Analysis",
        {
        {"Category", {"SDMA", "RDMA"}},
        {"Min Size", {"16MB", "1MB"}},
        {"Packet Size <= Min Size Percentage", {"2.1", "0.5"}}
        },
        {"Suggestion A", "Suggestion B", "Suggestion C"}};
    response.body.items.push_back(info);
    std::string err;
    std::optional<Dic::document_t> jsonOptional = protocol.ToJson(response, err);
    const int two = 2;
    const int three = 3;
    CheckResponseBaseStruct(jsonOptional);
    ASSERT_TRUE(jsonOptional.value()["body"].HasMember("items"));
    ASSERT_TRUE(jsonOptional.value()["body"]["items"].IsArray());
    ASSERT_EQ(jsonOptional.value()["body"]["items"].Size(), 1);
    ASSERT_TRUE(jsonOptional.value()["body"]["items"][0].HasMember("name"));
    EXPECT_EQ(jsonOptional.value()["body"]["items"][0]["name"], "Packet Analysis");
    ASSERT_TRUE(jsonOptional.value()["body"]["items"][0].HasMember("statistics"));
    ASSERT_TRUE(jsonOptional.value()["body"]["items"][0]["statistics"].IsObject());
    ASSERT_TRUE(jsonOptional.value()["body"]["items"][0]["statistics"].HasMember("Category"));
    ASSERT_TRUE(jsonOptional.value()["body"]["items"][0]["statistics"]["Category"].IsArray());
    ASSERT_EQ(jsonOptional.value()["body"]["items"][0]["statistics"]["Category"].Size(), two);
    EXPECT_EQ(jsonOptional.value()["body"]["items"][0]["statistics"]["Category"][0], "SDMA");
    EXPECT_EQ(jsonOptional.value()["body"]["items"][0]["statistics"]["Category"][1], "RDMA");
    ASSERT_TRUE(jsonOptional.value()["body"]["items"][0]["statistics"].HasMember("Min Size"));
    ASSERT_TRUE(jsonOptional.value()["body"]["items"][0]["statistics"]["Min Size"].IsArray());
    ASSERT_EQ(jsonOptional.value()["body"]["items"][0]["statistics"]["Min Size"].Size(), two);
    EXPECT_EQ(jsonOptional.value()["body"]["items"][0]["statistics"]["Min Size"][0], "16MB");
    EXPECT_EQ(jsonOptional.value()["body"]["items"][0]["statistics"]["Min Size"][1], "1MB");
    ASSERT_TRUE(jsonOptional.value()["body"]["items"][0]["statistics"].HasMember("Packet Size <= Min Size Percentage"));
    ASSERT_TRUE(jsonOptional.value()["body"]["items"][0]["statistics"]["Packet Size <= Min Size Percentage"].IsArray());
    ASSERT_EQ(jsonOptional.value()["body"]["items"][0]["statistics"]["Packet Size <= Min Size Percentage"].Size(), two);
    EXPECT_EQ(jsonOptional.value()["body"]["items"][0]["statistics"]["Packet Size <= Min Size Percentage"][0], "2.1");
    EXPECT_EQ(jsonOptional.value()["body"]["items"][0]["statistics"]["Packet Size <= Min Size Percentage"][1], "0.5");
    ASSERT_TRUE(jsonOptional.value()["body"]["items"][0].HasMember("suggestions"));
    ASSERT_TRUE(jsonOptional.value()["body"]["items"][0]["suggestions"].IsArray());
    ASSERT_EQ(jsonOptional.value()["body"]["items"][0]["suggestions"].Size(), three);
    EXPECT_EQ(jsonOptional.value()["body"]["items"][0]["suggestions"][0], "Suggestion A");
    EXPECT_EQ(jsonOptional.value()["body"]["items"][0]["suggestions"][1], "Suggestion B");
    EXPECT_EQ(jsonOptional.value()["body"]["items"][0]["suggestions"][two], "Suggestion C");
}