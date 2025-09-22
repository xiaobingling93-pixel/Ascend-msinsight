/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "MemoryProtocolUtil.h"
#include "MemoryProtocolRequest.h"
#include "JsonUtil.h"
#include "MemoryProtocol.h"

using namespace Dic;
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

    void CheckResponseBaseStruct(const std::optional<document_t> &jsonOptional)
    {
        ASSERT_TRUE(jsonOptional.has_value());
        ASSERT_TRUE(jsonOptional.value().HasMember("body"));
    }

    void CheckComponentEqual(const json_t &data, const MemoryComponent &expectData)
    {
        if (expectData.component.empty()) {
            EXPECT_EQ(data["component"].GetString(), "Unknown");
        } else {
            EXPECT_EQ(data["component"].GetString(), expectData.component);
        }
        EXPECT_EQ(data["timestamp"].GetString(), expectData.timestamp);
        EXPECT_EQ(data["totalReserved"].GetDouble(), expectData.totalReserved);
        EXPECT_EQ(data["device"].GetString(), expectData.device);
    }
    void CheckComponentResponseStruct(const std::optional<document_t> &jsonOptional,
        const MemoryComponentComparisonResponse &response)
    {
        ASSERT_TRUE(jsonOptional.value()["body"].HasMember("columnAttr"));
        ASSERT_TRUE(jsonOptional.value()["body"]["columnAttr"].IsArray());
        ASSERT_EQ(jsonOptional.value()["body"]["columnAttr"].Size(), response.columnAttr.size());
        for (size_t i = 0; i < response.columnAttr.size(); ++i) {
            EXPECT_EQ(jsonOptional.value()["body"]["columnAttr"][i]["name"].GetString(), response.columnAttr[i].name);
            EXPECT_EQ(jsonOptional.value()["body"]["columnAttr"][i]["type"].GetString(), response.columnAttr[i].type);
            EXPECT_EQ(jsonOptional.value()["body"]["columnAttr"][i]["key"].GetString(), response.columnAttr[i].key);
        }
        ASSERT_TRUE(jsonOptional.value()["body"].HasMember("componentDetail"));
        ASSERT_TRUE(jsonOptional.value()["body"]["componentDetail"].IsArray());
        ASSERT_EQ(jsonOptional.value()["body"]["componentDetail"].Size(), response.componentDiffDetails.size());
        for (size_t i = 0; i < response.componentDiffDetails.size(); ++i) {
            CheckComponentEqual(jsonOptional.value()["body"]["componentDetail"][i]["compare"],
                response.componentDiffDetails[i].compare);
            CheckComponentEqual(jsonOptional.value()["body"]["componentDetail"][i]["baseline"],
                response.componentDiffDetails[i].baseline);
            CheckComponentEqual(jsonOptional.value()["body"]["componentDetail"][i]["diff"],
                response.componentDiffDetails[i].diff);
        }
        ASSERT_TRUE(jsonOptional.value()["body"].HasMember("totalNum"));
        EXPECT_EQ(jsonOptional.value()["body"]["totalNum"].GetInt64(), response.totalNum);
    }

    void CheckOperatorEqual(const json_t &data, const MemoryOperator &expectData)
    {
        if (expectData.name.empty()) {
            EXPECT_EQ(data["name"].GetString(), "Unknown");
        } else {
            EXPECT_EQ(data["name"].GetString(), expectData.name);
        }
        EXPECT_EQ(data["size"].GetDouble(), expectData.size);
        EXPECT_EQ(data["allocationTime"].GetString(), expectData.allocationTime);
        EXPECT_EQ(data["releaseTime"].GetString(), expectData.releaseTime);
        EXPECT_EQ(data["duration"].GetDouble(), expectData.duration);
        EXPECT_EQ(data["activeReleaseTime"].GetString(), expectData.activeReleaseTime);
        EXPECT_EQ(data["activeDuration"].GetDouble(), expectData.activeDuration);
        EXPECT_EQ(data["allocationAllocated"].GetDouble(), expectData.allocationAllocated);
        EXPECT_EQ(data["allocationReserved"].GetDouble(), expectData.allocationReserved);
        EXPECT_EQ(data["allocationActive"].GetDouble(), expectData.allocationActive);
        EXPECT_EQ(data["releaseAllocated"].GetDouble(), expectData.releaseAllocated);
        EXPECT_EQ(data["releaseReserved"].GetDouble(), expectData.releaseReserved);
        EXPECT_EQ(data["releaseActive"].GetDouble(), expectData.releaseActive);
        EXPECT_EQ(data["streamId"].GetString(), expectData.streamId);
    }
    void CheckOperatorResponseStruct(const std::optional<document_t> &jsonOptional,
        const MemoryOperatorComparisonResponse &response)
    {
        ASSERT_TRUE(jsonOptional.value()["body"].HasMember("columnAttr"));
        ASSERT_TRUE(jsonOptional.value()["body"]["columnAttr"].IsArray());
        ASSERT_EQ(jsonOptional.value()["body"]["columnAttr"].Size(), response.columnAttr.size());
        for (size_t i = 0; i < response.columnAttr.size(); ++i) {
            EXPECT_EQ(jsonOptional.value()["body"]["columnAttr"][i]["name"].GetString(), response.columnAttr[i].name);
            EXPECT_EQ(jsonOptional.value()["body"]["columnAttr"][i]["type"].GetString(), response.columnAttr[i].type);
            EXPECT_EQ(jsonOptional.value()["body"]["columnAttr"][i]["key"].GetString(), response.columnAttr[i].key);
        }
        ASSERT_TRUE(jsonOptional.value()["body"].HasMember("operatorDetail"));
        ASSERT_TRUE(jsonOptional.value()["body"]["operatorDetail"].IsArray());
        ASSERT_EQ(jsonOptional.value()["body"]["operatorDetail"].Size(), response.operatorDiffDetails.size());
        for (size_t i = 0; i < response.operatorDiffDetails.size(); ++i) {
            CheckOperatorEqual(jsonOptional.value()["body"]["operatorDetail"][i]["compare"],
                response.operatorDiffDetails[i].compare);
            CheckOperatorEqual(jsonOptional.value()["body"]["operatorDetail"][i]["baseline"],
                response.operatorDiffDetails[i].baseline);
            CheckOperatorEqual(jsonOptional.value()["body"]["operatorDetail"][i]["diff"],
                response.operatorDiffDetails[i].diff);
        }
        ASSERT_TRUE(jsonOptional.value()["body"].HasMember("totalNum"));
        EXPECT_EQ(jsonOptional.value()["body"]["totalNum"].GetInt64(), response.totalNum);
    }

    void CheckStaticOperatorGraphResponseStruct(const std::optional<document_t> &jsonOptional,
        const MemoryStaticOperatorGraphResponse &response)
    {
        ASSERT_TRUE(jsonOptional.value()["body"].HasMember("legends"));
        ASSERT_TRUE(jsonOptional.value()["body"].HasMember("lines"));
        ASSERT_TRUE(jsonOptional.value()["body"]["legends"].IsArray());
        ASSERT_EQ(jsonOptional.value()["body"]["legends"].Size(), response.data.legends.size());
        for (size_t i = 0; i < response.data.legends.size(); ++i) {
            EXPECT_EQ(jsonOptional.value()["body"]["legends"][i].GetString(), response.data.legends[i]);
        }
        ASSERT_TRUE(jsonOptional.value()["body"]["lines"].IsArray());
        ASSERT_EQ(jsonOptional.value()["body"]["lines"].Size(), response.data.lines.size());
        for (size_t i = 0; i < response.data.lines.size(); ++i) {
            ASSERT_TRUE(jsonOptional.value()["body"]["lines"][i].IsArray());
            ASSERT_EQ(jsonOptional.value()["body"]["lines"][i].Size(), response.data.lines[i].size());
            for (size_t j = 0; j < response.data.lines[i].size(); ++j) {
                EXPECT_EQ(jsonOptional.value()["body"]["lines"][i][j].GetString(), response.data.lines[i][j]);
            }
        }
    }

    void CheckStaticOperatorEqual(const json_t &data, const StaticOperatorItem &expectData)
    {
        EXPECT_EQ(data["deviceId"].GetString(), expectData.deviceId);
        EXPECT_EQ(data["opName"].GetString(), expectData.opName);
        EXPECT_EQ(data["nodeIndexStart"].GetInt64(), expectData.nodeIndexStart);
        EXPECT_EQ(data["nodeIndexEnd"].GetInt64(), expectData.nodeIndexEnd);
        EXPECT_EQ(data["size"].GetDouble(), expectData.size);
    }
    void CheckStaticOperatorListResponseStruct(const std::optional<document_t> &jsonOptional,
        const MemoryStaticOperatorListCompResponse &response)
    {
        ASSERT_TRUE(jsonOptional.value()["body"].HasMember("columnAttr"));
        ASSERT_TRUE(jsonOptional.value()["body"]["columnAttr"].IsArray());
        ASSERT_EQ(jsonOptional.value()["body"]["columnAttr"].Size(), response.columnAttr.size());
        for (size_t i = 0; i < response.columnAttr.size(); ++i) {
            EXPECT_EQ(jsonOptional.value()["body"]["columnAttr"][i]["name"].GetString(), response.columnAttr[i].name);
            EXPECT_EQ(jsonOptional.value()["body"]["columnAttr"][i]["type"].GetString(), response.columnAttr[i].type);
            EXPECT_EQ(jsonOptional.value()["body"]["columnAttr"][i]["key"].GetString(), response.columnAttr[i].key);
        }
        ASSERT_TRUE(jsonOptional.value()["body"].HasMember("operatorDetail"));
        ASSERT_TRUE(jsonOptional.value()["body"]["operatorDetail"].IsArray());
        ASSERT_EQ(jsonOptional.value()["body"]["operatorDetail"].Size(), response.operatorDiffDetails.size());
        for (size_t i = 0; i < response.operatorDiffDetails.size(); ++i) {
            CheckStaticOperatorEqual(jsonOptional.value()["body"]["operatorDetail"][i]["compare"],
                response.operatorDiffDetails[i].compare);
            CheckStaticOperatorEqual(jsonOptional.value()["body"]["operatorDetail"][i]["baseline"],
                response.operatorDiffDetails[i].baseline);
            CheckStaticOperatorEqual(jsonOptional.value()["body"]["operatorDetail"][i]["diff"],
                response.operatorDiffDetails[i].diff);
        }
        ASSERT_TRUE(jsonOptional.value()["body"].HasMember("totalNum"));
        EXPECT_EQ(jsonOptional.value()["body"]["totalNum"].GetInt64(), response.totalNum);
    }

    void CheckViewResponseStruct(const std::optional<document_t> &jsonOptional, const MemoryViewResponse &response)
    {
        ASSERT_TRUE(jsonOptional.value()["body"].HasMember("title"));
        ASSERT_TRUE(jsonOptional.value()["body"].HasMember("legends"));
        ASSERT_TRUE(jsonOptional.value()["body"].HasMember("lines"));
        EXPECT_EQ(jsonOptional.value()["body"]["title"].GetString(), response.data.title);
        ASSERT_TRUE(jsonOptional.value()["body"]["legends"].IsArray());
        ASSERT_EQ(jsonOptional.value()["body"]["legends"].Size(), response.data.legends.size());
        for (size_t i = 0; i < response.data.legends.size(); ++i) {
            EXPECT_EQ(jsonOptional.value()["body"]["legends"][i].GetString(), response.data.legends[i]);
        }
        ASSERT_TRUE(jsonOptional.value()["body"]["lines"].IsArray());
        ASSERT_EQ(jsonOptional.value()["body"]["lines"].Size(), response.data.lines.size());
        for (size_t i = 0; i < response.data.lines.size(); ++i) {
            ASSERT_TRUE(jsonOptional.value()["body"]["lines"][i].IsArray());
            ASSERT_EQ(jsonOptional.value()["body"]["lines"][i].Size(), response.data.lines[i].size());
            for (size_t j = 0; j < response.data.lines[i].size(); ++j) {
                EXPECT_EQ(jsonOptional.value()["body"]["lines"][i][j].GetString(), response.data.lines[i][j]);
            }
        }
    }

    Dic::Protocol::MemoryProtocol memoryProtocol;
    const std::vector<Protocol::MemoryTableColumnAttr> tableColumnAttr = {
        {"Name", "string", "name"},
        {"Size(KB)", "number", "size"},
        {"Allocation Time(ms)", "number", "allocationTime"},
        {"Release Time(ms)", "number", "releaseTime"},
        {"Duration(ms)", "number", "duration"},
        {"Active Release Time(ms)", "number", "activeReleaseTime"},
        {"Active Duration(ms)", "number", "activeDuration"},
        {"Allocation Total Allocated(MB)", "number", "allocationAllocated"},
        {"Allocation Total Reserved(MB)", "number", "allocationReserved"},
        {"Allocation Total Active(MB)", "number", "allocationActive"},
        {"Release Total Allocated(MB)", "number", "releaseAllocated"},
        {"Release Total Reserved(MB)", "number", "releaseReserved"},
        {"Release Total Active(MB)", "number", "releaseActive"},
        {"Stream", "string", "streamId"}
    };
    const std::vector<Protocol::MemoryTableColumnAttr> staticOpTableColumnAttr = {
        {"Device ID", "string", "deviceId"},
        {"Name", "string", "opName"},
        {"Node Index Start", "number", "nodeIndexStart"},
        {"Node Index End", "number", "nodeIndexEnd"},
        {"Size(MB)", "number", "size"}
    };
    const std::vector<Protocol::MemoryTableColumnAttr> componentTableColumnAttr = {
        {"Component", "string", "component"},
        {"Peak Memory Reserved(MB)", "number", "totalReserved"},
        {"Timestamp(ms)", "number", "timestamp"}
    };
    const std::vector<std::string> graphLegends = {
        "Time (ms)", "Operators Allocated", "Operators Activated", "Operators Reserved"
    };
    const std::vector<std::string> staticGraphLegends = {
        "Node Index", "Size", "Total Size"
    };
};

