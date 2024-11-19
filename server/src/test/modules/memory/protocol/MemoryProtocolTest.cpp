/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "MemoryProtocolUtil.h"
#include "MemoryProtocolRequest.h"
#include "MemoryProtocol.h"

using namespace Dic::Protocol;
class MemoryProtocolTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        memoryProtocol.Register();
    }

    void TearDown() override
    {
        memoryProtocol.UnRegister();
    }
    Dic::Protocol::MemoryProtocol memoryProtocol;
};

TEST_F(MemoryProtocolTest, ToMemoryComponentRequestTestReturnNormal)
{
    std::string reqJson = R"({"id": 10, "moduleName": "memory", "type": "request", "resultCallbackId": 0,
        "command": "Memory/view/component", "params": {"rankId": "2", "currentPage": 1, "pageSize": 100,
        "orderBy": "component", "order": "ascend"}})";
    MemoryComponentParams expect = {"2", 1, 100, "component", "ascend", false};
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    auto result = dynamic_cast<MemoryComponentRequest &>(*(memoryProtocol.FromJson(json, err))).params;
    EXPECT_EQ(result.rankId, expect.rankId);
    EXPECT_EQ(result.currentPage, expect.currentPage);
    EXPECT_EQ(result.pageSize, expect.pageSize);
    EXPECT_EQ(result.orderBy, expect.orderBy);
    EXPECT_EQ(result.order, expect.order);
    EXPECT_EQ(result.isCompare, expect.isCompare);
}

TEST_F(MemoryProtocolTest, ToMemoryComponentRequestTestLackRankId)
{
    std::string reqJson = R"({"id": 10, "moduleName": "memory", "type": "request", "resultCallbackId": 0,
        "command": "Memory/view/component", "params": {"orderBy": "component", "order": "ascend"}})";
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    std::unique_ptr<Request> requestPtr = memoryProtocol.FromJson(json, err);
    EXPECT_EQ(requestPtr, nullptr);
    EXPECT_EQ(err, "Request json lacks member rankId.");
}

TEST_F(MemoryProtocolTest, ToMemoryOpeatorRequestTestReturnNormal)
{
    std::string reqJson = R"({"id": 10, "moduleName": "memory", "type": "request", "resultCallbackId": 0,
        "command": "Memory/view/operator", "params": {"rankId": "3", "type": "Stream", "searchName": "aten::add",
        "minSize": -100, "maxSize": 1000, "startTime": 100.314, "endTime": 256.397, "currentPage": 100, "pageSize": 10,
        "orderBy": "size", "order": "ascend"}})";
    MemoryOperatorParams expect = {"3", "Stream", "aten::add", -100, 1000, 100.314, 256.397, 100, 10,
        "size", "ascend", false};
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    auto result = dynamic_cast<MemoryOperatorRequest &>(*(memoryProtocol.FromJson(json, err))).params;
    EXPECT_EQ(result.rankId, expect.rankId);
    EXPECT_EQ(result.type, expect.type);
    EXPECT_EQ(result.searchName, expect.searchName);
    EXPECT_EQ(result.minSize, expect.minSize);
    EXPECT_EQ(result.maxSize, expect.maxSize);
    EXPECT_EQ(result.startTime, expect.startTime);
    EXPECT_EQ(result.endTime, expect.endTime);
    EXPECT_EQ(result.currentPage, expect.currentPage);
    EXPECT_EQ(result.pageSize, expect.pageSize);
    EXPECT_EQ(result.orderBy, expect.orderBy);
    EXPECT_EQ(result.order, expect.order);
    EXPECT_EQ(result.isCompare, expect.isCompare);
}

TEST_F(MemoryProtocolTest, ToMemoryResourceTypeRequestTestReturnNormal)
{
    std::string reqJson = R"({"id": 2, "moduleName": "memory", "type": "request", "command": "Memory/view/resourceType",
        "resultCallbackId": 0, "params": {"rankId": "0"}})";
    std::string expect = "0";
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    auto result = dynamic_cast<MemoryResourceTypeRequest &>(*(memoryProtocol.FromJson(json, err))).rankId;
    EXPECT_EQ(result, expect);
}

