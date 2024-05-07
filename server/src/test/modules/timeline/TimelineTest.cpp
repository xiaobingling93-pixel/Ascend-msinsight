/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include <gtest/gtest.h>
#include "TimelineProtocolRequest.h"
#include "DataBaseManager.h"
#include "../../TestSuit.cpp"
#include "cmath"

class TimelineTest : TestSuit {
};

TEST_F(TestSuit, QuerySystemViewData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::SystemViewParams requestParams;
    Dic::Protocol::SystemViewBody responseBody;
    uint64_t PAGE = 10;
    requestParams.layer = "Python";
    requestParams.current = 1;
    requestParams.order = "descend";
    requestParams.orderBy = "name";
    requestParams.pageSize = PAGE;
    database->QuerySystemViewData(requestParams, responseBody);
    int expectSize = 10;
    EXPECT_EQ(responseBody.systemViewDetail.size(), expectSize);
}

TEST_F(TestSuit, QueryPythonViewWithTotalNum)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::SystemViewParams requestParams;
    Dic::Protocol::SystemViewBody responseBody;
    uint64_t PAGE = 10;
    requestParams.layer = "Python";
    requestParams.current = 1;
    requestParams.order = "descend";
    requestParams.orderBy = "name";
    requestParams.pageSize = PAGE;
    requestParams.isQueryTotal = true;
    database->QuerySystemViewData(requestParams, responseBody);
    int expectSize = 162;
    EXPECT_EQ(responseBody.total, expectSize);
}

TEST_F(TestSuit, QueryLayerOperatorTimeWithCann)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");

    const Dic::Module::Timeline::LayerStatData &data = database->QueryLayerData("CANN", "%%");
    int expectSize = 158266100;
    EXPECT_EQ(lround(data.allOperatorTime), expectSize);
}

TEST_F(TestSuit, QueryLayerOperatorTimeWithPython)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");

    const Dic::Module::Timeline::LayerStatData &data = database->QueryLayerData("Python", "%%");
    int expectSize = 851869940;
    EXPECT_EQ(lround(data.allOperatorTime), expectSize);
}

TEST_F(TestSuit, QueryLayerOperatorTimeWithAscend)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");

    const Dic::Module::Timeline::LayerStatData &data = database->QueryLayerData("Ascend Hardware", "%%");
#ifdef WIN32
    int expectSize = 842011262;
#else
    int expectSize = 842011261;
#endif
    EXPECT_EQ(lround(data.allOperatorTime), expectSize);
    EXPECT_EQ(data.total, 57); // total operator = 57
}

TEST_F(TestSuit, QueryLayerOperatorTimeWithHCCL)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");

    const Dic::Module::Timeline::LayerStatData &data = database->QueryLayerData("HCCL", "%%");
#ifdef WIN32
    int expectSize = 449202040;
#else
    int expectSize = 449202038;
#endif
    EXPECT_EQ(lround(data.allOperatorTime), expectSize);
}

TEST_F(TestSuit, QueryLayerOperatorTimeWithOverlap)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");

    const Dic::Module::Timeline::LayerStatData &data = database->QueryLayerData("Overlap Analysis", "%%");
#ifdef WIN32
    int expectSize = 445796394;
#else
    int expectSize = 445796392;
#endif
    EXPECT_EQ(lround(data.allOperatorTime), expectSize);
}

TEST_F(TestSuit, QueryCoreType)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");

    const std::vector<std::string> &coreType = database->QueryCoreType();
    int expectSize = 7;
    EXPECT_EQ(coreType.size(), expectSize);
}

TEST_F(TestSuit, QueryKernelDetailData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::KernelDetailsParams requestParams;
    Dic::Protocol::KernelDetailsBody responseBody;
    uint64_t PAGE = 10;
    requestParams.current = 1;
    requestParams.order = "descend";
    requestParams.orderBy = "name";
    requestParams.pageSize = PAGE;
    requestParams.rankId = "0";
    database->QueryKernelDetailData(requestParams, responseBody, 0);
    int expectSize = 21;
    EXPECT_EQ(responseBody.count, expectSize);
}

