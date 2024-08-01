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
    std::string cardPath;
    std::string host;
    bool result = true;
    std::vector<std::string> dataPathList;
};

struct ImportActionResBody {
    std::vector<Action> result;
    std::vector<std::string> coreList;
    std::vector<std::string> sourceList;
    bool isBinary = false;
    bool isCluster = false;
    bool reset = true;
    bool isSimulation = false;
    bool isIpynb = false;
    // 导入文件是否待解析
    bool isPending = false;
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
    std::string threadId;
    std::string id;
    std::string cname;
};

struct UnitThreadTracesBody {
    uint64_t maxDepth = 0;
    bool havePythonFunction = false;
    std::vector<std::vector<ThreadTraces>> data;
};

struct UnitThreadTracesResponse : public Response {
    UnitThreadTracesResponse() : Response(REQ_RES_UNIT_THREAD_TRACES) {}
    UnitThreadTracesBody body;
};

struct ThreadTracesSummary {
    uint64_t startTime = 0;
    uint64_t duration = 0;
};

struct UnitThreadTracesSummaryBody {
    std::vector<ThreadTracesSummary> data;
};

struct UnitThreadTracesSummaryResponse : public Response {
    UnitThreadTracesSummaryResponse() : Response(REQ_RES_UNIT_THREAD_TRACES) {}
    UnitThreadTracesSummaryBody body;
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
    std::string inputShapes;
    std::string inputDataTypes;
    std::string inputFormats;
    std::string outputShapes;
    std::string outputDataTypes;
    std::string outputFormats;
    std::string attrInfo;
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
    std::string title;
    std::string flowId;
    std::string type; // s, f
    uint64_t timestamp;
};

struct UnitFlowNameBody {
    std::vector<FlowName> flowDetail;
};

struct UnitFlowNameResponse : public Response {
    UnitFlowNameResponse() : Response(REQ_RES_UNIT_FLOW_NAME) {}
    UnitFlowNameBody body;
};

struct FlowLocation {
    std::string tid;
    std::string id;
    std::string metaType;
    std::string rankId;
    int32_t depth = 0;
    uint64_t timestamp = 0;
    uint64_t duration = 0; // slice duration
    std::string pid;
    std::string name; // slice name
    std::string deviceId;
    std::string type;
};

struct UnitSingleFlow {
    std::string cat;
    std::string id;
    FlowLocation from;
    FlowLocation to;
    std::string title;
};

struct UnitFlowResponse : public Response {
    UnitFlowResponse() : Response(REQ_RES_UNIT_FLOW) {}
    UnitSingleFlow body;
};

struct UnitCatFlows {
    std::string cat;
    std::vector<UnitSingleFlow> flows;
};

struct UnitFlowsBody {
    std::vector<UnitCatFlows> unitAllFlows;
};

struct UnitFlowsResponse : public Response {
    UnitFlowsResponse() : Response(REQ_RES_UNIT_FLOWS) {}
    UnitFlowsBody body;
};

struct ResetWindowResponse : public Response {
    ResetWindowResponse() : Response(REQ_RES_RESET_WINDOW) {}
};

// struct
struct RowThreadTrace {
    int64_t id = 0;
    uint64_t startTime = 0;
    uint64_t duration = 0;
    uint32_t depth = 0;
    int64_t traceId = 0;
    std::string name;
    std::string cname;
    bool operator < (const RowThreadTrace &right) const
    {
        if (depth < right.depth) {
            return true;
        }
        if (depth == right.depth && startTime < right.startTime) {
            return true;
        }
        return false;
    }
};

struct ExtremumTimestamp {
    uint64_t minTimestamp = 0;
    uint64_t maxTimestamp = 0;
};

struct SimpleSlice {
    uint64_t id = 0;
    uint64_t timestamp = 0;
    uint64_t duration = 0;
    uint64_t endTime = 0;
    uint32_t depth = 0;
    std::string name;
    bool operator < (const SimpleSlice &right) const
    {
        if (depth < right.depth) {
            return true;
        }
        if (depth == right.depth && timestamp < right.timestamp) {
            return true;
        }
        return false;
    }

