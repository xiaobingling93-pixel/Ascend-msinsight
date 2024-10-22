/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include <gtest/gtest.h>
#include "TimelineProtocolRequest.h"
#include "DataBaseManager.h"
#include "../../TestSuit.cpp"
#include "cmath"
#include "TimelineTestUtil.h"

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
    int expectSize = 292195067;
#else
    int expectSize = 292195067;
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

TEST_F(TestSuit, QueryFlowCategoryList)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    std::vector<std::string> categories;
    database->QueryFlowCategoryList(categories, "");
    const int thirdNumber = 2;
    EXPECT_EQ(categories.size(), 3); // categories.size = 3
    EXPECT_EQ(categories[0], "HostToDevice");
    EXPECT_EQ(categories[1], "async_npu");
    EXPECT_EQ(categories[thirdNumber], "async_task_queue");
}

TEST_F(TestSuit, QueryThreads)
{
    // request parameters
    Dic::Protocol::UnitThreadsParams request;
    uint64_t STARTTIME = 1695115378734066176;
    uint64_t ENDTIME = 1695115378743155968;
    request.startTime = STARTTIME;
    request.endTime = ENDTIME;
    request.rankId = "0";
    Dic::Protocol::UnitThreadsBody response;
    uint64_t minTimestamp = 0;

    // expected data
    int size = 2;
    std::string title = "AscendCL@aclDestroyTensorDesc";
    uint64_t wallDuration = 42180;
    uint64_t occurrences = 62;
    uint64_t avgWallDuration = 680;
    uint64_t selfTime = 42180;
    Dic::Protocol::Metadata metadata;
    metadata.tid = "1413063";
    metadata.pid = "140836602";
    metadata.metaType.clear();
    request.metadataList.emplace_back(metadata);
    request.rankId = "0";
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    database->QueryThreads(request, response, minTimestamp, {30});

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

    auto database = std::dynamic_pointer_cast<Dic::Module::Timeline::TextTraceDatabase,
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
    params.rankId = "0";
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

    auto database = std::dynamic_pointer_cast<Dic::Module::Timeline::TextTraceDatabase,
            Dic::Module::Timeline::VirtualTraceDatabase>(Dic::Module::Timeline::DataBaseManager::Instance().
            GetTraceDatabase("0"));
    database->QueryThreadSameOperatorsDetails(requestParam, responseBody, minTimestamp, traceId);
    EXPECT_EQ(responseBody.sameOperatorsDetails[0].timestamp, TIMESTAMP1);
    EXPECT_EQ(responseBody.sameOperatorsDetails[0].duration, DURATION1);
    EXPECT_EQ(responseBody.sameOperatorsDetails[1].timestamp, TIMESTAMP2);
    EXPECT_EQ(responseBody.sameOperatorsDetails[1].duration, DURATION2);
    EXPECT_EQ(responseBody.count, COUNT);
}

TEST_F(TestSuit, SearchAllSlicesDetailsWithFuzzyMatch)
{
    uint64_t START_TIME = 1695115378726082200;
    uint64_t DURATION = 3439;
    uint64_t PAGE_SIZE = 10;
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    int expectCount = 91;
    SearchAllSliceParams params;
    params.searchContent = "Mul";
    params.pageSize = PAGE_SIZE;
    params.current = 1;
    params.orderBy = "duration";

    SearchAllSlicesBody body;

    database->SearchAllSlicesDetails(params, body, 0);
    EXPECT_EQ(body.searchAllSlices[0].timestamp, START_TIME);
    EXPECT_EQ(body.searchAllSlices[0].duration, DURATION);
    EXPECT_EQ(body.searchAllSlices[0].name, "Mul");
    EXPECT_EQ(body.count, expectCount);
}

TEST_F(TestSuit, QueryEventsViewData4Python)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    EventsViewParams params;
    params.processName = "Python (1408366)";
    params.pageSize = PAGE_SIZE;
    params.currentPage = CUR_PAGE_2;
    params.pid = "1408366";
    params.rankId = "0";

    EventsViewBody body;
    database->QueryEventsViewData(params, body, 0);
    const uint64_t EXPECT_COUNT = 6614;
    const uint64_t EXPECT_START = 1695115378714485500;
    const uint64_t EXPECT_DURATION = 262400;

    EXPECT_EQ(body.count, EXPECT_COUNT);
    CheckEventsViewColumns4Api(body);
    EXPECT_EQ(body.eventDetailList.size(), PAGE_SIZE);

    auto ptr = dynamic_cast<HostEventDetail*>(body.eventDetailList.at(0).get());
    EXPECT_TRUE(ptr != nullptr);
    EXPECT_EQ(ptr->name, "aten::_to_copy");
    EXPECT_EQ(ptr->startTime, EXPECT_START);
    EXPECT_EQ(ptr->duration, EXPECT_DURATION);
    EXPECT_EQ(ptr->tid, "1408366");
    EXPECT_EQ(ptr->pid, "1408366");
    EXPECT_EQ(ptr->processId, "1408366");
    EXPECT_EQ(ptr->threadId, "1408366");
}