TEST_F(TestSuit, QueryKernelDetailDataWithCoreType)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::KernelDetailsParams requestParams;
    Dic::Protocol::KernelDetailsBody responseBody;
    uint64_t PAGE = 10;
    requestParams.current = 1;
    requestParams.order = "descend";
    requestParams.orderBy = "name";
    requestParams.pageSize = PAGE;
    requestParams.rankId = "0";
    requestParams.coreType = "AI_CORE";
    requestParams.searchName = "";
    database->QueryKernelDetailData(requestParams, responseBody, 0);
    int expectSize = 0;
    EXPECT_EQ(responseBody.count, expectSize);
}

TEST_F(TestSuit, QueryKernelDepthAndThread)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::KernelParams requestParams;
    Dic::Protocol::OneKernelBody responseBody;
    uint64_t DURATION = 72861;
    uint64_t TIMESTAMP = 1695115378736217000;
    requestParams.duration = DURATION;
    requestParams.name = "trans_Cast_15";
    requestParams.timestamp = 0;
    uint64_t minTime = TIMESTAMP;
    database->QueryKernelDepthAndThread(requestParams, responseBody, minTime);
    uint64_t depth = 0;
    std::string tid = "17";
    std::string pid = "300";
    EXPECT_EQ(responseBody.depth, depth);
    EXPECT_EQ(responseBody.threadId, tid);
    EXPECT_EQ(responseBody.pid, pid);
}

TEST_F(TestSuit, QueryCommunicationStatisticsData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::SummaryStatisticParams requestParams;
    Dic::Protocol::SummaryStatisticsBody responseBody;
    database->QueryCommunicationStatisticsData(requestParams, responseBody);
    EXPECT_EQ(responseBody.summaryStatisticsItemList.size(), 2); // item number = 2
}

TEST_F(TestSuit, QueryUnitCounterData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::UnitCounterParams requestParams;
    std::vector<Dic::Protocol::UnitCounterData> unitData;
    uint64_t STARTTIME = 1695115378653323500;
    uint64_t ENDTIME = 1695115378673307500;
    requestParams.pid = "14083661300";
    requestParams.threadName = "APP/DDR";
    requestParams.startTime = STARTTIME;
    requestParams.endTime = ENDTIME;
    database->QueryUnitCounter(requestParams, 0, unitData);
    EXPECT_EQ(unitData.size(), 2); // unit data size = 2
    EXPECT_EQ(unitData[0].timestamp, 1695115378653323500); // timestamp = 1695115378653323500
}

TEST_F(TestSuit, QueryFlowCategoryEvents)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::FlowCategoryEventsParams requestParams;
    std::vector<std::unique_ptr<Dic::Protocol::FlowEvent>> flowDetailList;
    uint64_t STARTTIME = 1695115378713661000;
    uint64_t ENDTIME = 1695115378725106800;
    requestParams.startTime = STARTTIME;
    requestParams.endTime = ENDTIME;
    requestParams.timePerPx = 1;
    requestParams.category = "HostToDevice";
    database->QueryFlowCategoryEvents(requestParams, 0, flowDetailList);
    std::string EXPECT_FROM_TID = "1408366";
    EXPECT_EQ(flowDetailList.size(), 5); // flowDetailList size = 5
    EXPECT_EQ(flowDetailList[0]->category, "HostToDevice");
    EXPECT_EQ(flowDetailList[0]->from.timestamp, 1695115378722143800); // timestamp = 1695115378722143800
    EXPECT_EQ(flowDetailList[0]->from.pid, "140836602");
    EXPECT_EQ(flowDetailList[0]->from.depth, 0);
    EXPECT_EQ(flowDetailList[0]->from.tid, EXPECT_FROM_TID); // from.tid = 1408366
    EXPECT_EQ(flowDetailList[0]->to.timestamp, 1695115378722392500); // to.timestamp = 1695115378722392500
    EXPECT_EQ(flowDetailList[0]->to.pid, "14083661400");
    EXPECT_EQ(flowDetailList[0]->to.depth, 0);
    EXPECT_EQ(flowDetailList[0]->to.tid, "0"); // to.tid = 0
}