TEST_F(MemoryProtocolTest, ToMemoryComponentRequestNormalTest)
{
    std::string reqJson = R"({"id": 10, "moduleName": "memory", "type": "request", "resultCallbackId": 0,
        "command": "Memory/view/component", "params": {"rankId": "2", "currentPage": 1, "pageSize": 100,
        "orderBy": "component", "order": "ascend"}})";
    MemoryComponentParams expect = {"2", "2", 1, 100, "component", "ascend", false};
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

TEST_F(MemoryProtocolTest, ToMemoryComponentRequestLackIdTestReturnNull)
{
    std::string reqJson = R"({"moduleName": "memory", "type": "request", "resultCallbackId": 0,
        "command": "Memory/view/component", "params": {"orderBy": "component", "order": "ascend"}})";
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    std::unique_ptr<Request> requestPtr = memoryProtocol.FromJson(json, err);
    EXPECT_EQ(requestPtr, nullptr);
    EXPECT_EQ(err, "Failed to set request base info, command is: Memory/view/component");
}

TEST_F(MemoryProtocolTest, ToMemoryComponentRequestLackRankIdTestReturnNull)
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

TEST_F(MemoryProtocolTest, ToMemoryOpeatorRequestNormalTest)
{
    std::string reqJson = R"({"id": 10, "moduleName": "memory", "type": "request", "resultCallbackId": 0,
        "command": "Memory/view/operator", "params": {"rankId": "3", "type": "Stream", "searchName": "aten::add",
        "minSize": -100, "maxSize": 1000, "startTime": 100.314, "endTime": 256.397, "currentPage": 100, "pageSize": 10,
        "orderBy": "size", "order": "ascend"}})";
    MemoryOperatorParams expect = {"3", "3", "Stream", "aten::add", -100, 1000, 100.314, 256.397, 100, 10,
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

TEST_F(MemoryProtocolTest, ToMemoryOpeatorRequestLackMinSizeAndMaxSizeTest)
{
    std::string reqJson = R"({"id": 10, "moduleName": "memory", "type": "request", "resultCallbackId": 0,
        "command": "Memory/view/operator", "params": {"rankId": "3", "type": "Stream", "searchName": "aten::add",
        "startTime": 100.314, "endTime": 256.397, "currentPage": 100, "pageSize": 10,
        "orderBy": "size", "order": "ascend"}})";
    MemoryOperatorParams expect = {"3", "3", "Stream", "aten::add", std::numeric_limits<int64_t>::min(),
        std::numeric_limits<int64_t>::max(), 100.314, 256.397, 100, 10, "size", "ascend", false};
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

