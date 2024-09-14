/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "ProtocolTest.cpp"
#include "TimelineProtocol.h"
#include "JsonUtil.h"
#include "ProtocolDefs.h"
#include "TimelineProtocolResponse.h"
#include "TimelineProtocolEvent.h"

class TimelineProtocolTest : ProtocolTest {};

TEST_F(ProtocolTest, ToImportActionRequestTest)
{
    const uint64_t tempId = 89;
    Dic::Protocol::TimelineProtocol timelineProtocol;
    timelineProtocol.Register();
    std::string error;
    Dic::document_t json(Dic::kObjectType);
    auto &allocator = json.GetAllocator();
    Dic::JsonUtil::AddMember(json, "type", "request", allocator);
    Dic::JsonUtil::AddMember(json, "command", "import/action", allocator);
    timelineProtocol.FromJson(json, error);

    Dic::json_t params(Dic::kObjectType);
    Dic::json_t path(Dic::kArrayType);
    path.PushBack("kkkkkk", allocator);
    Dic::JsonUtil::AddMember(params, "path", path, allocator);
    Dic::JsonUtil::AddMember(json, "id", tempId, allocator);
    Dic::JsonUtil::AddMember(json, "moduleName", "hhh", allocator);
    Dic::JsonUtil::AddMember(json, "params", params, allocator);
    unsigned int id = timelineProtocol.FromJson(json, error).get()->id;
    EXPECT_EQ(id, tempId);
}
TEST_F(ProtocolTest, ToUnitThreadTracesRequestTest)
{
    const uint64_t tempId = 89;
    Dic::Protocol::TimelineProtocol timelineProtocol;
    timelineProtocol.Register();
    std::string error;
    Dic::document_t json(Dic::kObjectType);
    auto &allocator = json.GetAllocator();
    Dic::JsonUtil::AddMember(json, "type", "request", allocator);
    Dic::JsonUtil::AddMember(json, "command", "unit/threadTraces", allocator);
    timelineProtocol.FromJson(json, error);

    Dic::json_t params(Dic::kObjectType);
    Dic::JsonUtil::AddMember(json, "id", tempId, allocator);
    Dic::JsonUtil::AddMember(json, "moduleName", "hhh", allocator);
    Dic::JsonUtil::AddMember(json, "params", params, allocator);
    unsigned int id = timelineProtocol.FromJson(json, error).get()->id;
    EXPECT_EQ(id, tempId);
}
TEST_F(ProtocolTest, ToUnitThreadTracesSummaryRequest)
{
    const uint64_t tempId = 89;
    Dic::Protocol::TimelineProtocol timelineProtocol;
    timelineProtocol.Register();
    std::string error;
    Dic::document_t json(Dic::kObjectType);
    auto &allocator = json.GetAllocator();
    Dic::JsonUtil::AddMember(json, "type", "request", allocator);
    Dic::JsonUtil::AddMember(json, "command", "unit/threadTracesSummary", allocator);
    timelineProtocol.FromJson(json, error);

    Dic::json_t params(Dic::kObjectType);
    Dic::JsonUtil::AddMember(json, "id", tempId, allocator);
    Dic::JsonUtil::AddMember(json, "moduleName", "hhh", allocator);
    Dic::JsonUtil::AddMember(json, "params", params, allocator);
    unsigned int id = timelineProtocol.FromJson(json, error).get()->id;
    EXPECT_EQ(id, tempId);
}
TEST_F(ProtocolTest, ToUnitThreadsRequest)
{
    const uint64_t tempId = 89;
    Dic::Protocol::TimelineProtocol timelineProtocol;
    timelineProtocol.Register();
    std::string error;
    Dic::document_t json(Dic::kObjectType);
    auto &allocator = json.GetAllocator();
    Dic::JsonUtil::AddMember(json, "type", "request", allocator);
    Dic::JsonUtil::AddMember(json, "command", "unit/threads", allocator);
    timelineProtocol.FromJson(json, error);

    Dic::json_t params(Dic::kObjectType);
    Dic::JsonUtil::AddMember(json, "id", tempId, allocator);
    Dic::JsonUtil::AddMember(json, "moduleName", "hhh", allocator);
    Dic::JsonUtil::AddMember(json, "params", params, allocator);
    unsigned int id = timelineProtocol.FromJson(json, error).get()->id;
    EXPECT_EQ(id, tempId);
}
TEST_F(ProtocolTest, ToThreadDetailRequest)
{
    const uint64_t tempId = 89;
    Dic::Protocol::TimelineProtocol timelineProtocol;
    timelineProtocol.Register();
    std::string error;
    Dic::document_t json(Dic::kObjectType);
    auto &allocator = json.GetAllocator();
    Dic::JsonUtil::AddMember(json, "type", "request", allocator);
    Dic::JsonUtil::AddMember(json, "command", "unit/threadDetail", allocator);
    timelineProtocol.FromJson(json, error);

    Dic::json_t params(Dic::kObjectType);
    Dic::JsonUtil::AddMember(json, "id", tempId, allocator);
    Dic::JsonUtil::AddMember(json, "moduleName", "hhh", allocator);
    Dic::JsonUtil::AddMember(json, "params", params, allocator);
    unsigned int id = timelineProtocol.FromJson(json, error).get()->id;
    EXPECT_EQ(id, tempId);
}

