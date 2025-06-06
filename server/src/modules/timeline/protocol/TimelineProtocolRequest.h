/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol Requests declaration
 */

#ifndef DIC_TIMELINE_PROTOCOL_REQUEST_H
#define DIC_TIMELINE_PROTOCOL_REQUEST_H

#include <string>
#include <optional>
#include <vector>
#include <set>
#include "FileUtil.h"
#include "ProtocolDefs.h"
#include "ProtocolParamUtil.h"
#include "TimelineParamStrcut.h"
#include "ProtocolMessage.h"

namespace Dic {
namespace Protocol {
enum class ProjectActionEnum {
    TRANSFER_PROJECT = 0,
    ADD_FILE,
    UNKNOWN
};

struct ImportActionParams {
    std::string projectName;
    std::vector<std::string> path;
    ProjectActionEnum projectAction = ProjectActionEnum::UNKNOWN;
    bool isConflict = false;
    bool CommonCheck(std::string &errorMsg)
    {
        if (this->projectName.empty()) {
            errorMsg = "Import project is empty.";
            return false;
        }
        if (this->projectAction == ProjectActionEnum::UNKNOWN) {
            errorMsg = "Unknown operator.";
            return false;
        }
        return true;
    }
    bool ConvertToRealPath(std::string &errorMsg)
    {
        // 导入新文件时验证，路径不允许为空
        if (this->path.empty()) {
            errorMsg = "Import file path is empty.";
            return false;
        }
        if (!FileUtil::ConvertToRealPath(errorMsg, this->path)) {
            return false;
        }
        std::string importPath = this->path.front();
        std::string realPath = FileUtil::GetRealPath(importPath);
        if (!FileUtil::IsFolder(realPath)) {
            return true;
        }
        std::vector<std::string> folders;
        std::vector<std::string> files;
        FileUtil::FindFolders(realPath, folders, files);
        if (std::empty(folders) && std::empty(files)) {
            errorMsg = "Import path is empty folder!";
            return false;
        }
        return true;
    }
};

struct ImportActionRequest : public Request {
    ImportActionRequest() : Request(REQ_RES_IMPORT_ACTION){};
    ImportActionParams params;
};
struct ParseCardsParams {
    std::vector<std::string> cards;
    std::vector<std::string> fileIds;
};
struct ParseCardsRequest : public Request {
    ParseCardsRequest() : Request(REQ_RES_PARSE_CARDS){};
    ParseCardsParams params;
};

struct UnitThreadTracesParams {
    std::string cardId;
    std::string processId;
    std::string threadId;
    std::vector<std::string> threadIdList;
    std::string metaType;
    uint64_t startTime = 0;
    uint64_t endTime = 0;
    double timePerPx = 0; // totalTime / pixel
    bool isFilterPythonFunction = false;
    bool isHideFlagEvents = false;
    bool CheckParams(uint64_t minTime, std::string &warnMsg) const
    {
        if (startTime > endTime) {
            warnMsg = "unit thread traces start time is bigger than end time";
            return false;
        }
        if (endTime > UINT64_MAX - minTime) {
            warnMsg = "unit thread traces end time is invalid";
            return false;
        }
        return true;
    }
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
    bool CheckParams(uint64_t minTime, std::string &warnMsg) const
    {
        if (startTime > endTime) {
            warnMsg = "unit threads start time is bigger than end time";
            return false;
        }
        if (endTime > UINT64_MAX - minTime) {
            warnMsg = "unit threads end time is invalid";
            return false;
        }
        return true;
    }
};

struct UnitThreadTracesSummaryRequest : public Request {
    UnitThreadTracesSummaryRequest() : Request(REQ_RES_UNIT_THREAD_TRACES_SUMMARY){};
    UnitThreadTracesSummaryParams params;
};

struct UnitThreadsParams {
    std::string rankId;
    std::vector<Metadata> metadataList;
    uint64_t startTime = 0;
    uint64_t endTime = 0;
    bool CheckParams(uint64_t minTime, std::string &warnMsg) const
    {
        if (startTime > endTime) {
            warnMsg = "unit threads start time is bigger than end time";
            return false;
        }
        if (endTime > UINT64_MAX - minTime) {
            warnMsg = "unit threads end time is invalid";
            return false;
        }
        return true;
    }
};

struct UnitThreadsRequest : public Request {
    UnitThreadsRequest() : Request(REQ_RES_UNIT_THREADS){};
    UnitThreadsParams params;
};

struct ThreadDetailRequest : public Request {
    ThreadDetailRequest() : Request(REQ_RES_UNIT_THREAD_DETAIL){};
    ThreadDetailParams params;
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
    bool CheckParams(uint64_t minTime, std::string &warnMsg) const
    {
        if (startTime > endTime) {
            warnMsg = "unit flows start time is bigger than end time";
            return false;
        }
        if (endTime > UINT64_MAX - minTime) {
            warnMsg = "unit flows end time is invalid";
            return false;
        }
        return true;
    }
};

struct UnitFlowsRequest : public Request {
    UnitFlowsRequest() : Request(REQ_RES_UNIT_FLOWS){};
    UnitFlowsParams params;
};

struct SetCardAliasRequest : public Request {
    SetCardAliasRequest() : Request(REQ_RES_UNIT_SET_CARD_ALIAS){};
    SetCardAliasParams params;
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
    std::vector<Metadata> metadataList;
    bool CheckParams(uint64_t minTime, std::string &warnMsg) const
    {
        for (const auto &item: metadataList) {
            if (item.lockStartTime > item.lockEndTime) {
                warnMsg = "Search count lock start time is bigger than lock end time";
                return false;
            }
            if (item.lockEndTime > UINT64_MAX - minTime) {
                warnMsg = "Search count events lock end time is invalid";
                return false;
            }
        }
        return true;
    }
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
    std::vector<Metadata> metadataList;
    bool CheckParams(uint64_t minTime, std::string &warnMsg) const
    {
        for (const auto &item: metadataList) {
            if (item.lockStartTime > item.lockEndTime) {
                warnMsg = "Search slice lock start time is bigger than lock end time";
                return false;
            }
            if (item.lockEndTime > UINT64_MAX - minTime) {
                warnMsg = "Search slice events lock end time is invalid";
                return false;
            }
        }
        return true;
    }
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
    std::vector<Metadata> metadataList;
    uint64_t lockStartTime = 0;
    uint64_t lockEndTime = 0;
    bool CheckParams(uint64_t minTime, std::string &warnMsg) const
    {
        if (startTime > endTime) {
            warnMsg = "flow category events start time is bigger than end time";
            return false;
        }
        if (endTime > UINT64_MAX - minTime) {
            warnMsg = "flow category events end time is invalid";
            return false;
        }
        if (lockStartTime > lockEndTime) {
            warnMsg = "flow category events lock start time is bigger than lock end time";
            return false;
        }
        if (lockEndTime > UINT64_MAX - minTime) {
            warnMsg = "flow category events lock end time is invalid";
            return false;
        }
        return true;
    }
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
    bool CheckParams(uint64_t minTime, std::string &warnMsg) const
    {
        if (startTime > endTime) {
            warnMsg = "unit counter start time is bigger than end time";
            return false;
        }
        if (endTime > UINT64_MAX - minTime) {
            warnMsg = "unit counter end time is invalid";
            return false;
        }
        return true;
    }
};

struct UnitCounterRequest : public Request {
    UnitCounterRequest() : Request(REQ_RES_UNIT_COUNTER){};
    UnitCounterParams params;
};

struct SystemViewOverallReqParam {
    std::string rankId;
    std::vector<std::string> categoryList;
    std::string name;
    OrderParam order;
    PageParam page{};
};

struct SystemViewOverallRequest : public Request {
    SystemViewOverallRequest() : Request(REQ_RES_SYSTEM_VIEW_OVERALL) {};
    SystemViewOverallReqParam params;
};

struct SystemViewOverallMoreDetailsRequest : public Request {
    SystemViewOverallMoreDetailsRequest() : Request(REQ_RES_SYSTEM_VIEW_OVERALL_MORE_DETAILS) {};
    SystemViewOverallReqParam params;
};

struct SystemViewParams {
    std::string orderBy;
    std::string order;
    uint64_t current = 0;
    uint64_t pageSize = 0;
    std::string type;
    std::string rankId;
    uint64_t endTime = 0;
    bool isQueryTotal = false;
    std::string layer;
    std::string searchName;
    bool CheckParams(std::string &warnMsg) const
    {
        static const std::set<std::string> validLayerTypeSet = {
            "Python", "CANN",          "Ascend Hardware",
            "HCCL",   "COMMUNICATION", "Overlap Analysis" };
        if (validLayerTypeSet.find(layer) == validLayerTypeSet.end()) {
            warnMsg = "Layer is invalid";
            return false;
        }
        return CheckUnsignPageValid(pageSize, current, warnMsg);
    }
};

struct SystemViewRequest : public Request {
    SystemViewRequest() : Request(REQ_RES_UNIT_SYSTEM_VIEW){};
    SystemViewParams params;
};

struct SystemViewAICoreFreqParams {
    std::string rankId;
};

struct ExpAnaAICoreFreqRequest : public Request {
    ExpAnaAICoreFreqRequest() : Request(REQ_RES_EXPERT_ANALYSIS_AICORE_FREQ){};
    SystemViewAICoreFreqParams params;
};

struct EventsViewParams {
    std::string orderBy;
    std::string order;
    uint64_t currentPage = 0;
    uint64_t pageSize = 0;
    std::string rankId;
    std::string pid;
    std::string processName;
    std::string tid;
    std::string threadName;
    std::string metaType;
    std::vector<std::string> threadIdList;
    bool CheckParams(std::string &warnMsg) const
    {
        return CheckUnsignPageValid(pageSize, currentPage, warnMsg);
    }
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
    void Check(std::string &error) const;
};

struct KernelDetailsRequest : public Request {
    KernelDetailsRequest() : Request(REQ_RES_UNIT_KERNEL_DETAILS){};
    KernelDetailsParams params;
};

struct KernelParams {
    std::string rankId;
    std::string name;
    uint64_t timestamp = 0;
    uint64_t duration = 0;
    bool CheckParams(uint64_t minTime, std::string &warnMsg) const
    {
        if (timestamp > UINT64_MAX - minTime) {
            warnMsg = "kernel time is invalid";
            return false;
        }
        return true;
    }
};

struct KernelRequest : public Request {
    KernelRequest() : Request(REQ_RES_ONE_KERNEL_DETAILS){};
    KernelParams params;
};

struct CommunicationKernelParams {
    std::string rankId;
    std::string name;
    std::string clusterPath;
};

struct CommunicationKernelRequest : public Request {
    CommunicationKernelRequest() : Request(REQ_RES_COMMUNICATION_KERNEL_DETAIL){};
    CommunicationKernelParams params;
};

struct UnitThreadsOperatorsParams {
    std::string rankId;
    std::vector<std::string> tid;
    std::string pid;
    std::string metaType;
    uint64_t startTime = 0;
    uint64_t endTime = 0;
    std::string name;
    std::string orderBy;
    std::string order;
    uint64_t current = 0;
    uint64_t pageSize = 0;
    bool CheckParams(uint64_t minTime, std::string &warnMsg) const
    {
        if (startTime > endTime) {
            warnMsg = "unit threads operators start time is bigger than end time";
            return false;
        }
        if (endTime > UINT64_MAX - minTime) {
            warnMsg = "unit threads operators end time is invalid";
            return false;
        }
        if (tid.empty()) {
            warnMsg = "Failed to query threads same operator. Track id list is empty.";
        }
        return CheckUnsignPageValid(pageSize, current, warnMsg);
    }
};

struct UnitThreadsOperatorsRequest : public Request {
    UnitThreadsOperatorsRequest() : Request(REQ_RES_SAME_OPERATORS_DURATION){};
    UnitThreadsOperatorsParams params;
};

struct SearchAllSlicesRequest : public Request {
    SearchAllSlicesRequest() : Request(REQ_RES_SEARCH_ALL_SLICES){};
    SearchAllSliceParams params;
};

struct TableDataNameListRequest : public Request {
    TableDataNameListRequest() : Request(REQ_RES_TABLE_DATA_NAME_LIST){};
    TableDataNameListParams params;
};

struct TableDataDetailRequest : public Request {
    TableDataDetailRequest() : Request(REQ_RES_TABLE_DATA_DETAIL){};
    TableDataDetailParams params;
};
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_TIMELINE_PROTOCOL_REQUEST_H