TEST_F(TestSuit, QueryEventsViewData4PythonThread)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    EventsViewParams params;
    params.pid = "1408366";
    params.processName = "Python (1408366)";
    params.tid = "1408366";
    params.threadName = "Thread 1408366";
    params.pageSize = PAGE_SIZE;
    params.currentPage = CUR_PAGE_2;
    params.rankId = "0";

    EventsViewBody body;
    database->QueryEventsViewData(params, body, 0);
    const uint64_t EXPECT_COUNT = 5644;
    const uint64_t EXPECT_START = 1695115378714532200;
    const uint64_t EXPECT_DURATION = 96310;

    EXPECT_EQ(body.count, EXPECT_COUNT);
    CheckEventsViewColumns4Api(body);
    EXPECT_EQ(body.eventDetailList.size(), PAGE_SIZE);
    auto ptr = dynamic_cast<HostEventDetail*>(body.eventDetailList.at(0).get());
    EXPECT_TRUE(ptr != nullptr);
    EXPECT_EQ(ptr->name, "aten::empty_strided");
    EXPECT_EQ(ptr->startTime, EXPECT_START);
    EXPECT_EQ(ptr->duration, EXPECT_DURATION);
    EXPECT_EQ(ptr->tid, "1408366");
    EXPECT_EQ(ptr->pid, "1408366");
    EXPECT_EQ(ptr->processId, "1408366");
    EXPECT_EQ(ptr->threadId, "1408366");
}

TEST_F(TestSuit, QueryEventsViewData4CANN)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    EventsViewParams params;
    params.pid = "140836602";
    params.processName = "CANN (140836602)";
    params.pageSize = PAGE_SIZE;
    params.currentPage = CUR_PAGE_2;
    params.rankId = "0";

    EventsViewBody body;
    database->QueryEventsViewData(params, body, 0);
    const uint64_t EXPECT_COUNT = 9523;
    const uint64_t EXPECT_START = 1695115378714600200;
    const uint64_t EXPECT_DURATION = 27010;

    EXPECT_EQ(body.count, EXPECT_COUNT);
    CheckEventsViewColumns4Api(body);
    EXPECT_EQ(body.eventDetailList.size(), PAGE_SIZE);
    auto ptr = dynamic_cast<HostEventDetail*>(body.eventDetailList.at(0).get());
    EXPECT_TRUE(ptr != nullptr);
    EXPECT_EQ(ptr->name, "AscendCL@aclrtDestroyEvent");
    EXPECT_EQ(ptr->startTime, EXPECT_START);
    EXPECT_EQ(ptr->duration, EXPECT_DURATION);
    EXPECT_EQ(ptr->tid, "1408366");
    EXPECT_EQ(ptr->pid, "140836602");
    EXPECT_EQ(ptr->processId, "140836602");
    EXPECT_EQ(ptr->threadId, "1408366");
    EXPECT_EQ(ptr->depth, 0);
}

TEST_F(TestSuit, QueryEventsViewData4CANNThread)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    EventsViewParams params;
    params.pid = "140836602";
    params.processName = "CANN (140836602)";
    params.tid = "1408366";
    params.threadName = "Thread 1408366";
    params.pageSize = PAGE_SIZE;
    params.currentPage = CUR_PAGE_2;
    params.rankId = "0";

    EventsViewBody body;
    database->QueryEventsViewData(params, body, 0);
    const uint64_t EXPECT_COUNT = 5524;
    const uint64_t EXPECT_START = 1695115378714991500;
    const uint64_t EXPECT_DURATION = 468180;

    EXPECT_EQ(body.count, EXPECT_COUNT);
    CheckEventsViewColumns4Api(body);
    EXPECT_EQ(body.eventDetailList.size(), PAGE_SIZE);
    auto ptr = dynamic_cast<HostEventDetail*>(body.eventDetailList.at(0).get());
    EXPECT_TRUE(ptr != nullptr);
    EXPECT_EQ(ptr->name, "AscendCL@hcom_broadcast_");
    EXPECT_EQ(ptr->startTime, EXPECT_START);
    EXPECT_EQ(ptr->duration, EXPECT_DURATION);
    EXPECT_EQ(ptr->tid, "1408366");
    EXPECT_EQ(ptr->pid, "140836602");
    EXPECT_EQ(ptr->processId, "140836602");
    EXPECT_EQ(ptr->threadId, "1408366");
    EXPECT_EQ(ptr->depth, 0);
}

