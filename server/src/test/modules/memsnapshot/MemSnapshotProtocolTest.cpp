/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

#include "../../TestSuit.h"
#include "MemSnapshotDefs.h"
#include "MemSnapshotProtocolRequest.h"
#include "MemSnapshotProtocolResponse.h"

class MemSnapshotProtocolTest : public ::testing::Test {
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
};

TEST_F(MemSnapshotProtocolTest, BuildBlocksTableRequestFromJson)
{
    std::string jsonStr = "{"
                          "  \"id\": 21, "
                          "  \"moduleName\": \"leaks\", "
                          "  \"type\": \"request\", "
                          "  \"command\": \"Memory/snapshot/blocks\", "
                          "  \"fileId\": \"\", "
                          "  \"projectName\": \"/home/test/data/memsnapshot/test.db\", "
                          "  \"params\": { "
                          "    \"deviceId\": \"1\", "
                          "    \"eventType\": \"PTA\", "
                          "    \"isTable\": true, "
                          "    \"startTimestamp\": 1000, "
                          "    \"endTimestamp\": 50000, "
                          "    \"minSize\": 1024, "
                          "    \"maxSize\": 1048576, "
                          "    \"currentPage\": 1, "
                          "    \"pageSize\": 10, "
                          "    \"desc\": false, "
                          "    \"orderBy\": \"allocEventId\", "
                          "    \"filters\": { "
                          "      \"state\": \"active_allocated\" "
                          "    } "
                          "  } "
                          "}";
    std::string errMsg;
    auto json = JsonUtil::TryParse(jsonStr, errMsg);
    EXPECT_TRUE(errMsg.empty());
    EXPECT_FALSE(json->HasParseError());
    EXPECT_TRUE(json.has_value());
    auto requestPtr = Dic::Protocol::MemSnapshotBlocksRequest::FromJson(json.value(), errMsg);
    EXPECT_TRUE(errMsg.empty());
    auto &request = dynamic_cast<Dic::Protocol::MemSnapshotBlocksRequest &>(*requestPtr);
    EXPECT_TRUE(request.params.CommonCheck(errMsg));
    EXPECT_EQ(request.params.currentPage, 1);
    EXPECT_EQ(request.params.pageSize, 10);
    EXPECT_EQ(request.params.orderBy, "allocEventId");
    EXPECT_EQ(request.params.filters.size(), 1);
    EXPECT_EQ(request.params.startEventIdx, 1000);
    EXPECT_EQ(request.params.endEventIdx, 50000);
    EXPECT_EQ(request.params.minSize, 1024);
    EXPECT_EQ(request.params.maxSize, 1048576);
    EXPECT_EQ(request.params.deviceId, "1");
    EXPECT_EQ(request.params.eventType, "PTA");
    EXPECT_TRUE(request.isTable);
}

TEST_F(MemSnapshotProtocolTest, BuildBlocksViewRequestFromJson)
{
    std::string jsonStr = "{"
                          "  \"id\": 22, "
                          "  \"moduleName\": \"leaks\", "
                          "  \"type\": \"request\", "
                          "  \"command\": \"Memory/snapshot/blocks\", "
                          "  \"fileId\": \"\", "
                          "  \"projectName\": \"/home/test/data/memsnapshot/test.db\", "
                          "  \"params\": { "
                          "    \"deviceId\": \"1\", "
                          "    \"eventType\": \"PTA\", "
                          "    \"isTable\": false, "
                          "    \"startTimestamp\": 1000, "
                          "    \"endTimestamp\": 50000 "
                          "  } "
                          "}";
    std::string errMsg;
    auto json = JsonUtil::TryParse(jsonStr, errMsg);
    EXPECT_TRUE(errMsg.empty());
    EXPECT_FALSE(json->HasParseError());
    EXPECT_TRUE(json.has_value());
    auto requestPtr = Dic::Protocol::MemSnapshotBlocksRequest::FromJson(json.value(), errMsg);
    EXPECT_TRUE(errMsg.empty());
    auto &request = dynamic_cast<Dic::Protocol::MemSnapshotBlocksRequest &>(*requestPtr);
    EXPECT_TRUE(request.params.CommonCheck(errMsg));
    EXPECT_EQ(request.params.deviceId, "1");
    EXPECT_EQ(request.params.eventType, "PTA");
    EXPECT_FALSE(request.isTable);
    EXPECT_EQ(request.params.orderBy, "allocEventId");
}

