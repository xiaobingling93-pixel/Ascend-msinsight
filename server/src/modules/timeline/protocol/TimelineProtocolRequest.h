/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol Requests declaration
 */

#ifndef DIC_TIMELINE_PROTOCOL_REQUEST_H
#define DIC_TIMELINE_PROTOCOL_REQUEST_H

#include <string>
#include <optional>
#include <vector>
#include "FileUtil.h"
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"

namespace Dic {
namespace Protocol {
struct ImportActionParams {
    std::string projectName;
    std::vector<std::string> path;
    bool CommonCheck(std::string &errorMsg)
    {
        if (this->projectName.empty()) {
            errorMsg = "Import project is empty.";
            return false;
        }
        return true;
    }
    bool ConvertToRealPath(std::string &errorMsg)
    {
        return FileUtil::ConvertToRealPath(errorMsg, this->path);
    }
};

struct ImportActionRequest : public Request {
    ImportActionRequest() : Request(REQ_RES_IMPORT_ACTION){};
    ImportActionParams params;
};
struct ParseCardsParams {
    std::vector<std::string> cards;
};
struct ParseCardsRequest : public Request {
    ParseCardsRequest() : Request(REQ_RES_PARSE_CARDS){};
    ParseCardsParams params;
};

struct UnitThreadTracesParams {
    std::string cardId;
    std::string processId;
    std::string threadId;
    std::string metaType;
    uint64_t startTime = 0;
    uint64_t endTime = 0;
    double timePerPx = 0; // totalTime / pixel
    bool isFilterPythonFunction = false;
    bool isHideFlagEvents = false;
};

struct UnitThreadTracesRequest : public Request {
    UnitThreadTracesRequest() : Request(REQ_RES_UNIT_THREAD_TRACES){};
    UnitThreadTracesParams params;
};

struct UnitThreadTracesSummaryParams {
    std::string cardId;
    std::string processId;
    std::string metaType;
    uint64_t startTime = 0;
    uint64_t endTime = 0;
};

struct UnitThreadTracesSummaryRequest : public Request {
    UnitThreadTracesSummaryRequest() : Request(REQ_RES_UNIT_THREAD_TRACES_SUMMARY){};
    UnitThreadTracesSummaryParams params;
};

struct Metadata {
    std::string tid;
    std::string pid;
    std::string metaType;
};

struct UnitThreadsParams {
    std::string rankId;
    std::vector<Metadata> metadataList;
    uint64_t startTime = 0;
    uint64_t endTime = 0;
};

struct UnitThreadsRequest : public Request {
    UnitThreadsRequest() : Request(REQ_RES_UNIT_THREADS){};
    UnitThreadsParams params;
};

struct ThreadDetailParams {
    std::string rankId;
    std::string pid;
    std::string tid;
    std::string id;
    std::string metaType;
    uint64_t startTime = 0;
    uint32_t depth = 0;
};

struct ThreadDetailRequest : public Request {
    ThreadDetailRequest() : Request(REQ_RES_UNIT_THREAD_DETAIL){};
    ThreadDetailParams params;
};

struct UnitFlowNameParams {
    std::string rankId;
    std::string tid;
    std::string pid;
    std::string id;
    std::string metaType;
    uint64_t startTime = 0;
    uint64_t endTime = 0;
};

struct UnitFlowNameRequest : public Request {
    UnitFlowNameRequest() : Request(REQ_RES_UNIT_FLOW_NAME){};
    UnitFlowNameParams params;
};

struct UnitFlowsParams {
    std::string rankId;
    std::string tid;
    std::string pid;
    std::string id;
    std::string metaType;
    uint64_t startTime = 0;
    uint64_t endTime = 0;
    bool isSimulation = false;
};

struct UnitFlowsRequest : public Request {
    UnitFlowsRequest() : Request(REQ_RES_UNIT_FLOWS){};
    UnitFlowsParams params;
};

struct UnitFlowParams {
    uint64_t startTime = 0;
    std::string flowId;
    std::string rankId;
    std::string id;
    std::string metaType;
    std::string type; // s, f
};

struct UnitFlowRequest : public Request {
    UnitFlowRequest() : Request(REQ_RES_UNIT_FLOW){};
    UnitFlowParams params;
};

struct ResetWindowParams {};

struct ResetWindowRequest : public Request {
    ResetWindowRequest() : Request(REQ_RES_RESET_WINDOW){};
    ResetWindowParams params;
};

struct SearchCountParams {
    bool isMatchCase = false;
    bool isMatchExact = false;
    std::string rankId;
    std::string searchContent;
};

struct SearchCountRequest : public Request {
    SearchCountRequest() : Request(REQ_RES_SEARCH_COUNT){};
    SearchCountParams params;
};

struct SearchSliceParams {
    bool isMatchCase = false;
    bool isMatchExact = false;
    std::string rankId;
    std::string searchContent;
    int index = 0;
};

struct SearchSliceRequest : public Request {
    SearchSliceRequest() : Request(REQ_RES_SEARCH_SLICE){};
    SearchSliceParams params;
};

struct RemoteDeleteParams {
    std::vector<std::string> rankId;
};

struct RemoteDeleteRequest : public Request {
    RemoteDeleteRequest() : Request(REQ_RES_REMOTE_DELETE){};
    RemoteDeleteParams params;
};

struct FlowCategoryListParams {
    std::string rankId;
};

struct FlowCategoryListRequest : public Request {
    FlowCategoryListRequest() : Request(REQ_RES_FLOW_CATEGORY_LIST){};
    FlowCategoryListParams params;
};

struct FlowCategoryEventsParams {
    std::string rankId;
    std::string host;
    std::string category;
    uint64_t startTime = 0;
    uint64_t endTime = 0;
    double timePerPx = 0;
    bool isSimulation = false;
};

struct FlowCategoryEventsRequest : public Request {
    FlowCategoryEventsRequest() : Request(REQ_RES_FLOW_CATEGORY_EVENTS){};
    FlowCategoryEventsParams params;
};

struct UnitCounterParams {
    std::string rankId;
    std::string pid;
    std::string threadName;
    std::string threadId;
    uint64_t startTime = 0;
    uint64_t endTime = 0;
    std::string metaType;
};

struct UnitCounterRequest : public Request {
    UnitCounterRequest() : Request(REQ_RES_UNIT_COUNTER){};
    UnitCounterParams params;
};

struct SystemViewParams {
    std::string orderBy;
    std::string order;
    uint64_t current;
    uint64_t pageSize;
    std::string type;
    std::string rankId;
    uint64_t endTime;
    bool isQueryTotal = false;
    std::string layer;
    std::string searchName;
};

struct SystemViewRequest : public Request {
    SystemViewRequest() : Request(REQ_RES_UNIT_SYSTEM_VIEW){};
    SystemViewParams params;
};

struct EventsViewParams {
    std::string orderBy;
    std::string order;
    uint64_t currentPage;
    uint64_t pageSize;
    std::string rankId;
    std::string pid;
    std::string processName;
    std::string tid;
    std::string threadName;
    std::string metaType;
};

struct EventsViewRequest : public Request {
    EventsViewRequest() : Request(REQ_RES_UNIT_EVENTS_VIEW){};
    EventsViewParams params;
};

struct KernelDetailsParams {
    std::string orderBy;
    std::string order;
    uint64_t current{};
    uint64_t pageSize{};
    std::string rankId;
    std::string coreType;
    std::string searchName;
    std::vector<std::pair<std::string, std::string>> filters;
    void Check(std::string &error) const
    {
        if (current == 0) {
            error = "current is invaild";
            return;
        }
        if (pageSize == 0) {
            error = "pageSize is invaild";
            return;
        }
    }
};

struct KernelDetailsRequest : public Request {
    KernelDetailsRequest() : Request(REQ_RES_UNIT_KERNEL_DETAILS){};
    KernelDetailsParams params;
};

struct KernelParams {
    std::string rankId;
    std::string name;
    uint64_t timestamp;
    uint64_t duration;
};

struct KernelRequest : public Request {
    KernelRequest() : Request(REQ_RES_ONE_KERNEL_DETAILS){};
    KernelParams params;
};

struct UnitThreadsOperatorsParams {
    std::string rankId;
    std::string tid;
    std::string pid;
    std::string metaType;
    uint64_t startTime = 0;
    uint64_t endTime = 0;
    std::string name;
    std::string orderBy;
    std::string order;
    uint64_t current;
    uint64_t pageSize;
};

struct UnitThreadsOperatorsRequest : public Request {
    UnitThreadsOperatorsRequest() : Request(REQ_RES_SAME_OPERATORS_DURATION){};
    UnitThreadsOperatorsParams params;
};

struct SearchAllSliceParams {
    bool isMatchCase = false;
    bool isMatchExact = false;
    std::string rankId;
    std::string searchContent;
    std::string orderBy;
    std::string order;
    uint64_t current;
    uint64_t pageSize;
};

struct SearchAllSlicesRequest : public Request {
    SearchAllSlicesRequest() : Request(REQ_RES_SEARCH_ALL_SLICES){};
    SearchAllSliceParams params;
};
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_TIMELINE_PROTOCOL_REQUEST_H