    bool operator > (const SimpleSlice &right) const
    {
        if (depth > right.depth) {
            return true;
        }
        if (depth == right.depth && timestamp > right.timestamp) {
            return true;
        }
        return false;
    }
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
    std::string tid;
    std::string id;
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

struct FlowCategoryEventsBody {
    std::vector<std::unique_ptr<UnitSingleFlow>> flowDetailList;
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
    double totalTime;
    uint64_t numberCalls;
    double avg;
    double stdDev;
    double min;
    double max;
};

struct SystemViewBody {
    std::vector<SystemViewDetail> systemViewDetail;
    uint64_t total = 0;
    uint64_t pageSize;
    uint64_t currentPage;
};

struct SystemViewResponse : public Response {
    SystemViewResponse() : Response(REQ_RES_UNIT_SYSTEM_VIEW) {}
    SystemViewBody body;
};

class EventDetail {
public:
    virtual void Base() {}
    std::string id;
    std::string name;
    uint64_t startTime = 0;
    uint64_t duration = 0;
    uint64_t depth = 0;
    std::string threadId;
    std::string processId;
};

class HostEventDetail : public EventDetail {
public:
    std::string tid;
    std::string pid;
};

class DeviceEventDetail : public EventDetail {
public:
    std::string threadName;
    std::string rankId;
};

struct EventsViewColumnAttr {
    std::string name;
    std::string type;
    std::string key;
};

struct EventsViewBody {
    std::vector<EventsViewColumnAttr> columnList;
    std::vector<std::unique_ptr<EventDetail>> eventDetailList;
    uint64_t count;
    uint64_t pageSize;
    uint64_t currentPage;
};

struct EventsViewResponse : public Response {
    EventsViewResponse() : Response(REQ_RES_UNIT_EVENTS_VIEW) {}
    EventsViewBody body;
};

struct ParseCardsBody {
    bool isContinueParse = false;
};

struct ParseCardsResponse : public Response {
    ParseCardsResponse() : Response(REQ_RES_PARSE_CARDS) {}
    ParseCardsBody body;
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
    uint64_t count = 0;
    uint64_t pageSize;
    uint64_t currentPage;
};

struct KernelDetailsResponse : public Response {
    KernelDetailsResponse() : Response(REQ_RES_UNIT_KERNEL_DETAILS) {}
    KernelDetailsBody body;
};

struct OneKernelBody {
    std::string id;
    uint64_t depth;
    std::string threadId;
    std::string pid;
    std::string step;
    std::string group;
    std::string rankId;
};

struct KernelBaseInfo {
    std::string rankId;
    std::string name;
    std::string type;
    uint64_t startTime{};
    uint64_t duration{};
    std::string pid;
    std::string tid;
    uint64_t depth{};
    std::string inputType;
    std::string outputType;
    std::string note;
};

struct OneKernelResponse : public Response {
    OneKernelResponse() : Response(REQ_RES_ONE_KERNEL_DETAILS) {}
    OneKernelBody body;
};

struct SameOperatorsDetails {
    uint64_t timestamp = 0;
    uint64_t duration = 0;
    std::string id;
    uint64_t depth;
};

struct UnitThreadsOperatorsBody {
    std::vector<SameOperatorsDetails> sameOperatorsDetails;
    uint64_t count;
    uint64_t pageSize;
    uint64_t currentPage;
};

struct UnitThreadsOperatorsResponse : public Response {
    UnitThreadsOperatorsResponse() : Response(REQ_RES_SAME_OPERATORS_DURATION) {}
    UnitThreadsOperatorsBody body;
};

struct UploadFileResBody {
    std::vector<Action> result;
    bool isCluster = false;
    bool reset = false;
};

struct UploadFileResponse : public Response {
    UploadFileResponse() : Response(REQ_RES_UPLOAD_FILE) {}
    UploadFileResBody body;
};

struct SearchAllSlices {
    std::string name;
    uint64_t timestamp = 0;
    uint64_t duration = 0;
    std::string id;
    std::string pid;
    std::string tid;
    uint64_t depth = 0;
    std::string deviceId;
};

struct SearchAllSlicesBody {
    std::vector<SearchAllSlices> searchAllSlices;
    uint64_t count;
    uint64_t pageSize;
    uint64_t currentPage;
};

struct SearchAllSlicesResponse : public Response {
    SearchAllSlicesResponse() : Response(REQ_RES_SEARCH_ALL_SLICES) {}
    SearchAllSlicesBody body;
};
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_TIMELINE_PROTOCOL_RESPONSE_H