TEST_F(MemSnapshotProtocolTest, BuildEventsTableRequestFromJson)
{
    std::string jsonStr = "{"
                          "  \"id\": 23, "
                          "  \"moduleName\": \"leaks\", "
                          "  \"type\": \"request\", "
                          "  \"command\": \"Memory/snapshot/events\", "
                          "  \"fileId\": \"\", "
                          "  \"projectName\": \"/home/test/data/memsnapshot/test.db\", "
                          "  \"params\": { "
                          "    \"deviceId\": \"1\", "
                          "    \"isTable\": true, "
                          "    \"startTimestamp\": 1000, "
                          "    \"endTimestamp\": 50000, "
                          "    \"currentPage\": 1, "
                          "    \"pageSize\": 10, "
                          "    \"desc\": false, "
                          "    \"orderBy\": \"id\", "
                          "    \"filters\": { "
                          "      \"action\": \"alloc\" "
                          "    } "
                          "  } "
                          "}";
    std::string errMsg;
    auto json = JsonUtil::TryParse(jsonStr, errMsg);
    EXPECT_TRUE(errMsg.empty());
    EXPECT_FALSE(json->HasParseError());
    EXPECT_TRUE(json.has_value());
    auto requestPtr = Dic::Protocol::MemSnapshotEventsRequest::FromJson(json.value(), errMsg);
    EXPECT_TRUE(errMsg.empty());
    auto &request = dynamic_cast<Dic::Protocol::MemSnapshotEventsRequest &>(*requestPtr);
    EXPECT_TRUE(request.params.CommonCheck(errMsg));
    EXPECT_EQ(request.params.currentPage, 1);
    EXPECT_EQ(request.params.pageSize, 10);
    EXPECT_EQ(request.params.orderBy, "id");
    EXPECT_EQ(request.params.filters.size(), 1);
    EXPECT_EQ(request.params.startEventIdx, 1000);
    EXPECT_EQ(request.params.endEventIdx, 50000);
    EXPECT_EQ(request.params.deviceId, "1");
    EXPECT_TRUE(request.isTable);
}

TEST_F(MemSnapshotProtocolTest, BuildEventsViewRequestFromJson)
{
    std::string jsonStr = "{"
                          "  \"id\": 24, "
                          "  \"moduleName\": \"leaks\", "
                          "  \"type\": \"request\", "
                          "  \"command\": \"Memory/snapshot/events\", "
                          "  \"fileId\": \"\", "
                          "  \"projectName\": \"/home/test/data/memsnapshot/test.db\", "
                          "  \"params\": { "
                          "    \"deviceId\": \"1\", "
                          "    \"isTable\": false, "
                          "    \"startTimestamp\": 1000, "
                          "    \"endTimestamp\": 50000, "
                          "    \"currentPage\": 1, "
                          "    \"pageSize\": 10 "
                          "  } "
                          "}";
    std::string errMsg;
    auto json = JsonUtil::TryParse(jsonStr, errMsg);
    EXPECT_TRUE(errMsg.empty());
    EXPECT_FALSE(json->HasParseError());
    EXPECT_TRUE(json.has_value());
    auto requestPtr = Dic::Protocol::MemSnapshotEventsRequest::FromJson(json.value(), errMsg);
    EXPECT_TRUE(errMsg.empty());
    auto &request = dynamic_cast<Dic::Protocol::MemSnapshotEventsRequest &>(*requestPtr);
    EXPECT_TRUE(request.params.CommonCheck(errMsg));
    EXPECT_EQ(request.params.deviceId, "1");
    EXPECT_FALSE(request.isTable);
    EXPECT_EQ(request.params.orderBy, "id");
}

TEST_F(MemSnapshotProtocolTest, BuildAllocationsRequestFromJson)
{
    std::string jsonStr = "{"
                          "  \"id\": 25, "
                          "  \"moduleName\": \"leaks\", "
                          "  \"type\": \"request\", "
                          "  \"command\": \"Memory/snapshot/allocations\", "
                          "  \"fileId\": \"\", "
                          "  \"projectName\": \"/home/test/data/memsnapshot/test.db\", "
                          "  \"params\": { "
                          "    \"deviceId\": \"1\", "
                          "    \"eventType\": \"PTA\" "
                          "  } "
                          "}";
    std::string errMsg;
    auto json = JsonUtil::TryParse(jsonStr, errMsg);
    EXPECT_TRUE(errMsg.empty());
    EXPECT_FALSE(json->HasParseError());
    EXPECT_TRUE(json.has_value());
    auto requestPtr = Dic::Protocol::MemSnapshotAllocationsRequest::FromJson(json.value(), errMsg);
    EXPECT_TRUE(errMsg.empty());
    auto &request = dynamic_cast<Dic::Protocol::MemSnapshotAllocationsRequest &>(*requestPtr);
    EXPECT_TRUE(request.params.CommonCheck(errMsg));
    EXPECT_EQ(request.params.deviceId, "1");
    EXPECT_EQ(request.params.eventType, "PTA");
}

TEST_F(MemSnapshotProtocolTest, BlocksParamsCommonCheckInvalidMinSize)
{
    std::string jsonStr = "{"
                          "  \"id\": 26, "
                          "  \"moduleName\": \"leaks\", "
                          "  \"type\": \"request\", "
                          "  \"command\": \"Memory/snapshot/blocks\", "
                          "  \"fileId\": \"\", "
                          "  \"projectName\": \"/home/test/data/memsnapshot/test.db\", "
                          "  \"params\": { "
                          "    \"deviceId\": \"1\", "
                          "    \"eventType\": \"PTA\", "
                          "    \"minSize\": 1048576, "
                          "    \"maxSize\": 1024 "
                          "  } "
                          "}";
    std::string errMsg;
    auto json = JsonUtil::TryParse(jsonStr, errMsg);
    EXPECT_TRUE(errMsg.empty());
    EXPECT_TRUE(json.has_value());
    auto requestPtr = Dic::Protocol::MemSnapshotBlocksRequest::FromJson(json.value(), errMsg);
    EXPECT_TRUE(errMsg.empty());
    auto &request = dynamic_cast<Dic::Protocol::MemSnapshotBlocksRequest &>(*requestPtr);
    EXPECT_FALSE(request.params.CommonCheck(errMsg));
    EXPECT_FALSE(errMsg.empty());
    EXPECT_TRUE(errMsg.find("minSize") != std::string::npos);
}

