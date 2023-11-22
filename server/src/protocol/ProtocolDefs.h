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
const std::string MODULE_OPERATOR = "operator";
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
const std::string EVENT_PARSE_FAIL = "parse/fail";
const std::string EVENT_PARSE_CLUSTER_COMPLETED = "parse/clusterCompleted";
const std::string EVENT_PARSE_MEMORY_COMPLETED = "parse/memoryCompleted";
#pragma endregion

#pragma region << Request / Response Command>>
// global
const std::string REQ_RES_TOKEN_CREATE = "token.create";
const std::string REQ_RES_TOKEN_DESTROY = "token.destroy";
const std::string REQ_RES_TOKEN_CHECK = "token.check";
const std::string REQ_RES_FILES_GET = "files/get";

// timeline
const std::string REQ_RES_IMPORT_ACTION = "import/action";
const std::string REQ_RES_UNIT_THREAD_TRACES = "unit/threadTraces";
const std::string REQ_RES_UNIT_THREADS = "unit/threads";
const std::string REQ_RES_UNIT_THREAD_DETAIL = "unit/threadDetail";
const std::string REQ_RES_UNIT_FLOW_NAME = "unit/flowName";
const std::string REQ_RES_UNIT_FLOW = "unit/flow";
const std::string REQ_RES_RESET_WINDOW = "remote/reset";
const std::string REQ_RES_UNIT_CHART = "unit/chart";
const std::string REQ_RES_UNIT_SYSTEM_VIEW = "unit/systemView";
const std::string REQ_RES_UNIT_KERNEL_DETAILS = "unit/kernelDetails";
const std::string REQ_RES_ONE_KERNEL_DETAILS = "unit/one/kernelDetail";
const std::string REQ_RES_SEARCH_COUNT = "search/count";
const std::string REQ_RES_SEARCH_SLICE = "search/slice";
const std::string REQ_RES_REMOTE_DELETE = "remote/delete";
const std::string REQ_RES_FLOW_CATEGORY_LIST = "flow/categoryList";
const std::string REQ_RES_FLOW_CATEGORY_EVENTS = "flow/categoryEvents";
const std::string REQ_RES_UNIT_COUNTER = "unit/counter";

// communication
const std::string REQ_RES_COMMUNICATION_OPERATOR_DETAILS = "communication/operatorDetails";
const std::string REQ_RES_COMMUNICATION_BANDWIDTH = "communication/bandwidth";
const std::string REQ_RES_COMMUNICATION_DISTRIBUTION = "communication/distribution";
const std::string REQ_RES_COMMUNICATOR_PARSE = "communicator/parse";
const std::string REQ_RES_COMMUNICATION_ITERATIONS = "communication/duration/iterations";
const std::string REQ_RES_COMMUNICATION_RANKS = "communication/duration/ranks";
const std::string REQ_RES_COMMUNICATION_OPERATORNAMES = "communication/duration/operatorNames";
const std::string REQ_RES_COMMUNICATION_LIST = "communication/duration/list";

// summary
const std::string REQ_RES_SUMMARY_QUERY_TOP_DATA = "summary/queryTopData";
const std::string REQ_RES_SUMMARY_STATISTIC = "summary/statistic";
const std::string REQ_RES_COMPUTE_DETAIL = "summary/queryComputeDetail";
const std::string REQ_RES_COMMUNICATION_DETAIL = "summary/queryCommunicationDetail";
const std::string AFFINITY_OPERATOR_QUERY = "summary/queryAffinityOperator";

// pipeline
const std::string REQ_RES_PIPELINE_GET_ALL_STEPS = "parallelism/pipeline/getAllSteps";
const std::string REQ_RES_PIPELINE_GET_ALL_STAGES = "parallelism/pipeline/getAllStages";
const std::string REQ_RES_PIPELINE_STAGE_BUBBLE = "parallelism/pipeline/stageAndBubbleTime";
const std::string REQ_RES_PIPELINE_RANK_BUBBLE = "parallelism/pipeline/rankAndBubbleTime";

// matrix
const std::string REQ_RES_COMMUNICATION_MATRIX_GROUP = "communication/matrix/group";
const std::string REQ_RES_COMMUNICATION_MATRIX_BANDWIDTH = "communication/matrix/bandwidthInfo";

// memory
const std::string REQ_RES_MEMORY_OPERATOR = "Memory/view/operator";
const std::string REQ_RES_MEMORY_VIEW = "Memory/view/memoryUsage";
const std::string REQ_RES_MEMORY_OPERATOR_MIN_MAX = "Memory/view/operator/size";

// Operator Request
const std::string REQ_RES_OPERATOR_CATEGORY_INFO = "operator/category";
const std::string REQ_RES_OPERATOR_COMPUTE_UNIT_INFO = "operator/compute_unit";
const std::string REQ_RES_OPERATOR_STATISTIC_INFO = "operator/statistic";
const std::string REQ_RES_OPERATOR_DETAIL_INFO = "operator/details";
const std::string REQ_RES_OPERATOR_MORE_INFO  = "operator/more_info";

// Operator Event
const std::string EVENT_PARSE_OPERATOR_STATUS = "parse/operatorCompleted";

#pragma endregion
} // end of namespace
}

#endif // DIC_PROTOCOL_DEFS_H