TEST_F(TestSuit, QueryEventsViewData4Hardware)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    EventsViewParams params;
    params.pid = "300";
    params.processName = "Ascend Hardware (300)";
    params.pageSize = PAGE_SIZE;
    params.currentPage = CUR_PAGE_2;
    params.rankId = "0";

    EventsViewBody body;
    database->QueryEventsViewData(params, body, 0);
    const uint64_t EXPECT_COUNT = 2263;
    const uint64_t EXPECT_START = 1695115378715411000;
    const uint64_t EXPECT_DURATION = 20;

    EXPECT_EQ(body.count, EXPECT_COUNT);
    CheckEventsViewColumns4Hardware(body);
    EXPECT_EQ(body.eventDetailList.size(), PAGE_SIZE);
    auto ptr = dynamic_cast<DeviceEventDetail*>(body.eventDetailList.at(0).get());
    EXPECT_TRUE(ptr != nullptr);
    EXPECT_EQ(ptr->name, "Notify Wait");
    EXPECT_EQ(ptr->startTime, EXPECT_START);
    EXPECT_EQ(ptr->duration, EXPECT_DURATION);
    EXPECT_EQ(ptr->threadName, "Stream 22");
    EXPECT_EQ(ptr->rankId, "0");
    EXPECT_EQ(ptr->processId, "300");
    EXPECT_EQ(ptr->threadId, "22");
    EXPECT_EQ(ptr->depth, 0);
}

TEST_F(TestSuit, QueryEventsViewData4HardwareStream)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    EventsViewParams params;
    params.pid = "300";
    params.processName = "Ascend Hardware (300)";
    params.tid = "16";
    params.threadName = "Stream 16";
    params.pageSize = PAGE_SIZE;
    params.currentPage = CUR_PAGE_2;
    params.rankId = "0";

    EventsViewBody body;
    database->QueryEventsViewData(params, body, 0);
    const uint64_t EXPECT_COUNT = 879;
    const uint64_t EXPECT_START = 1695115378723943800;
    const uint64_t EXPECT_DURATION = 369064;

    EXPECT_EQ(body.count, EXPECT_COUNT);
    CheckEventsViewColumns4Hardware(body);
    EXPECT_EQ(body.eventDetailList.size(), PAGE_SIZE);
    auto ptr = dynamic_cast<DeviceEventDetail*>(body.eventDetailList.at(0).get());
    EXPECT_TRUE(ptr != nullptr);
    EXPECT_EQ(ptr->name, "Tril");
    EXPECT_EQ(ptr->startTime, EXPECT_START);
    EXPECT_EQ(ptr->duration, EXPECT_DURATION);
    EXPECT_EQ(ptr->threadName, "Stream 16");
    EXPECT_EQ(ptr->rankId, "0");
    EXPECT_EQ(ptr->processId, "300");
    EXPECT_EQ(ptr->threadId, "16");
    EXPECT_EQ(ptr->depth, 0);
}

TEST_F(TestSuit, QueryEventsViewData4HCCL)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    EventsViewParams params;
    params.pid = "14083661400";
    params.processName = "HCCL (14083661400)";
    params.pageSize = PAGE_SIZE;
    params.currentPage = CUR_PAGE_1;
    params.rankId = "0";
    params.orderBy = "duration";
    params.order = "descend";

    EventsViewBody body;
    database->QueryEventsViewData(params, body, 0);
    const uint64_t EXPECT_COUNT = 28;
    const uint64_t EXPECT_START = 1695115378818416800;
    const uint64_t EXPECT_DURATION = 102721612;

    EXPECT_EQ(body.count, EXPECT_COUNT);
    CheckEventsViewColumns4Group(body);
    EXPECT_EQ(body.eventDetailList.size(), PAGE_SIZE);
    auto ptr = dynamic_cast<DeviceEventDetail*>(body.eventDetailList.at(0).get());
    EXPECT_TRUE(ptr != nullptr);
    EXPECT_EQ(ptr->name, "hcom_allReduce__459_0");
    EXPECT_EQ(ptr->startTime, EXPECT_START);
    EXPECT_EQ(ptr->duration, EXPECT_DURATION);
    EXPECT_EQ(ptr->threadName, "Group 2 Communication");
    EXPECT_EQ(ptr->rankId, "0");
    EXPECT_EQ(ptr->processId, "14083661400");
    EXPECT_EQ(ptr->threadId, "17");
    EXPECT_EQ(ptr->depth, 0);
}

