/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol defines declaration
 */

#ifndef DIC_PROTOCOL_DEFS_H
#define DIC_PROTOCOL_DEFS_H

#include <string>

namespace Dic {
namespace Protocol {
#pragma region <<Module>>
const std::string MODULE_UNKNOWN = "unknown";
const std::string MODULE_GLOBAL = "global";
const std::string MODULE_TIMELINE = "timeline";
const std::string MODULE_SUMMARY = "summary";
const std::string MODULE_COMMUNICATION = "communication";
const std::string MODULE_MEMORY = "memory";
#pragma endregion

#pragma region << Base>>
const std::string REQUEST_NAME = "request";
const std::string RESPONSE_NAME = "response";
const std::string EVENT_NAME = "event";
#pragma endregion

#pragma region << Event Name>>
const std::string EVENT_INITIALIZED = "initialized";
// timeline
const std::string EVENT_PARSE_SUCCESS = "parse/success";
#pragma endregion

#pragma region << Request / Response Command>>
// global
const std::string REQ_RES_TOKEN_CREATE = "token.create";
const std::string REQ_RES_TOKEN_DESTROY = "token.destroy";
const std::string REQ_RES_TOKEN_CHECK = "token.check";

// template
const std::string REQ_RES_IMPORT_ACTION = "import/action";
const std::string REQ_RES_UNIT_THREAD_TRACES = "unit/threadTraces";
const std::string REQ_RES_UNIT_THREADS = "unit/threads";
const std::string REQ_RES_UNIT_THREAD_DETAIL = "unit/threadDetail";
const std::string REQ_RES_UNIT_FLOW_NAME = "unit/flowName";
const std::string REQ_RES_UNIT_FLOW = "unit/flow";
const std::string REQ_RES_RESET_WINDOW = "reset/window";
const std::string REQ_RES_UNIT_CHART = "unit/chart";
const std::string REQ_RES_SEARCH_COUNT = "search/count";
const std::string REQ_RES_SEARCH_SLICE = "search/slice";

// communication
const std::string REQ_RES_COMMUNICATION_OPERATOR_DETAILS = "communication/operatorDetails";
const std::string REQ_RES_COMMUNICATION_BANDWIDTH = "communication/bandwidth";
const std::string REQ_RES_COMMUNICATION_DISTRIBUTION = "communication/distribution";

// summary
const std::string REQ_RES_SUMMARY_QUERY_TOP_DATA = "summary/queryTopData";
const std::string REQ_RES_SUMMARY_STATISTIC = "summary/statistic";
const std::string REQ_RES_COMPUTE_DETAIL = "summary/queryComputeDetail";
const std::string REQ_RES_COMMUNICATION_DETAIL = "summary/queryCommunicationDetail";

// memory
const std::string REQ_RES_MEMORY_OPERATOR = "Memory/view/operator";
const std::string REQ_RES_MEMORY_VIEW = "Memory/view/memoryUsage";
#pragma endregion
} // end of namespace
}

#endif // DIC_PROTOCOL_DEFS_H