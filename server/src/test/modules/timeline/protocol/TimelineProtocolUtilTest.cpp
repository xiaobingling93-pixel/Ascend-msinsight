/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include "TimelineProtocolUtil.h"
#include "TimelineProtocolResponse.h"
#include "JsonUtil.h"
class TimelineProtocolUtilTest : public ::testing::Test {};

/**
 * 测试ThreadTracesResponseToJson正常情况
 */
TEST_F(TimelineProtocolUtilTest, TestThreadTracesResponseToJsonNormal)
{
    Dic::Protocol::UnitThreadTracesResponse response;
    const uint64_t curDep = 1;
    response.result = true;
    response.body.currentMaxDepth = curDep;
    response.body.havePythonFunction = true;
    const uint64_t maxDep = 2;
    response.body.maxDepth = maxDep;
    Dic::Protocol::ThreadTraces trace;
    trace.name = "kkk";
    const uint64_t end = 4444;
    trace.endTime = end;
    trace.id = "hhh";
    trace.threadId = "lll";
    const uint64_t dur = 777;
    trace.duration = dur;
    const uint64_t str = 444;
    trace.startTime = str;
    trace.pid = "ggg";
    trace.cname = "qqq";
    const uint64_t dep = 1;
    trace.depth = dep;
    std::vector<Dic::Protocol::ThreadTraces> traces;
    traces.emplace_back(trace);
    response.body.data.emplace_back(traces);
    auto jsonOp = Dic::Protocol::ToResponseJson(response);
    EXPECT_EQ(jsonOp.has_value(), true);
    const std::string json = Dic::JsonUtil::JsonDump(jsonOp.value());
    const std::string jsonStr = "{\"type\":\"response\",\"id\":0,\"requestId\":0,\"result\":true,\"command\":\"unit/"
        "threadTraces\",\"moduleName\":\"unknown\",\"body\":{\"maxDepth\":2,\"currentMaxDepth\":1,"
        "\"havePythonFunction\":true,\"data\":[[{\"name\":\"kkk\",\"duration\":777,\"startTime\":444,\"endTime\":4444,"
        "\"depth\":1,\"threadId\":\"lll\",\"cname\":\"qqq\",\"id\":\"hhh\"}]]}}";
    EXPECT_EQ(json, jsonStr);
}

/**
 * 测试ThreadTracesResponseToJson的error情况
 */
TEST_F(TimelineProtocolUtilTest, TestThreadTracesResponseToJsonError)
{
    Dic::Protocol::UnitThreadTracesResponse response;
    response.result = false;
    const uint64_t errorCode = 3;
    Dic::Protocol::ErrorMessage error = { errorCode, "ll" };
    response.error = error;
    auto jsonOp = Dic::Protocol::ToResponseJson(response);
    EXPECT_EQ(jsonOp.has_value(), true);
    const std::string json = Dic::JsonUtil::JsonDump(jsonOp.value());
    const std::string jsonStr = "{\"type\":\"response\",\"id\":0,\"requestId\":0,\"result\":false,\"command\":\"unit/"
        "threadTraces\",\"moduleName\":\"unknown\",\"message\":\"ll\",\"error\":{\"code\":3},\"body\":{\"maxDepth\":0,"
        "\"currentMaxDepth\":0,\"havePythonFunction\":false,\"data\":[]}}";
    EXPECT_EQ(json, jsonStr);
}

TEST_F(TimelineProtocolUtilTest, TestUnitThreadsResponseToJsonError)
{
    Dic::Protocol::UnitThreadsResponse response;
    Dic::Protocol::Threads threads;
    response.body.data.emplace_back(threads);
    auto jsonOp = Dic::Protocol::ToResponseJson(response);
    EXPECT_EQ(jsonOp.has_value(), true);
    const std::string json = Dic::JsonUtil::JsonDump(jsonOp.value());
    const std::string jsonStr = "{\"type\":\"response\",\"id\":0,\"requestId\":0,\"result\":false,\"command\":\"unit/"
        "threads\",\"moduleName\":\"unknown\",\"body\":{\"emptyFlag\":false,\"data\":[{\"title\":\"\",\"wallDuration\":"
        "0,\"occurrences\":0,\"avgWallDuration\":0,\"selfTime\":0,\"tid\":\"\",\"pid\":\"\",\"metaType\":\"\"}]}}";
    EXPECT_EQ(json, jsonStr);
}