TEST_F(TestSuit, QueryEventsViewData4HCCLGroup)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    EventsViewParams params;
    params.pid = "14083661400";
    params.processName = "HCCL (14083661400)";
    params.tid = "0";
    params.threadName = "Group 0 Communication";
    params.pageSize = PAGE_SIZE;
    params.currentPage = CUR_PAGE_1;
    params.rankId = "0";
    params.orderBy = "duration";
    params.order = "descend";

    EventsViewBody body;
    database->QueryEventsViewData(params, body, 0);
    const uint64_t EXPECT_COUNT = 20;
    const uint64_t EXPECT_START = 1695115378715400200;
    const uint64_t EXPECT_DURATION = 4975266;

    EXPECT_EQ(body.count, EXPECT_COUNT);
    CheckEventsViewColumns4Group(body);
    EXPECT_EQ(body.eventDetailList.size(), PAGE_SIZE);
    auto ptr = dynamic_cast<DeviceEventDetail*>(body.eventDetailList.at(0).get());
    EXPECT_TRUE(ptr != nullptr);
    EXPECT_EQ(ptr->name, "hcom_broadcast__483_0");
    EXPECT_EQ(ptr->startTime, EXPECT_START);
    EXPECT_EQ(ptr->duration, EXPECT_DURATION);
    EXPECT_EQ(ptr->threadName, "Group 0 Communication");
    EXPECT_EQ(ptr->rankId, "0");
    EXPECT_EQ(ptr->processId, "14083661400");
    EXPECT_EQ(ptr->threadId, "0");
    EXPECT_EQ(ptr->depth, 0);
}

TEST_F(TestSuit, QueryEventsViewData4Overlap)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    EventsViewParams params;
    params.pid = "14083661700";
    params.processName = "Overlap Analysis (14083661700)";
    params.pageSize = PAGE_SIZE;
    params.currentPage = CUR_PAGE_1;
    params.rankId = "0";
    params.orderBy = "duration";
    params.order = "descend";

    EventsViewBody body;
    database->QueryEventsViewData(params, body, 0);
    const uint64_t EXPECT_COUNT = 1243;
    const uint64_t EXPECT_START = 1695115378818416665;
    const uint64_t EXPECT_DURATION = 102721612;

    EXPECT_EQ(body.count, EXPECT_COUNT);
    CheckEventsViewColumns4Overlap(body);
    EXPECT_EQ(body.eventDetailList.size(), PAGE_SIZE);
    auto ptr = dynamic_cast<DeviceEventDetail*>(body.eventDetailList.at(0).get());
    EXPECT_TRUE(ptr != nullptr);
    EXPECT_EQ(ptr->name, "Communication");
    EXPECT_EQ(ptr->startTime, EXPECT_START);
    EXPECT_EQ(ptr->duration, EXPECT_DURATION);
    EXPECT_EQ(ptr->threadName, "Communication");
    EXPECT_EQ(ptr->rankId, "0");
    EXPECT_EQ(ptr->processId, "14083661700");
    EXPECT_EQ(ptr->threadId, "1");
    EXPECT_EQ(ptr->depth, 0);
}

TEST_F(TestSuit, QueryEventsViewData4OverlapComputing)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    EventsViewParams params;
    params.pid = "14083661700";
    params.processName = "Overlap Analysis (14083661700)";
    params.tid = "0";
    params.threadName = "Computing";
    params.pageSize = PAGE_SIZE;
    params.currentPage = CUR_PAGE_1;
    params.rankId = "0";
    params.orderBy = "duration";
    params.order = "descend";

    EventsViewBody body;
    database->QueryEventsViewData(params, body, 0);
    const uint64_t EXPECT_COUNT = 586;
    const uint64_t EXPECT_START = 1695115378795017389;
    const uint64_t EXPECT_DURATION = 7151681;

    EXPECT_EQ(body.count, EXPECT_COUNT);
    CheckEventsViewColumns4Overlap(body);
    EXPECT_EQ(body.eventDetailList.size(), PAGE_SIZE);
    auto ptr = dynamic_cast<DeviceEventDetail*>(body.eventDetailList.at(0).get());
    EXPECT_TRUE(ptr != nullptr);
    EXPECT_EQ(ptr->name, "Computing");
    EXPECT_EQ(ptr->startTime, EXPECT_START);
    EXPECT_EQ(ptr->duration, EXPECT_DURATION);
    EXPECT_EQ(ptr->threadName, "Computing");
    EXPECT_EQ(ptr->rankId, "0");
    EXPECT_EQ(ptr->processId, "14083661700");
    EXPECT_EQ(ptr->threadId, "0");
    EXPECT_EQ(ptr->depth, 0);
}