TEST_F(MemoryProtocolTest, ToMemoryOpeatorRequestLackIdTestReturnNull)
{
    std::string reqJson = R"({"moduleName": "memory", "type": "request", "resultCallbackId": 0,
        "command": "Memory/view/operator", "params": {"rankId": "3", "type": "Stream", "searchName": "aten::add",
        "minSize": -100, "maxSize": 1000, "startTime": 100.314, "endTime": 256.397, "currentPage": 100, "pageSize": 10,
        "orderBy": "size", "order": "ascend"}})";
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    std::unique_ptr<Request> requestPtr = memoryProtocol.FromJson(json, err);
    EXPECT_EQ(requestPtr, nullptr);
    EXPECT_EQ(err, "Failed to set request base info, command is: Memory/view/operator");
}

TEST_F(MemoryProtocolTest, ToMemoryResourceTypeRequestNormalTest)
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

TEST_F(MemoryProtocolTest, ToMemoryResourceTypeRequestLackIdTestReturnNull)
{
    std::string reqJson = R"({"moduleName": "memory", "type": "request", "command": "Memory/view/resourceType",
        "resultCallbackId": 0, "params": {"rankId": "0"}})";
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    std::unique_ptr<Request> requestPtr = memoryProtocol.FromJson(json, err);
    EXPECT_EQ(requestPtr, nullptr);
    EXPECT_EQ(err, "Failed to set request base info, command is: Memory/view/resourceType");
}