TEST_F(TimelineProtocolUtilTest, TestUnitFlowsResponseToJson)
{
    Dic::Protocol::UnitFlowsResponse response;
    Dic::Protocol::UnitSingleFlow unitSingleFlow;
    Dic::Protocol::UnitCatFlows unitCatFlows;
    unitCatFlows.flows.emplace_back(unitSingleFlow);
    response.body.unitAllFlows.emplace_back(unitCatFlows);
    auto jsonOp = Dic::Protocol::ToResponseJson(response);
    EXPECT_EQ(jsonOp.has_value(), true);
    const std::string json = Dic::JsonUtil::JsonDump(jsonOp.value());
    const std::string jsonStr = "{\"type\":\"response\",\"id\":0,\"requestId\":0,\"result\":false,\"command\":\"unit/"
        "flows\",\"moduleName\":\"unknown\",\"body\":{\"unitAllFlows\":[{\"cat\":\"\",\"flows\":[{\"title\":\"\","
        "\"cat\":\"\",\"id\":\"\",\"from\":{\"pid\":\"\",\"tid\":\"\",\"timestamp\":0,\"duration\":0,\"depth\":0,"
        "\"name\":\"\",\"id\":\"\",\"metaType\":\"\",\"rankId\":\"\"},\"to\":{\"pid\":\"\",\"tid\":\"\",\"timestamp\":"
        "0,\"duration\":0,\"depth\":0,\"name\":\"\",\"id\":\"\",\"metaType\":\"\",\"rankId\":\"\"}}]}]}}";
    EXPECT_EQ(json, jsonStr);
}

TEST_F(TimelineProtocolUtilTest, TestFlowCategoryEventsResponseToJson)
{
    Dic::Protocol::FlowCategoryEventsResponse response;
    std::unique_ptr<Dic::Protocol::UnitSingleFlow> unitSingleFlow = std::make_unique<Dic::Protocol::UnitSingleFlow>();
    response.body.flowDetailList.emplace_back(std::move(unitSingleFlow));
    auto jsonOp = Dic::Protocol::ToResponseJson(response);
    EXPECT_EQ(jsonOp.has_value(), true);
    const std::string json = Dic::JsonUtil::JsonDump(jsonOp.value());
    const std::string jsonStr = "{\"type\":\"response\",\"id\":0,\"requestId\":0,\"result\":false,\"command\":\"flow/"
        "categoryEvents\",\"moduleName\":\"unknown\",\"body\":{\"flowDetailList\":[{\"category\":\"\",\"from\":{"
        "\"pid\":\"\",\"tid\":\"\",\"timestamp\":0,\"depth\":0,\"rankId\":\"\"},\"to\":{\"pid\":\"\",\"tid\":\"\","
        "\"timestamp\":0,\"depth\":0,\"rankId\":\"\"}}]}}";
    EXPECT_EQ(json, jsonStr);
}

TEST_F(TimelineProtocolUtilTest, TestEventsViewResponseToJson)
{
    Dic::Protocol::EventsViewResponse response;
    Dic::Protocol::EventsViewColumnAttr eventsViewColumnAttr;
    std::unique_ptr<Dic::Protocol::EventDetail> host = std::make_unique<Dic::Protocol::HostEventDetail>();
    std::unique_ptr<Dic::Protocol::EventDetail> device = std::make_unique<Dic::Protocol::DeviceEventDetail>();
    class Other : public Dic::Protocol::EventDetail {};
    std::unique_ptr<Dic::Protocol::EventDetail> other = std::make_unique<Other>();
    response.body.eventDetailList.emplace_back(std::move(host));
    response.body.eventDetailList.emplace_back(std::move(device));
    response.body.eventDetailList.emplace_back(std::move(other));
    response.body.columnList.emplace_back(eventsViewColumnAttr);
    auto jsonOp = Dic::Protocol::ToResponseJson(response);
    EXPECT_EQ(jsonOp.has_value(), true);
    const std::string json = Dic::JsonUtil::JsonDump(jsonOp.value());
    const std::string jsonStr = "{\"type\":\"response\",\"id\":0,\"requestId\":0,\"result\":false,\"command\":\"unit/"
        "eventView\",\"moduleName\":\"unknown\",\"body\":{\"eventDetails\":[{\"id\":\"\",\"name\":\"\",\"start\":0,"
        "\"duration\":0,\"depth\":0,\"threadId\":\"\",\"processId\":\"\",\"tid\":\"\",\"pid\":\"\"},{\"id\":\"\","
        "\"name\":\"\",\"start\":0,\"duration\":0,\"depth\":0,\"threadId\":\"\",\"processId\":\"\",\"threadName\":\"\","
        "\"rankId\":\"\"}],\"columnList\":[{\"name\":\"\",\"type\":\"\",\"key\":\"\"}],\"count\":0,\"pageSize\":0,"
        "\"currentPage\":0}}";
    EXPECT_EQ(json, jsonStr);
}