TEST_F(MemoryProtocolTest, ToMemoryStaticOperatorGraphRequestTestReturnNormal)
{
    std::string reqJson = R"({"id": 2, "moduleName": "memory", "type": "request",
        "command": "Memory/view/staticOpMemoryGraph", "resultCallbackId": 0,
        "params": {"rankId": "1", "modelName": "0", "graphId": "0", "isCompare": true}})";
    StaticOperatorGraphParams expect = {"1", "0", "0", true};
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    auto result = dynamic_cast<MemoryStaticOperatorGraphRequest &>(*(memoryProtocol.FromJson(json, err))).params;
    EXPECT_EQ(result.rankId, expect.rankId);
    EXPECT_EQ(result.modelName, expect.modelName);
    EXPECT_EQ(result.graphId, expect.graphId);
    EXPECT_EQ(result.isCompare, expect.isCompare);
}

TEST_F(MemoryProtocolTest, ToMemoryStaticOperatorListRequestTestReturnNormal)
{
    std::string reqJson = R"({"id": 10, "moduleName": "memory", "type": "request", "resultCallbackId": 0,
        "command": "Memory/view/staticOpMemoryList", "params": {"rankId": "3", "deviceId": "host", "modelName": "0",
        "graphId": "0", "searchName": "model.layers.0.attention_norm.weight", "minSize": -1000, "maxSize": 10000,
        "startNodeIndex": 1, "endNodeIndex": 2533, "currentPage": 1000, "pageSize": 20,
        "orderBy": "op_name", "order": "descend"}})";
    StaticOperatorListParams expect = {"3", "host", "0", "0", "model.layers.0.attention_norm.weight",
        -1000, 10000, 1, 2533, 1000, 20, "op_name", "descend", false};
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    auto result = dynamic_cast<MemoryStaticOperatorListRequest &>(*(memoryProtocol.FromJson(json, err))).params;
    EXPECT_EQ(result.rankId, expect.rankId);
    EXPECT_EQ(result.deviceId, expect.deviceId);
    EXPECT_EQ(result.modelName, expect.modelName);
    EXPECT_EQ(result.graphId, expect.graphId);
    EXPECT_EQ(result.searchName, expect.searchName);
    EXPECT_EQ(result.minSize, expect.minSize);
    EXPECT_EQ(result.maxSize, expect.maxSize);
    EXPECT_EQ(result.startNodeIndex, expect.startNodeIndex);
    EXPECT_EQ(result.endNodeIndex, expect.endNodeIndex);
    EXPECT_EQ(result.currentPage, expect.currentPage);
    EXPECT_EQ(result.pageSize, expect.pageSize);
    EXPECT_EQ(result.orderBy, expect.orderBy);
    EXPECT_EQ(result.order, expect.order);
    EXPECT_EQ(result.isCompare, expect.isCompare);
}

TEST_F(MemoryProtocolTest, ToMemoryTypeRequestTestReturnNormal)
{
    std::string reqJson = R"({"id": 2, "moduleName": "memory", "type": "request", "command": "Memory/view/type",
        "resultCallbackId": 0, "params": {"rankId": "15"}})";
    std::string expect = "15";
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    auto result = dynamic_cast<MemoryTypeRequest &>(*(memoryProtocol.FromJson(json, err))).rankId;
    EXPECT_EQ(result, expect);
}

TEST_F(MemoryProtocolTest, ToMemoryViewRequestTestReturnNormal)
{
    std::string reqJson = R"({"id": 2, "moduleName": "memory", "type": "request", "command": "Memory/view/memoryUsage",
        "resultCallbackId": 0, "params": {"rankId": "1", "type": "Overall", "isCompare": false}})";
    MemoryViewParams expect = {"1", "Overall", false};
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    auto result = dynamic_cast<MemoryViewRequest &>(*(memoryProtocol.FromJson(json, err))).params;
    EXPECT_EQ(result.rankId, expect.rankId);
    EXPECT_EQ(result.type, expect.type);
    EXPECT_EQ(result.isCompare, expect.isCompare);
}

TEST_F(MemoryProtocolTest, ToMemoryOperatorSizeRequestTestReturnNormal)
{
    std::string reqJson = R"({"id": 2, "moduleName": "memory", "type": "request", "resultCallbackId": 0,
        "command": "Memory/view/operator/size", "params": {"rankId": "1", "type": "Overall"}})";
    MemoryViewParams expect = {"1", "Overall", false};
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    auto result = dynamic_cast<MemoryOperatorSizeRequest &>(*(memoryProtocol.FromJson(json, err))).params;
    EXPECT_EQ(result.rankId, expect.rankId);
    EXPECT_EQ(result.type, expect.type);
    EXPECT_EQ(result.isCompare, expect.isCompare);
}