TEST_F(ProtocolTest, ToResetWindowRequest)
{
    const uint64_t tempId = 89;
    Dic::Protocol::TimelineProtocol timelineProtocol;
    timelineProtocol.Register();
    std::string error;
    Dic::document_t json(Dic::kObjectType);
    auto &allocator = json.GetAllocator();
    Dic::JsonUtil::AddMember(json, "type", "request", allocator);
    Dic::JsonUtil::AddMember(json, "command", "remote/reset", allocator);
    timelineProtocol.FromJson(json, error);

    Dic::json_t params(Dic::kObjectType);
    Dic::JsonUtil::AddMember(json, "id", tempId, allocator);
    Dic::JsonUtil::AddMember(json, "moduleName", "hhh", allocator);
    Dic::JsonUtil::AddMember(json, "params", params, allocator);
    unsigned int id = timelineProtocol.FromJson(json, error).get()->id;
    EXPECT_EQ(id, tempId);
}
TEST_F(ProtocolTest, ToSearchCountRequest)
{
    const uint64_t tempId = 89;
    Dic::Protocol::TimelineProtocol timelineProtocol;
    timelineProtocol.Register();
    std::string error;
    Dic::document_t json(Dic::kObjectType);
    auto &allocator = json.GetAllocator();
    Dic::JsonUtil::AddMember(json, "type", "request", allocator);
    Dic::JsonUtil::AddMember(json, "command", "search/count", allocator);
    timelineProtocol.FromJson(json, error);

    Dic::json_t params(Dic::kObjectType);
    Dic::JsonUtil::AddMember(json, "id", tempId, allocator);
    Dic::JsonUtil::AddMember(json, "moduleName", "hhh", allocator);
    Dic::JsonUtil::AddMember(json, "params", params, allocator);
    unsigned int id = timelineProtocol.FromJson(json, error).get()->id;
    EXPECT_EQ(id, tempId);
}
TEST_F(ProtocolTest, ToSearchSliceRequest)
{
    const uint64_t tempId = 89;
    Dic::Protocol::TimelineProtocol timelineProtocol;
    timelineProtocol.Register();
    std::string error;
    Dic::document_t json(Dic::kObjectType);
    auto &allocator = json.GetAllocator();
    Dic::JsonUtil::AddMember(json, "type", "request", allocator);
    Dic::JsonUtil::AddMember(json, "command", "search/slice", allocator);
    timelineProtocol.FromJson(json, error);

    Dic::json_t params(Dic::kObjectType);
    Dic::JsonUtil::AddMember(json, "id", tempId, allocator);
    Dic::JsonUtil::AddMember(json, "moduleName", "hhh", allocator);
    Dic::JsonUtil::AddMember(json, "params", params, allocator);
    unsigned int id = timelineProtocol.FromJson(json, error).get()->id;
    EXPECT_EQ(id, tempId);
}
TEST_F(ProtocolTest, ToRemoteDeleteRequest)
{
    const uint64_t tempId = 89;
    Dic::Protocol::TimelineProtocol timelineProtocol;
    timelineProtocol.Register();
    std::string error;
    Dic::document_t json(Dic::kObjectType);
    auto &allocator = json.GetAllocator();
    Dic::JsonUtil::AddMember(json, "type", "request", allocator);
    Dic::JsonUtil::AddMember(json, "command", "remote/delete", allocator);
    timelineProtocol.FromJson(json, error);

    Dic::json_t params(Dic::kObjectType);
    Dic::json_t rankId(Dic::kArrayType);
    rankId.PushBack("kkkkkk", allocator);
    Dic::JsonUtil::AddMember(params, "777", rankId, allocator);
    Dic::JsonUtil::AddMember(json, "id", tempId, allocator);
    Dic::JsonUtil::AddMember(json, "moduleName", "hhh", allocator);
    Dic::JsonUtil::AddMember(json, "params", params, allocator);
    unsigned int id = timelineProtocol.FromJson(json, error).get()->id;
    EXPECT_EQ(id, tempId);
}
TEST_F(ProtocolTest, ToFlowCategoryListRequest)
{
    const uint64_t tempId = 89;
    Dic::Protocol::TimelineProtocol timelineProtocol;
    timelineProtocol.Register();
    std::string error;
    Dic::document_t json(Dic::kObjectType);
    auto &allocator = json.GetAllocator();
    Dic::JsonUtil::AddMember(json, "type", "request", allocator);
    Dic::JsonUtil::AddMember(json, "command", "flow/categoryList", allocator);
    timelineProtocol.FromJson(json, error);

    Dic::json_t params(Dic::kObjectType);
    Dic::JsonUtil::AddMember(json, "id", tempId, allocator);
    Dic::JsonUtil::AddMember(json, "moduleName", "hhh", allocator);
    Dic::JsonUtil::AddMember(json, "params", params, allocator);
    unsigned int id = timelineProtocol.FromJson(json, error).get()->id;
    EXPECT_EQ(id, tempId);
}
TEST_F(ProtocolTest, ToFlowCategoryEventsRequest)
{
    const uint64_t tempId = 89;
    Dic::Protocol::TimelineProtocol timelineProtocol;
    timelineProtocol.Register();
    std::string error;
    Dic::document_t json(Dic::kObjectType);
    auto &allocator = json.GetAllocator();
    Dic::JsonUtil::AddMember(json, "type", "request", allocator);
    Dic::JsonUtil::AddMember(json, "command", "flow/categoryEvents", allocator);
    timelineProtocol.FromJson(json, error);

    Dic::json_t params(Dic::kObjectType);
    Dic::JsonUtil::AddMember(json, "id", tempId, allocator);
    Dic::JsonUtil::AddMember(json, "moduleName", "hhh", allocator);
    Dic::JsonUtil::AddMember(json, "params", params, allocator);
    unsigned int id = timelineProtocol.FromJson(json, error).get()->id;
    EXPECT_EQ(id, tempId);
}
TEST_F(ProtocolTest, ToUnitCounterRequest)
{
    const uint64_t tempId = 89;
    Dic::Protocol::TimelineProtocol timelineProtocol;
    timelineProtocol.Register();
    std::string error;
    Dic::document_t json(Dic::kObjectType);
    auto &allocator = json.GetAllocator();
    Dic::JsonUtil::AddMember(json, "type", "request", allocator);
    Dic::JsonUtil::AddMember(json, "command", "unit/counter", allocator);
    timelineProtocol.FromJson(json, error);

    Dic::json_t params(Dic::kObjectType);
    Dic::JsonUtil::AddMember(json, "id", tempId, allocator);
    Dic::JsonUtil::AddMember(json, "moduleName", "hhh", allocator);
    Dic::JsonUtil::AddMember(json, "params", params, allocator);
    unsigned int id = timelineProtocol.FromJson(json, error).get()->id;
    EXPECT_EQ(id, tempId);
}
TEST_F(ProtocolTest, ToSystemViewRequest)
{
    const uint64_t tempId = 89;
    Dic::Protocol::TimelineProtocol timelineProtocol;
    timelineProtocol.Register();
    std::string error;
    Dic::document_t json(Dic::kObjectType);
    auto &allocator = json.GetAllocator();
    Dic::JsonUtil::AddMember(json, "type", "request", allocator);
    Dic::JsonUtil::AddMember(json, "command", "unit/systemView", allocator);
    timelineProtocol.FromJson(json, error);

    Dic::json_t params(Dic::kObjectType);
    Dic::JsonUtil::AddMember(json, "id", tempId, allocator);
    Dic::JsonUtil::AddMember(json, "moduleName", "hhh", allocator);
    Dic::JsonUtil::AddMember(json, "params", params, allocator);
    unsigned int id = timelineProtocol.FromJson(json, error).get()->id;
    EXPECT_EQ(id, tempId);
}
TEST_F(ProtocolTest, ToKernelDetailRequest)
{
    const uint64_t tempId = 89;
    Dic::Protocol::TimelineProtocol timelineProtocol;
    timelineProtocol.Register();
    std::string error;
    Dic::document_t json(Dic::kObjectType);
    auto &allocator = json.GetAllocator();
    Dic::JsonUtil::AddMember(json, "type", "request", allocator);
    Dic::JsonUtil::AddMember(json, "command", "unit/kernelDetails", allocator);
    timelineProtocol.FromJson(json, error);

    Dic::json_t params(Dic::kObjectType);
    Dic::JsonUtil::AddMember(json, "id", tempId, allocator);
    Dic::JsonUtil::AddMember(json, "moduleName", "hhh", allocator);
    Dic::JsonUtil::AddMember(json, "params", params, allocator);
    unsigned int id = timelineProtocol.FromJson(json, error).get()->id;
    EXPECT_EQ(id, tempId);
}
TEST_F(ProtocolTest, ToOneKernelRequest)
{
    const uint64_t tempId = 89;
    Dic::Protocol::TimelineProtocol timelineProtocol;
    timelineProtocol.Register();
    std::string error;
    Dic::document_t json(Dic::kObjectType);
    auto &allocator = json.GetAllocator();
    Dic::JsonUtil::AddMember(json, "type", "request", allocator);
    Dic::JsonUtil::AddMember(json, "command", "unit/one/kernelDetail", allocator);
    timelineProtocol.FromJson(json, error);

    Dic::json_t params(Dic::kObjectType);
    Dic::JsonUtil::AddMember(json, "id", tempId, allocator);
    Dic::JsonUtil::AddMember(json, "moduleName", "hhh", allocator);
    Dic::JsonUtil::AddMember(json, "params", params, allocator);
    unsigned int id = timelineProtocol.FromJson(json, error).get()->id;
    EXPECT_EQ(id, tempId);
}
TEST_F(ProtocolTest, ToUnitThreadsOperatorsRequest)
{
    const uint64_t tempId = 89;
    Dic::Protocol::TimelineProtocol timelineProtocol;
    timelineProtocol.Register();
    std::string error;
    Dic::document_t json(Dic::kObjectType);
    auto &allocator = json.GetAllocator();
    Dic::JsonUtil::AddMember(json, "type", "request", allocator);
    Dic::JsonUtil::AddMember(json, "command", "query/all/same/operators/duration", allocator);
    timelineProtocol.FromJson(json, error);

    Dic::json_t params(Dic::kObjectType);
    Dic::JsonUtil::AddMember(json, "id", tempId, allocator);
    Dic::JsonUtil::AddMember(json, "moduleName", "hhh", allocator);
    Dic::JsonUtil::AddMember(json, "params", params, allocator);
    unsigned int id = timelineProtocol.FromJson(json, error).get()->id;
    EXPECT_EQ(id, tempId);
}