TEST_F(TestSuit, QueryFlowCategoryList)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    std::vector<std::string> categories;
    database->QueryFlowCategoryList(categories);
    const int thirdNumber = 2;
    EXPECT_EQ(categories.size(), 3); // categories.size = 3
    EXPECT_EQ(categories[0], "HostToDevice");
    EXPECT_EQ(categories[1], "async_npu");
    EXPECT_EQ(categories[thirdNumber], "async_task_queue");
}

TEST_F(TestSuit, QueryThreadTraces)
{
    Dic::Protocol::UnitThreadTracesParams request;
    uint64_t STARTTIME = 1695115378713851200;
    uint64_t ENDTIME = 1695115378728583500;
    request.startTime = STARTTIME;
    request.endTime = ENDTIME;
    request.threadId = "0";
    request.timePerPx = 1;
    Dic::Protocol::UnitThreadTracesBody response;
    uint64_t minTimestamp = 0;
    int64_t traceId = 30;

    int expectSize = 27;
    std::string expectName = "AscendCL@aclDestroyTensorDesc";
    uint64_t expectDuration = 1900;
    uint64_t expectStartTime = 1695115378713851200;
    uint64_t expectEndTime = expectStartTime + expectDuration;
    int32_t expectDepth = 0;
    std::string expectThreadId = request.threadId;

    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    database->QueryThreadTraces(request, response, minTimestamp, traceId);

    EXPECT_EQ(response.data[0].size(), expectSize);
}

TEST_F(TestSuit, QueryThreads)
{
    // request parameters
    Dic::Protocol::UnitThreadsParams request;
    uint64_t STARTTIME = 1695115378734066176;
    uint64_t ENDTIME = 1695115378743155968;
    request.startTime = STARTTIME;
    request.endTime = ENDTIME;
    Dic::Protocol::UnitThreadsBody response;
    uint64_t minTimestamp = 0;
    int64_t traceId = 30;

    // expected data
    int size = 2;
    std::string title = "AscendCL@aclDestroyTensorDesc";
    uint64_t wallDuration = 42180;
    uint64_t occurrences = 62;
    uint64_t avgWallDuration = 680;
    uint64_t selfTime = 42180;

    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    database->QueryThreads(request, response, minTimestamp, traceId);

    EXPECT_EQ(response.emptyFlag, false);
    EXPECT_EQ(response.data.size(), size);
    EXPECT_EQ(response.data[0].title, title);
    EXPECT_EQ(response.data[0].wallDuration, wallDuration);
    EXPECT_EQ(response.data[0].occurrences, occurrences);
    EXPECT_EQ(response.data[0].avgWallDuration, avgWallDuration);
    EXPECT_EQ(response.data[0].selfTime, selfTime);
}

TEST_F(TestSuit, QueryThreadDetail)
{
    // request parameters
    uint64_t STARTTIME = 1695115378728277500;
    uint32_t DEPTH = 7;
    Dic::Protocol::ThreadDetailParams request;
    uint64_t id = 18906;
    request.id = std::to_string(id);
    request.depth = DEPTH;
    request.startTime = STARTTIME;
    uint64_t minTimestamp = 0;
    int64_t traceId = 41;

    // expected data
    uint64_t selfTime = 0;
    uint64_t duration = 17250;
    std::string args = "{\"correlation_id\":\"2684\"}";
    std::string title = "Enqueue";
    std::string cat = "enqueue";

    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::UnitThreadDetailBody response;
    database->QueryThreadDetail(request, response, minTimestamp, traceId);

    EXPECT_EQ(response.emptyFlag, false);
    EXPECT_EQ(response.data.selfTime, selfTime);
    EXPECT_EQ(response.data.duration, duration);
    EXPECT_EQ(response.data.args, args);
    EXPECT_EQ(response.data.title, title);
    EXPECT_EQ(response.data.cat, cat);
}