TEST_F(MemoryProtocolTest, ToMemoryStaticOperatorGraphRequestNormalTest)
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

TEST_F(MemoryProtocolTest, ToMemoryStaticOperatorGraphRequestLackIdTestReturnNull)
{
    std::string reqJson = R"({"moduleName": "memory", "type": "request",
        "command": "Memory/view/staticOpMemoryGraph", "resultCallbackId": 0,
        "params": {"rankId": "1", "modelName": "0", "graphId": "0", "isCompare": true}})";
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    std::unique_ptr<Request> requestPtr = memoryProtocol.FromJson(json, err);
    EXPECT_EQ(requestPtr, nullptr);
    EXPECT_EQ(err, "Failed to set request base info, command is: Memory/view/staticOpMemoryGraph");
}

TEST_F(MemoryProtocolTest, ToMemoryStaticOperatorListRequestNormalTest)
{
    std::string reqJson = R"({"id": 10, "moduleName": "memory", "type": "request", "resultCallbackId": 0,
        "command": "Memory/view/staticOpMemoryList", "params": {"rankId": "3", "graphId": "0",
        "searchName": "model.layers.0.attention_norm.weight", "minSize": -1000, "maxSize": 10000,
        "startNodeIndex": 1, "endNodeIndex": 2533, "currentPage": 1000, "pageSize": 20,
        "orderBy": "op_name", "order": "descend"}})";
    StaticOperatorListParams expect = {"3", "0", "model.layers.0.attention_norm.weight",
        -1000, 10000, 1, 2533, 1000, 20, "op_name", "descend", false};
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    auto result = dynamic_cast<MemoryStaticOperatorListRequest &>(*(memoryProtocol.FromJson(json, err))).params;
    EXPECT_EQ(result.rankId, expect.rankId);
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

