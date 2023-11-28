/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol Response declaration
 */

#ifndef DIC_TIMELINE_PROTOCOL_RESPONSE_H
#define DIC_TIMELINE_PROTOCOL_RESPONSE_H

#include <utility>
#include <vector>
#include "GlobalDefs.h"
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"

namespace Dic {
namespace Protocol {
struct Action {
    std::string cardName;
    std::string rankId;
    bool result = true;
};

struct ImportActionResBody {
    std::vector<Action> result;
    bool isCluster = false;
    bool reset = false;
};

struct ImportActionResponse : public Response {
    ImportActionResponse() : Response(REQ_RES_IMPORT_ACTION) {}
    ImportActionResBody body;
};

struct ThreadTraces {
    std::string name;
    uint64_t duration = 0;
    uint64_t startTime = 0;
    uint64_t endTime = 0;
    int32_t depth = 0;
    int32_t threadId = 0;
};

struct UnitThreadTracesBody {
    std::vector<std::vector<ThreadTraces>> data;
};

struct UnitThreadTracesResponse : public Response {
    UnitThreadTracesResponse() : Response(REQ_RES_UNIT_THREAD_TRACES) {}
    UnitThreadTracesBody body;
};

struct Threads {
    std::string title;
    uint64_t wallDuration = 0;
    uint64_t occurrences = 0;
    uint64_t avgWallDuration = 0;
    uint64_t selfTime = 0;
};

struct UnitThreadsBody {
    bool emptyFlag = false;
    std::vector<Threads> data;
};

struct UnitThreadsResponse : public Response {
    UnitThreadsResponse() : Response(REQ_RES_UNIT_THREADS) {}
    UnitThreadsBody body;
};

struct ThreadDetail {
    uint64_t selfTime = 0;
    uint64_t duration = 0;
    std::string args;
    std::string title;
    std::string cat;
};

struct UnitThreadDetailBody {
    bool emptyFlag = false;
    ThreadDetail data;
};

struct UnitThreadDetailResponse : public Response {
    UnitThreadDetailResponse() : Response(REQ_RES_UNIT_THREAD_DETAIL) {}
    UnitThreadDetailBody body;
};

struct FlowName {
    FlowName(std::string name, std::string id, std::string type)
        : title(std::move(name)), flowId(std::move(id)), type(std::move(type)) {};
    std::string title;
    std::string flowId;
    std::string type; // s, f
};

struct UnitFlowNameBody {
    std::vector<FlowName> flowDetail;
};

struct UnitFlowNameResponse : public Response {
    UnitFlowNameResponse() : Response(REQ_RES_UNIT_FLOW_NAME) {}
    UnitFlowNameBody body;
};

struct FlowLocation {
    int32_t tid = 0;
    int32_t depth = 0;
    uint64_t timestamp = 0;
    uint64_t duration = 0; // slice duration
    std::string pid;
    std::string name; // slice name
};

struct UnitFlowBody {
    std::string title;
    std::string cat;
    std::string id;
    FlowLocation from;
    FlowLocation to;
};

struct UnitFlowResponse : public Response {
    UnitFlowResponse() : Response(REQ_RES_UNIT_FLOW) {}
    UnitFlowBody body;
};

struct ResetWindowResponse : public Response {
    ResetWindowResponse() : Response(REQ_RES_RESET_WINDOW) {}
};

struct Chart {
    uint32_t ts = 0;
    uint32_t value = 0;
};

struct UnitChartBody {
    std::vector<Chart> data;
};

struct UnitChartResponse : public Response {
    UnitChartResponse() : Response(REQ_RES_UNIT_CHART) {}
    UnitChartBody body;
};

// struct
struct RowThreadTrace {
    int64_t id = 0;
    uint64_t startTime = 0;
    uint64_t duration = 0;
    int32_t depth = 0;
    int64_t traceId = 0;
    std::string name;
};

struct ExtremumTimestamp {
    uint64_t minTimestamp = 0;
    uint64_t maxTimestamp = 0;
};

struct SimpleSlice {
    uint64_t timestamp = 0;
    uint64_t duration = 0;
    uint64_t endTime = 0;
    int32_t depth = 0;
    std::string name;
};

struct SearchResult {
    std::string rankId;
    int count = 0;
};

struct SearchCountBody {
    int totalCount = 0;
    std::vector<SearchResult> countList;
};

struct SearchCountResponse : public Response {
    SearchCountResponse() : Response(REQ_RES_SEARCH_COUNT) {}
    SearchCountBody body;
};

struct SearchSliceBody {
    std::string rankId;
    std::string pid;
    int32_t tid = 0;
    uint64_t startTime = 0;
    uint64_t duration = 0;
    int32_t depth = 0;
};

struct SearchSliceResponse : public Response {
    SearchSliceResponse() : Response(REQ_RES_SEARCH_SLICE) {}
    SearchSliceBody body;
};

struct RemoteDeleteBody {
    bool startTimeUpdated = false;
    uint64_t maxTimeStamp = 0;
};

struct RemoteDeleteResponse : public Response {
    RemoteDeleteResponse() : Response(REQ_RES_REMOTE_DELETE) {}
    RemoteDeleteBody body;
};

struct FlowCategoryListBody {
    std::vector<std::string> category;
};

struct FlowCategoryListResponse : public Response {
    FlowCategoryListResponse() : Response(REQ_RES_FLOW_CATEGORY_LIST) {}
    FlowCategoryListBody body;
};

struct FlowEventLocation {
    int32_t tid = 0;
    int32_t depth = 0;
    uint64_t timestamp = 0;
    std::string pid;
};

struct FlowEvent {
    std::string category;
    FlowEventLocation from;
    FlowEventLocation to;
};

struct FlowCategoryEventsBody {
    std::vector<std::unique_ptr<FlowEvent>> flowDetailList;
};

struct FlowCategoryEventsResponse : public Response {
    FlowCategoryEventsResponse() : Response(REQ_RES_FLOW_CATEGORY_EVENTS) {}
    FlowCategoryEventsBody body;
};

struct UnitCounterData {
    uint64_t timestamp = 0;
    std::string valueJsonStr; // json string, need to convert to json object before send
};

struct UnitCounterBody {
    std::vector<UnitCounterData> data;
};

struct UnitCounterResponse : public Response {
    UnitCounterResponse() : Response(REQ_RES_UNIT_COUNTER) {}
    UnitCounterBody body;
};

struct SystemViewDetail {
    std::string name;
    double time;
    uint64_t totalTime;
    uint64_t numberCalls;
    double avg;
    double stdDev;
    uint64_t min;
    uint64_t max;
};

struct SystemViewBody {
    std::vector<SystemViewDetail> systemViewDetail;
    uint64_t total;
    uint64_t pageSize;
    uint64_t currentPage;
};

struct SystemViewResponse  : public Response {
    SystemViewResponse() : Response(REQ_RES_UNIT_SYSTEM_VIEW) {}
    SystemViewBody body;
};

struct KernelDetail {
    std::string name;
    std::string type;
    std::string acceleratorCore;
    uint64_t startTime;
    double duration;
    double waitTime;
    uint64_t blockDim;
    std::string inputShapes;
    std::string inputDataTypes;
    std::string inputFormats;
    std::string outputShapes;
    std::string outputDataTypes;
    std::string outputFormats;
};

struct KernelDetailsBody {
    std::vector<std::string> acceleratorCoreList;
    std::vector<KernelDetail> kernelDetails;
    uint64_t count;
    uint64_t pageSize;
    uint64_t currentPage;
};

struct KernelDetailsResponse  : public Response {
    KernelDetailsResponse() : Response(REQ_RES_UNIT_KERNEL_DETAILS) {}
    KernelDetailsBody body;
};

struct OneKernelBody {
    uint64_t depth;
    uint64_t threadId;
    std::string pid;
};

struct OneKernelResponse  : public Response {
    OneKernelResponse() : Response(REQ_RES_ONE_KERNEL_DETAILS) {}
    OneKernelBody body;
};
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_TIMELINE_PROTOCOL_RESPONSE_H