TEST_F(TestSuit, QueryKernelShapes)
{
    // request parameters
    std::vector<SliceDto> sliceDtoVec;
    SliceDto sliceDto;
    uint64_t STARTTIME = 1695115378713661000;
    sliceDto.name = "ZerosLike";
    sliceDto.timestamp = STARTTIME;
    sliceDtoVec.emplace_back(sliceDto);

    auto database = std::dynamic_pointer_cast<Dic::Module::Timeline::JsonTraceDatabase,
    Dic::Module::Timeline::VirtualTraceDatabase>(Dic::Module::Timeline::DataBaseManager::Instance().
    GetTraceDatabase("0"));
    const KernelShapesDataDto &dto = database->QueryKernelShapes(sliceDtoVec);
    EXPECT_EQ(dto.inputShapes, "\"232138240\"");
    EXPECT_EQ(dto.inputFormats, "FORMAT_ND");
    EXPECT_EQ(dto.inputDataTypes, "FLOAT");
    EXPECT_EQ(dto.outputShapes, "\"232138240\"");
    EXPECT_EQ(dto.outputDataTypes, "FLOAT");
    EXPECT_EQ(dto.outputFormats, "FORMAT_ND");
}

void assertFlowLocationEqual(Dic::Protocol::FlowLocation &location, Dic::Protocol::FlowLocation &other)
{
    EXPECT_EQ(location.duration, other.duration);
    EXPECT_EQ(location.depth, other.depth);
    EXPECT_EQ(location.tid, other.tid);
    EXPECT_EQ(location.pid, other.pid);
    EXPECT_EQ(location.name, other.name);
    EXPECT_EQ(location.timestamp, other.timestamp);
}

TEST_F(TestSuit, QueryFlowDetail)
{
    // request parameters
    Dic::Protocol::UnitFlowParams request;
    request.flowId = "85899345919";
    request.startTime = 0;
    request.type = "s";
    uint64_t minTimestamp = 0;

    // expected data
    uint64_t TOTIMESTAMP = 1695115378715400200;
    uint64_t FROMTIMESTAMP = 1695115378714991500;
    uint64_t TODURATION = 4975266;
    uint64_t FROMDURATION = 468180;
    std::string FROMTID = "1408366";

    std::string title = "HostToDevice85899345919";
    std::string cat = "HostToDevice";
    std::string id = "85899345919";
    Dic::Protocol::FlowLocation from;
    from.duration = FROMDURATION;
    from.tid = FROMTID;
    from.pid = "140836602";
    from.name = "AscendCL@hcom_broadcast_";
    from.timestamp = FROMTIMESTAMP;
    Dic::Protocol::FlowLocation to;
    to.duration = TODURATION;
    to.tid = "0";
    to.pid = "14083661400";
    to.name = "hcom_broadcast__483_0";
    to.timestamp = TOTIMESTAMP;

    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::UnitFlowBody response;
    database->QueryFlowDetail(request, response, minTimestamp);

    EXPECT_EQ(response.title, title);
    EXPECT_EQ(response.cat, cat);
    EXPECT_EQ(response.id, id);
    assertFlowLocationEqual(response.from, from);
    assertFlowLocationEqual(response.to, to);
}

TEST_F(TestSuit, QueryExtremumTimestamp)
{
    uint64_t expectMin = 1695115378653323500;
    uint64_t expectMax = 1695115379212601200;
    bool expectResult = true;
    uint64_t min;
    uint64_t max;

    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    bool result = database->QueryExtremumTimestamp(min, max);
    EXPECT_EQ(result, expectResult);
    EXPECT_EQ(min, expectMin);
    EXPECT_EQ(max, expectMax);
}

TEST_F(TestSuit, SearchSliceNameCountWithFuzzyMatch)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    int expectCount = 91;
    SearchCountParams params;
    params.searchContent = "Mul";

    int count = database->SearchSliceNameCount(params);
    EXPECT_EQ(count, expectCount);
}

TEST_F(TestSuit, SearchSliceNameCountWithExactMatch)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    int expectCount = 4;
    SearchCountParams params;
    params.isMatchExact = true;
    params.searchContent = "Mul";

    int count = database->SearchSliceNameCount(params);
    EXPECT_EQ(count, expectCount);
}