TEST_F(MemoryProtocolTest, ToMemoryStaticOperatorListRequestLackNodeIndexAndSizeTest)
{
    std::string reqJson = R"({"id": 10, "moduleName": "memory", "type": "request", "resultCallbackId": 0,
        "command": "Memory/view/staticOpMemoryList", "params": {"rankId": "3", "graphId": "0",
        "searchName": "model.layers.0.attention_norm.weight", "currentPage": 1000, "pageSize": 20,
        "orderBy": "op_name", "order": "descend"}})";
    StaticOperatorListParams expect = {"3", "0", "model.layers.0.attention_norm.weight",
        std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max(), -1, -1, 1000, 20,
        "op_name", "descend", false};
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    auto result = dynamic_cast<MemoryStaticOperatorListRequest &>(*(memoryProtocol.FromJson(json, err))).params;
    EXPECT_EQ(result.rankId, expect.rankId);
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

TEST_F(MemoryProtocolTest, ToMemoryStaticOperatorListRequestLackIdTestReturnNull)
{
    std::string reqJson = R"({"moduleName": "memory", "type": "request", "resultCallbackId": 0,
        "command": "Memory/view/staticOpMemoryList", "params": {"rankId": "3", "graphId": "0",
        "searchName": "model.layers.0.attention_norm.weight", "minSize": -1000, "maxSize": 10000,
        "startNodeIndex": 1, "endNodeIndex": 2533, "currentPage": 1000, "pageSize": 20,
        "orderBy": "op_name", "order": "descend"}})";
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    std::unique_ptr<Request> requestPtr = memoryProtocol.FromJson(json, err);
    EXPECT_EQ(requestPtr, nullptr);
    EXPECT_EQ(err, "Failed to set request base info, command is: Memory/view/staticOpMemoryList");
}

TEST_F(MemoryProtocolTest, ToMemoryStaticOperatorSizeRequestNormalTest)
{
    std::string reqJson = R"({"id": 2, "moduleName": "memory", "type": "request", "resultCallbackId": 0,
        "command": "Memory/view/staticOpMemorySize", "params": {"rankId": "1", "graphId": "0", "isCompare": false}})";
    StaticOperatorSizeParams expect = {"1", "0", false};
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    auto result = dynamic_cast<MemoryStaticOperatorSizeRequest &>(*(memoryProtocol.FromJson(json, err))).params;
    EXPECT_EQ(result.rankId, expect.rankId);
    EXPECT_EQ(result.graphId, expect.graphId);
    EXPECT_EQ(result.isCompare, expect.isCompare);
}

TEST_F(MemoryProtocolTest, ToMemoryStaticOperatorSizeRequestLackIdTestReturnNull)
{
    std::string reqJson = R"({"moduleName": "memory", "type": "request", "resultCallbackId": 0,
        "command": "Memory/view/staticOpMemorySize", "params": {"rankId": "1", "graphId": "0", "isCompare": false}})";
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    std::unique_ptr<Request> requestPtr = memoryProtocol.FromJson(json, err);
    EXPECT_EQ(requestPtr, nullptr);
    EXPECT_EQ(err, "Failed to set request base info, command is: Memory/view/staticOpMemorySize");
}

TEST_F(MemoryProtocolTest, ToMemoryTypeRequestNormalTest)
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

TEST_F(MemoryProtocolTest, ToMemoryTypeRequestLackIdTestReturnNull)
{
    std::string reqJson = R"({"moduleName": "memory", "type": "request", "command": "Memory/view/type",
        "resultCallbackId": 0, "params": {"rankId": "15"}})";
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    std::unique_ptr<Request> requestPtr= memoryProtocol.FromJson(json, err);
    EXPECT_EQ(requestPtr, nullptr);
    EXPECT_EQ(err, "Failed to set request base info, command is: Memory/view/type");
}

TEST_F(MemoryProtocolTest, ToMemoryViewRequestNormalTest)
{
    std::string reqJson = R"({"id": 2, "moduleName": "memory", "type": "request", "command": "Memory/view/memoryUsage",
        "resultCallbackId": 0, "params": {"rankId": "1", "type": "Overall", "isCompare": false}})";
    MemoryViewParams expect = {"1", "1", "Overall"};
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    auto result = dynamic_cast<MemoryViewRequest &>(*(memoryProtocol.FromJson(json, err))).params;
    EXPECT_EQ(result.rankId, expect.rankId);
    EXPECT_EQ(result.type, expect.type);
    EXPECT_EQ(result.isCompare, expect.isCompare);
}

TEST_F(MemoryProtocolTest, ToMemoryViewRequestLackIdTestReturnNull)
{
    std::string reqJson = R"({"moduleName": "memory", "type": "request", "command": "Memory/view/memoryUsage",
        "resultCallbackId": 0, "params": {"rankId": "1", "type": "Overall", "isCompare": false}})";
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    std::unique_ptr<Request> requestPtr = memoryProtocol.FromJson(json, err);
    EXPECT_EQ(requestPtr, nullptr);
    EXPECT_EQ(err, "Failed to set request base info, command is: Memory/view/memoryUsage");
}