TEST_F(MemSnapshotProtocolTest, BlocksParamsCommonlyCheckInvalidEventIdx)
{
    std::string jsonStr = "{"
                          "  \"id\": 27, "
                          "  \"moduleName\": \"leaks\", "
                          "  \"type\": \"request\", "
                          "  \"command\": \"Memory/snapshot/blocks\", "
                          "  \"fileId\": \"\", "
                          "  \"projectName\": \"/home/test/data/memsnapshot/test.db\", "
                          "  \"params\": { "
                          "    \"deviceId\": \"1\", "
                          "    \"eventType\": \"PTA\", "
                          "    \"startTimestamp\": 50000, "
                          "    \"endTimestamp\": 1000 "
                          "  } "
                          "}";
    std::string errMsg;
    auto json = JsonUtil::TryParse(jsonStr, errMsg);
    EXPECT_TRUE(errMsg.empty());
    EXPECT_TRUE(json.has_value());
    auto requestPtr = Dic::Protocol::MemSnapshotBlocksRequest::FromJson(json.value(), errMsg);
    EXPECT_TRUE(errMsg.empty());
    auto &request = dynamic_cast<Dic::Protocol::MemSnapshotBlocksRequest &>(*requestPtr);
    EXPECT_FALSE(request.params.CommonCheck(errMsg));
    EXPECT_FALSE(errMsg.empty());
    EXPECT_TRUE(errMsg.find("start idx") != std::string::npos);
}

TEST_F(MemSnapshotProtocolTest, BlocksRequestMissingRequiredParams)
{
    std::string jsonStr = "{"
                          "  \"id\": 28, "
                          "  \"moduleName\": \"leaks\", "
                          "  \"type\": \"request\", "
                          "  \"command\": \"Memory/snapshot/blocks\", "
                          "  \"fileId\": \"\", "
                          "  \"projectName\": \"/home/test/data/memsnapshot/test.db\", "
                          "  \"params\": { "
                          "    \"eventType\": \"PTA\" "
                          "  } "
                          "}";
    std::string errMsg;
    auto json = JsonUtil::TryParse(jsonStr, errMsg);
    EXPECT_TRUE(errMsg.empty());
    EXPECT_TRUE(json.has_value());
    auto requestPtr = Dic::Protocol::MemSnapshotBlocksRequest::FromJson(json.value(), errMsg);
    EXPECT_FALSE(errMsg.empty());
    EXPECT_TRUE(errMsg.find("deviceId") != std::string::npos);
}

TEST_F(MemSnapshotProtocolTest, EventsRequestMissingRequiredParams)
{
    std::string jsonStr = "{"
                          "  \"id\": 29, "
                          "  \"moduleName\": \"leaks\", "
                          "  \"type\": \"request\", "
                          "  \"command\": \"Memory/snapshot/events\", "
                          "  \"fileId\": \"\", "
                          "  \"projectName\": \"/home/test/data/memsnapshot/test.db\", "
                          "  \"params\": { "
                          "  } "
                          "}";
    std::string errMsg;
    auto json = JsonUtil::TryParse(jsonStr, errMsg);
    EXPECT_TRUE(errMsg.empty());
    EXPECT_TRUE(json.has_value());
    auto requestPtr = Dic::Protocol::MemSnapshotEventsRequest::FromJson(json.value(), errMsg);
    EXPECT_FALSE(errMsg.empty());
    EXPECT_TRUE(errMsg.find("deviceId") != std::string::npos);
}

TEST_F(MemSnapshotProtocolTest, AllocationsRequestMissingRequiredParams)
{
    std::string jsonStr = "{"
                          "  \"id\": 30, "
                          "  \"moduleName\": \"leaks\", "
                          "  \"type\": \"request\", "
                          "  \"command\": \"Memory/snapshot/allocations\", "
                          "  \"fileId\": \"\", "
                          "  \"projectName\": \"/home/test/data/memsnapshot/test.db\", "
                          "  \"params\": { "
                          "    \"deviceId\": \"1\" "
                          "  } "
                          "}";
    std::string errMsg;
    auto json = JsonUtil::TryParse(jsonStr, errMsg);
    EXPECT_TRUE(errMsg.empty());
    EXPECT_TRUE(json.has_value());
    auto requestPtr = Dic::Protocol::MemSnapshotAllocationsRequest::FromJson(json.value(), errMsg);
    EXPECT_FALSE(errMsg.empty());
    EXPECT_TRUE(errMsg.find("eventType") != std::string::npos);
}