TEST_F(TimelineProtocolUtilTest, TestCommunicationKernelResponseToJson)
{
    Dic::Protocol::CommunicationKernelResponse response;
    auto jsonOp = Dic::Protocol::ToResponseJson(response);
    EXPECT_EQ(jsonOp.has_value(), true);
    const std::string json = Dic::JsonUtil::JsonDump(jsonOp.value());
}

TEST_F(TimelineProtocolUtilTest, TestUnitCounterResponseToJson)
{
    Dic::Protocol::UnitCounterResponse response;
    Dic::Protocol::UnitCounterData unitCounterData;
    unitCounterData.valueJsonStr = "{\"name\":\"\",\"type\":\"\",\"key\":\"\"}";
    Dic::Protocol::UnitCounterData unitCounterData2;
    unitCounterData2.valueJsonStr = "lll";
    response.body.data.emplace_back(unitCounterData);
    auto jsonOp = Dic::Protocol::ToResponseJson(response);
    EXPECT_EQ(jsonOp.has_value(), true);
    const std::string json = Dic::JsonUtil::JsonDump(jsonOp.value());
    const std::string jsonStr = "{\"type\":\"response\",\"id\":0,\"requestId\":0,\"result\":false,\"command\":\"unit/"
        "counter\",\"moduleName\":\"unknown\",\"body\":{\"data\":[{\"timestamp\":0,\"value\":{"
        "\"name\":\"\",\"type\":\"\",\"key\":\"\"}}]}}";
    EXPECT_EQ(json, jsonStr);
}

TEST_F(TimelineProtocolUtilTest, TestUnitThreadsOperatorsResponseToJson)
{
    Dic::Protocol::UnitThreadsOperatorsResponse response;
    Dic::Protocol::SameOperatorsDetails sameOperatorsDetails;
    response.body.sameOperatorsDetails.emplace_back(sameOperatorsDetails);
    auto jsonOp = Dic::Protocol::ToResponseJson(response);
    EXPECT_EQ(jsonOp.has_value(), true);
}

TEST_F(TimelineProtocolUtilTest, TestSearchAllSlicesResponseToJson)
{
    Dic::Protocol::SearchAllSlicesResponse response;
    Dic::Protocol::SearchAllSlices searchAllSlices;
    response.body.searchAllSlices.emplace_back(searchAllSlices);
    auto jsonOp = Dic::Protocol::ToResponseJson(response);
    EXPECT_EQ(jsonOp.has_value(), true);
}

TEST_F(TimelineProtocolUtilTest, TestAllSuccessEventToJson)
{
    Dic::Protocol::AllSuccessEvent response;
    Dic::Protocol::CardOffset cardOffset;
    response.body.cardOffsets.emplace_back(cardOffset);
    auto jsonOp = Dic::Protocol::ToEventJson(response);
    EXPECT_EQ(jsonOp.has_value(), true);
    const std::string json = Dic::JsonUtil::JsonDump(jsonOp.value());
    const std::string jsonStr =
        "{\"type\":\"event\",\"id\":0,\"event\":\"allPagesSuccess\",\"moduleName\":\"unknown\",\"body\":{"
        "\"isAllPageParsed\":false,\"cardOffsets\":[{\"cardId\":\"\",\"offset\":0}],\"minTime\":0}}";
    EXPECT_EQ(json, jsonStr);
}

TEST_F(TimelineProtocolUtilTest, TestParseMemoryCompletedEventToJson)
{
    Dic::Protocol::ParseMemoryCompletedEvent response;
    Dic::Protocol::MemorySuccess memorySuccess;
    response.memoryResult.emplace_back(memorySuccess);
    auto jsonOp = Dic::Protocol::ToEventJson(response);
    EXPECT_EQ(jsonOp.has_value(), true);
    const std::string json = Dic::JsonUtil::JsonDump(jsonOp.value());
    const std::string jsonStr = "{\"type\":\"event\",\"id\":0,\"event\":\"parse/"
                                "memoryCompleted\",\"moduleName\":\"unknown\",\"body\":{\"isCluster\":false,"
                                "\"memoryResult\":[{\"rankId\":\"\",\"hasMemory\":false}]}}";
    EXPECT_EQ(json, jsonStr);
}

TEST_F(TimelineProtocolUtilTest, TestParseProgressEventToJson)
{
    Dic::Protocol::ParseProgressEvent response;
    auto jsonOp = Dic::Protocol::ToEventJson(response);
    EXPECT_EQ(jsonOp.has_value(), true);
}