TEST_F(MemoryProtocolTest, ToMemoryOperatorSizeRequestNormalTest)
{
    std::string reqJson = R"({"id": 2, "moduleName": "memory", "type": "request", "resultCallbackId": 0,
        "command": "Memory/view/operatorSize", "params": {"rankId": "1", "type": "Overall", "isCompare": false}})";
    MemoryOperatorSizeParams expect = {"1", "1", "Overall", false};
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    auto result = dynamic_cast<MemoryOperatorSizeRequest &>(*(memoryProtocol.FromJson(json, err))).params;
    EXPECT_EQ(result.rankId, expect.rankId);
    EXPECT_EQ(result.type, expect.type);
    EXPECT_EQ(result.isCompare, expect.isCompare);
}

TEST_F(MemoryProtocolTest, ToMemoryOperatorSizeRequestLackIdTestReturnNull)
{
    std::string reqJson = R"({"moduleName": "memory", "type": "request", "resultCallbackId": 0,
        "command": "Memory/view/operatorSize", "params": {"rankId": "1", "type": "Overall", "isCompare": false}})";
    Dic::document_t json;
    json.Parse(reqJson.c_str());
    std::string err;
    std::unique_ptr<Request> requestPtr = memoryProtocol.FromJson(json, err);
    EXPECT_EQ(requestPtr, nullptr);
    EXPECT_EQ(err, "Failed to set request base info, command is: Memory/view/operatorSize");
}

TEST_F(MemoryProtocolTest, ToMemoryComponentResponseEmptyDataTest)
{
    MemoryComponentComparisonResponse response;
    std::string err;
    response.columnAttr.clear();
    response.componentDiffDetails.clear();
    response.totalNum = 0;
    std::optional<document_t> jsonOptional = memoryProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
}

TEST_F(MemoryProtocolTest, ToMemoryComponentResponseNoComparisonDataTest)
{
    MemoryComponentComparisonResponse response;
    std::string err;
    response.columnAttr = componentTableColumnAttr;
    response.componentDiffDetails = {
        {{"SLOG", "132.459", 115.28, "NPU:0"}, {}, {}},
        {{"APP", "299.666", 19.79, "NPU:0"}, {}, {}},
    };
    response.totalNum = response.componentDiffDetails.size();
    std::optional<document_t> jsonOptional = memoryProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
    CheckComponentResponseStruct(jsonOptional, response);
}

TEST_F(MemoryProtocolTest, ToMemoryOperatorResponseEmptyDataTest)
{
    MemoryOperatorComparisonResponse response;
    std::string err;
    response.columnAttr.clear();
    response.operatorDiffDetails.clear();
    response.totalNum = 0;
    std::optional<document_t> jsonOptional = memoryProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
}

TEST_F(MemoryProtocolTest, ToMemoryOperatorResponseNoComparisonDataTest)
{
    MemoryOperatorComparisonResponse response;
    std::string err;
    response.columnAttr = tableColumnAttr;
    response.operatorDiffDetails = {
        {{"aten::add", 211.627, "113.791", "299.928", 186.137, "297.805", 184.014,
            147.956, 157.988, 201.017, 47.231, 56.338, 94.215, "14789652333", "NPU:0"}, {}, {}},
        {{"aten::matmul", 21.004, "119.791", "298.928", 179.137, "293.805", 174.014,
            25.346, 21.054, 20.337, 19.687, 15.276, 9.417, "14789652333277", "NPU:2"}, {}, {}},
    };
    response.totalNum = response.operatorDiffDetails.size();
    std::optional<document_t> jsonOptional = memoryProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
    CheckOperatorResponseStruct(jsonOptional, response);
}

TEST_F(MemoryProtocolTest, ToMemoryResourceTypeResponseEmptyDataTest)
{
    MemoryResourceTypeResponse response;
    std::string err;
    response.type = "";
    std::optional<document_t> jsonOptional = memoryProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
}

TEST_F(MemoryProtocolTest, ToMemoryResourceTypeResponsePyTorchDataTest)
{
    MemoryResourceTypeResponse response;
    std::string err;
    response.type = Module::Memory::MEMORY_RESOURCE_TYPE_PYTORCH;
    std::optional<document_t> jsonOptional = memoryProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
    ASSERT_TRUE(jsonOptional.value()["body"].HasMember("type"));
    EXPECT_EQ(jsonOptional.value()["body"]["type"].GetString(), response.type);
}

TEST_F(MemoryProtocolTest, ToMemoryResourceTypeResponseMindSporeDataTest)
{
    MemoryResourceTypeResponse response;
    std::string err;
    response.type = Module::Memory::MEMORY_RESOURCE_TYPE_MIND_SPORE;
    std::optional<document_t> jsonOptional = memoryProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
    ASSERT_TRUE(jsonOptional.value()["body"].HasMember("type"));
    EXPECT_EQ(jsonOptional.value()["body"]["type"].GetString(), response.type);
}