TEST_F(TestSuit, SearchSliceNameCountWithCaseMatch)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    int expectCount = 55;
    SearchCountParams params;
    params.isMatchCase = true;
    params.searchContent = "Mul";

    int count = database->SearchSliceNameCount(params);
    EXPECT_EQ(count, expectCount);
}

TEST_F(TestSuit, SearchSliceNameCountWithCaseAndExactMatch)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    int expectCount = 4;
    SearchCountParams params;
    params.isMatchExact = true;
    params.isMatchCase = true;
    params.searchContent = "Mul";

    int count = database->SearchSliceNameCount(params);
    EXPECT_EQ(count, expectCount);
}

TEST_F(TestSuit, SearchSliceName)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::SearchSliceParams params;
    params.searchContent = "Enqueue";
    int index = 0;
    uint64_t minTimestamp = 0;
    Dic::Protocol::SearchSliceBody body;

    std::string expectPid = "1408366";
    std::string expectTid = "1408366";
    uint64_t expectStartTime = 1695115378713520800;
    int32_t expectDepth = 3;
    uint64_t expectDuration = 18250;

    database->SearchSliceName(params, index, minTimestamp, body);
    EXPECT_EQ(body.pid, expectPid);
    EXPECT_EQ(body.tid, expectTid);
    EXPECT_EQ(body.startTime, expectStartTime);
    EXPECT_EQ(body.depth, expectDepth);
    EXPECT_EQ(body.duration, expectDuration);
}

TEST_F(TestSuit, QueryFlowName)
{
    // request parameters
    Dic::Protocol::UnitFlowNameParams request;
    uint64_t START_TIME = 1695115378713661000;
    request.startTime = START_TIME;
    uint64_t MIN_TIMESTAMP = 0;
    int64_t trackId = 7;

    // expected data
    int size = 0;
    Dic::Protocol::FlowName name;
    name.title = "torch_to_npu";
    name.flowId = "1695115378713661.0";
    name.type = "f";
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::UnitFlowNameBody response;
    database->QueryFlowName(request, response, MIN_TIMESTAMP, trackId);
    EXPECT_EQ(response.flowDetail.size(), size);
}

TEST_F(TestSuit, QueryThreadSameOperatorsDetails)
{
    // request parameters
    Protocol::UnitThreadsOperatorsParams requestParam;
    uint64_t START_TIME = 1695115378713505000;
    uint64_t END_TIME = 1695115378714916500;
    uint64_t PAGE_SIZE = 10;
    requestParam.name = "aten::empty";
    requestParam.startTime = START_TIME;
    requestParam.endTime = END_TIME;
    requestParam.orderBy = "timestamp";
    requestParam.current = 1;
    requestParam.pageSize = PAGE_SIZE;
    Protocol::UnitThreadsOperatorsBody responseBody;
    uint64_t minTimestamp = 0;
    int64_t traceId = 41;

    // response data
    uint64_t TIMESTAMP1 = 1695115378714082800;
    uint64_t DURATION1 = 27310;
    uint64_t TIMESTAMP2 = 1695115378714385200;
    uint64_t DURATION2 = 35710;
    uint64_t COUNT = 2;

    auto database = std::dynamic_pointer_cast<Dic::Module::Timeline::JsonTraceDatabase,
            Dic::Module::Timeline::VirtualTraceDatabase>(Dic::Module::Timeline::DataBaseManager::Instance().
            GetTraceDatabase("0"));
    database->QueryThreadSameOperatorsDetails(requestParam, responseBody, minTimestamp, traceId);
    EXPECT_EQ(responseBody.sameOperatorsDetails[0].timestamp, TIMESTAMP1);
    EXPECT_EQ(responseBody.sameOperatorsDetails[0].duration, DURATION1);
    EXPECT_EQ(responseBody.sameOperatorsDetails[1].timestamp, TIMESTAMP2);
    EXPECT_EQ(responseBody.sameOperatorsDetails[1].duration, DURATION2);
    EXPECT_EQ(responseBody.count, COUNT);
}