TEST_F(ProtocolTest, ResponseToJson)
{
    EXPECT_NO_THROW({
        Dic::Protocol::TimelineProtocol timelineProtocol;
        timelineProtocol.Register();
        std::string error;
        Dic::Protocol::ImportActionResponse response1;
        timelineProtocol.ToJson(response1, error);
        Dic::Protocol::UnitThreadTracesResponse response2;
        timelineProtocol.ToJson(response2, error);
        Dic::Protocol::UnitThreadTracesSummaryResponse response3;
        timelineProtocol.ToJson(response3, error);
        Dic::Protocol::UnitThreadsResponse response4;
        timelineProtocol.ToJson(response4, error);
        Dic::Protocol::UnitThreadDetailResponse response5;
        timelineProtocol.ToJson(response5, error);
        Dic::Protocol::ResetWindowResponse response8;
        timelineProtocol.ToJson(response8, error);
        Dic::Protocol::SearchCountResponse response9;
        timelineProtocol.ToJson(response9, error);
        Dic::Protocol::SearchSliceResponse response10;
        timelineProtocol.ToJson(response10, error);
        Dic::Protocol::RemoteDeleteResponse response11;
        timelineProtocol.ToJson(response11, error);
        Dic::Protocol::FlowCategoryListResponse response12;
        timelineProtocol.ToJson(response12, error);
        Dic::Protocol::FlowCategoryEventsResponse response13;
        timelineProtocol.ToJson(response13, error);
        Dic::Protocol::UnitCounterResponse response14;
        timelineProtocol.ToJson(response14, error);
        Dic::Protocol::SystemViewResponse response15;
        timelineProtocol.ToJson(response15, error);
        Dic::Protocol::KernelDetailsResponse response16;
        timelineProtocol.ToJson(response16, error);
        Dic::Protocol::OneKernelResponse response17;
        timelineProtocol.ToJson(response17, error);
        Dic::Protocol::UnitThreadsOperatorsResponse response18;
        timelineProtocol.ToJson(response18, error);
    });
}

TEST_F(ProtocolTest, EventToJson)
{
    EXPECT_NO_THROW({
        Dic::Protocol::TimelineProtocol timelineProtocol;
        timelineProtocol.Register();
        std::string error;
        Dic::Protocol::ParseSuccessEvent event;
        timelineProtocol.ToJson(event, error);
        Dic::Protocol::ParseFailEvent event2;
        timelineProtocol.ToJson(event2, error);
        Dic::Protocol::ParseClusterCompletedEvent event3;
        timelineProtocol.ToJson(event3, error);
        Dic::Protocol::ParseClusterStep2CompletedEvent event4;
        timelineProtocol.ToJson(event4, error);
        Dic::Protocol::ParseMemoryCompletedEvent event5;
        timelineProtocol.ToJson(event5, error);
        Dic::Protocol::ModuleResetEvent event6;
        timelineProtocol.ToJson(event6, error);
    });
}