TEST_F(MemoryProtocolTest, ToMemoryStaticOperatorGraphResponseEmptyDataTest)
{
    MemoryStaticOperatorGraphResponse response;
    std::string err;
    response.data.legends.clear();
    response.data.lines.clear();
    std::optional<document_t> jsonOptional = memoryProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
}

TEST_F(MemoryProtocolTest, ToMemoryStaticOperatorGraphResponseNoComparisonDataTest)
{
    MemoryStaticOperatorGraphResponse response;
    std::string err;
    response.data.legends = staticGraphLegends;
    response.data.lines = {
        {"310", "4300.19", "122446.02"},
        {"1116", "3760.18", "122446.02"},
        {"2435", "88.58", "122446.02"}
    };
    std::optional<document_t> jsonOptional = memoryProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
    CheckStaticOperatorGraphResponseStruct(jsonOptional, response);
}

TEST_F(MemoryProtocolTest, ToMemoryStaticOperatorListResponseEmptyDataTest)
{
    MemoryStaticOperatorListCompResponse response;
    std::string err;
    response.columnAttr.clear();
    response.operatorDiffDetails.clear();
    response.totalNum = 0;
    std::optional<document_t> jsonOptional = memoryProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
}

TEST_F(MemoryProtocolTest, ToMemoryStaticOperatorListResponseNoComparisonDataTest)
{
    MemoryStaticOperatorListCompResponse response;
    std::string err;
    response.columnAttr = staticOpTableColumnAttr;
    response.operatorDiffDetails = {
        {{"host", "MatMul-op129", 1034, 2517, 200.7}, {}, {}},
        {{"host", "Cast-op32", 1, 967, 20000.478}, {}, {}},
    };
    response.totalNum = response.operatorDiffDetails.size();
    std::optional<document_t> jsonOptional = memoryProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
    CheckStaticOperatorListResponseStruct(jsonOptional, response);
}

TEST_F(MemoryProtocolTest, ToMemoryStaticOperatorSizeResponseEmptyDataTest)
{
    MemoryStaticOperatorSizeResponse response;
    std::string err;
    std::optional<document_t> jsonOptional = memoryProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
}

TEST_F(MemoryProtocolTest, ToMemoryStaticOperatorSizeResponseValidDataTest)
{
    MemoryStaticOperatorSizeResponse response;
    std::string err;
    response.size.minSize = -1.0;
    response.size.maxSize = 1.0;
    std::optional<document_t> jsonOptional = memoryProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
    ASSERT_TRUE(jsonOptional.value()["body"].HasMember("minSize"));
    ASSERT_TRUE(jsonOptional.value()["body"].HasMember("maxSize"));
    EXPECT_EQ(jsonOptional.value()["body"]["minSize"].GetDouble(), response.size.minSize);
    EXPECT_EQ(jsonOptional.value()["body"]["maxSize"].GetDouble(), response.size.maxSize);
}

TEST_F(MemoryProtocolTest, ToMemoryTypeResponseEmptyDataTest)
{
    MemoryTypeResponse response;
    std::string err;
    response.type = "";
    response.graphId.clear();
    std::optional<document_t> jsonOptional = memoryProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
}

TEST_F(MemoryProtocolTest, ToMemoryTypeResponseDynamicComputationalGraphDataTest)
{
    MemoryTypeResponse response;
    std::string err;
    response.type = Module::Memory::MEMORY_TYPE_DYNAMIC;
    response.graphId = {};
    std::optional<document_t > jsonOptional = memoryProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
    ASSERT_TRUE(jsonOptional.value()["body"].HasMember("type"));
    EXPECT_EQ(jsonOptional.value()["body"]["type"].GetString(), response.type);
}

TEST_F(MemoryProtocolTest, ToMemoryTypeResponseStaticComputationalGraphDataTest)
{
    MemoryTypeResponse response;
    std::string err;
    response.type = Module::Memory::MEMORY_TYPE_STATIC;
    response.graphId = {"1", "2"};
    std::optional<document_t> jsonOptional = memoryProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
    ASSERT_TRUE(jsonOptional.value()["body"].HasMember("type"));
    EXPECT_EQ(jsonOptional.value()["body"]["type"].GetString(), response.type);
    ASSERT_TRUE(jsonOptional.value()["body"].HasMember("graphId"));
    ASSERT_TRUE(jsonOptional.value()["body"]["graphId"].IsArray());
    ASSERT_EQ(jsonOptional.value()["body"]["graphId"].Size(), response.graphId.size());
    EXPECT_EQ(jsonOptional.value()["body"]["graphId"][0].GetString(), response.graphId[0]);
    EXPECT_EQ(jsonOptional.value()["body"]["graphId"][1].GetString(), response.graphId[1]);
}

TEST_F(MemoryProtocolTest, ToMemoryViewResponseEmptyDataTest)
{
    MemoryViewResponse response;
    std::string err;
    response.data.title.clear();
    response.data.legends.clear();
    response.data.lines.clear();
    std::optional<document_t> jsonOptional = memoryProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
}

TEST_F(MemoryProtocolTest, ToMemoryViewResponseNoComparisonDataTest)
{
    MemoryViewResponse response;
    std::string err;
    response.data.title = "Peak Memory Usage: Operators Allocated: 25531.52MB | Operators Activated: 25621.52MB |"
                          " Operators Reserved: 26370.00MB | APP Reserved: 27743.03MB";
    response.data.legends = graphLegends;
    response.data.lines = {
        {"126.550", "18182.40", "18182.40", "25750.00", "NULL"},
        {"181.221", "18859.86", "18859.86", "25750.00", "NULL"},
        {"740.047", "NULL", "NULL", "NULL", "26742.36"}
    };
    std::optional<document_t> jsonOptional = memoryProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
    CheckViewResponseStruct(jsonOptional, response);
}

TEST_F(MemoryProtocolTest, ToMemoryOperatorSizeResponseEmptyDataTest)
{
    MemoryOperatorSizeResponse response;
    std::string err;
    std::optional<document_t> jsonOptional = memoryProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
}

TEST_F(MemoryProtocolTest, ToMemoryOperatorSizeResponseValidDataTest)
{
    MemoryOperatorSizeResponse response;
    std::string err;
    response.size.minSize = -1.0;
    response.size.maxSize = 1.0;
    std::optional<document_t> jsonOptional = memoryProtocol.ToJson(response, err);
    CheckResponseBaseStruct(jsonOptional);
    ASSERT_TRUE(jsonOptional.value()["body"].HasMember("minSize"));
    ASSERT_TRUE(jsonOptional.value()["body"].HasMember("maxSize"));
    EXPECT_EQ(jsonOptional.value()["body"]["minSize"].GetDouble(), response.size.minSize);
    EXPECT_EQ(jsonOptional.value()["body"]["maxSize"].GetDouble(), response.size.maxSize);
}

TEST_F(MemoryProtocolTest, ToMemoryFindSliceRequest)
{
    const uint64_t tempId = 89;
    Dic::Protocol::MemoryProtocol protocol;
    protocol.Register();
    std::string error;
    Dic::document_t json(Dic::kObjectType);
    auto &allocator = json.GetAllocator();
    Dic::JsonUtil::AddMember(json, "type", "request", allocator);
    Dic::JsonUtil::AddMember(json, "command", "Memory/find/slice", allocator);
    protocol.FromJson(json, error);

    Dic::json_t params(Dic::kObjectType);
    Dic::JsonUtil::AddMember(params, "rankId", "mmmmm", allocator);
    Dic::JsonUtil::AddMember(params, "id", "89", allocator);
    Dic::JsonUtil::AddMember(params, "name", "HHHHHHHH", allocator);
    Dic::JsonUtil::AddMember(json, "id", tempId, allocator);
    Dic::JsonUtil::AddMember(json, "moduleName", "hhh", allocator);
    Dic::JsonUtil::AddMember(json, "params", params, allocator);
    unsigned int id = protocol.FromJson(json, error)->id;
    std::string memoryId = dynamic_cast<MemoryFindSliceRequest &>(*protocol.FromJson(json, error)).params.id;
    std::string memoryRankId = dynamic_cast<MemoryFindSliceRequest &>(*protocol.FromJson(json, error)).params.rankId;
    std::string memoryName = dynamic_cast<MemoryFindSliceRequest &>(*protocol.FromJson(json, error)).params.name;
    EXPECT_EQ(id, tempId);
    EXPECT_EQ(memoryId, "89");
    EXPECT_EQ(memoryRankId, "mmmmm");
    EXPECT_EQ(memoryName, "HHHHHHHH");
}

/**
 * 测试MemoryFindSliceResponseJson的error情况
 */
TEST_F(MemoryProtocolTest, TestTMemoryFindSliceResponseError)
{
    Dic::Protocol::MemoryFindSliceResponse response;
    response.result = false;
    const uint64_t errorCode = 3;
    Dic::Protocol::ErrorMessage error = { errorCode, "ll" };
    response.error = error;
    auto jsonOp = Dic::Protocol::ToResponseJson(response);
    EXPECT_EQ(jsonOp.has_value(), true);
    const std::string json = Dic::JsonUtil::JsonDump(jsonOp.value());
    const std::string jsonStr =
        "{\"type\":\"response\",\"id\":0,\"requestId\":0,\"result\":false,\"command\":\"Memory/find/"
        "slice\",\"moduleName\":\"unknown\",\"message\":\"ll\",\"error\":{\"code\":3},\"body\":{\"id\":\"\",\"rankId\":"
        "\"\",\"processId\":\"\",\"threadId\":\"\",\"metaType\":\"\",\"depth\":0,\"startTime\":0,\"duration\":0}}";
    EXPECT_EQ(json, jsonStr);
}