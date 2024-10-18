/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol defines declaration
 */

#ifndef DIC_PROTOCOL_DEFS_H
#define DIC_PROTOCOL_DEFS_H

#include <string>

namespace Dic {
namespace Protocol {
#pragma region <<JsonDataBase>>
const std::string LINE_START = "s";
const std::string LINE_END = "f";
const std::string LINE_END_OPTIONAL = "t";
#pragma region <<JsonDataBase>>

#pragma region <<Module>>
const std::string MODULE_UNKNOWN = "unknown";
const std::string MODULE_GLOBAL = "global";
const std::string MODULE_TIMELINE = "timeline";
const std::string MODULE_SUMMARY = "summary";
const std::string MODULE_COMMUNICATION = "communication";
const std::string MODULE_MEMORY = "memory";
const std::string MODULE_OPERATOR = "operator";
const std::string MODULE_SOURCE = "source";
const std::string MODULE_ADVISOR = "advisor";
const std::string MODULE_JUPYTER = "jupyter";
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
const std::string EVENT_ALL_SUCCESS = "allPagesSuccess";
const std::string EVENT_PARSE_CLUSTER_STEP2_COMPLETED = "parse/clusterStep2Completed";
const std::string EVENT_PARSE_MEMORY_COMPLETED = "parse/memoryCompleted";
const std::string EVENT_MODULE_RESET = "module.reset";
const std::string EVENT_PARSE_PROGRESS = "parse/progress";
#pragma endregion

#pragma region << Request / Response Command>>
// global
const std::string REQ_RES_HEART_CHECK = "heartCheck";
const std::string REQ_RES_FILES_GET = "files/get";
const std::string REQ_RES_PROJECT_EXPLORER_UPDATE = "files/updateProjectExplorer";
const std::string REQ_RES_PROJECT_EXPLORER_INFO_GET = "files/getProjectExplorer";
const std::string REQ_RES_PROJECT_EXPLORER_INFO_DELETE = "files/deleteProjectExplorer";
const std::string REQ_RES_PROJECT_EXPLORER_CLEAR = "files/clearProjectExplorer";
const std::string REQ_RES_PROJECT_VALID_CHECK = "files/checkProjectValid";
const std::string REQ_RES_PROJECT_SET_BASELINE = "global/setBaseline";
const std::string REQ_RES_PROJECT_CANCEL_BASELINE = "global/cancelBaseline";

// timeline
const std::string REQ_RES_IMPORT_ACTION = "import/action";
const std::string REQ_RES_PARSE_CARDS = "parse/cards";
const std::string REQ_RES_UNIT_THREAD_TRACES = "unit/threadTraces";
const std::string REQ_RES_UNIT_THREAD_TRACES_SUMMARY = "unit/threadTracesSummary";
const std::string REQ_RES_UNIT_THREADS = "unit/threads";
const std::string REQ_RES_UNIT_THREAD_DETAIL = "unit/threadDetail";
const std::string REQ_RES_UNIT_FLOWS = "unit/flows";
const std::string REQ_RES_RESET_WINDOW = "remote/reset";
const std::string REQ_RES_UNIT_SYSTEM_VIEW = "unit/systemView";
const std::string REQ_RES_UNIT_EVENTS_VIEW = "unit/eventView";
const std::string REQ_RES_UNIT_KERNEL_DETAILS = "unit/kernelDetails";
const std::string REQ_RES_ONE_KERNEL_DETAILS = "unit/one/kernelDetail";
const std::string REQ_RES_SEARCH_COUNT = "search/count";
const std::string REQ_RES_SEARCH_SLICE = "search/slice";
const std::string REQ_RES_SEARCH_ALL_SLICES = "search/all/slices";
const std::string REQ_RES_REMOTE_DELETE = "remote/delete";
const std::string REQ_RES_FLOW_CATEGORY_LIST = "flow/categoryList";
const std::string REQ_RES_FLOW_CATEGORY_EVENTS = "flow/categoryEvents";
const std::string REQ_RES_UNIT_COUNTER = "unit/counter";
const std::string REQ_RES_SAME_OPERATORS_DURATION = "query/all/same/operators/duration";

// communication
const std::string REQ_RES_COMMUNICATION_OPERATOR_DETAILS = "communication/operatorDetails";
const std::string REQ_RES_COMMUNICATION_OPERATOR_LISTS = "communication/operatorLists";
const std::string REQ_RES_COMMUNICATION_BANDWIDTH = "communication/bandwidth";
const std::string REQ_RES_COMMUNICATION_DISTRIBUTION = "communication/distribution";
const std::string REQ_RES_COMMUNICATION_COMMUNICATOR = "communication/communicator";
const std::string REQ_RES_COMMUNICATION_ITERATIONS = "communication/duration/iterations";
const std::string REQ_RES_COMMUNICATION_RANKS = "communication/duration/ranks";
const std::string REQ_RES_COMMUNICATION_OPERATORNAMES = "communication/duration/operatorNames";
const std::string REQ_RES_COMMUNICATION_SORT_OP = "communication/matrix/sortOpNames";
const std::string REQ_RES_COMMUNICATION_LIST = "communication/duration/list";

// summary
const std::string REQ_RES_SUMMARY_QUERY_TOP_DATA = "summary/queryTopData";
const std::string REQ_RES_SUMMARY_STATISTIC = "summary/statistic";
const std::string REQ_RES_SUMMARY_QUERY_PARALLEL_STRATEGY = "summary/query/parallelStrategy";
const std::string REQ_RES_SUMMARY_SET_PARALLEL_STRATEGY = "summary/set/parallelStrategy";
const std::string REQ_RES_COMPUTE_DETAIL = "summary/queryComputeDetail";
const std::string REQ_RES_COMMUNICATION_DETAIL = "summary/queryCommunicationDetail";

// pipeline
const std::string REQ_RES_PIPELINE_GET_ALL_STEPS = "parallelism/pipeline/getAllSteps";
const std::string REQ_RES_PIPELINE_GET_ALL_STAGES = "parallelism/pipeline/getAllStages";
const std::string REQ_RES_PIPELINE_STAGE_BUBBLE = "parallelism/pipeline/stageAndBubbleTime";
const std::string REQ_RES_PIPELINE_RANK_BUBBLE = "parallelism/pipeline/rankAndBubbleTime";

// matrix
const std::string REQ_RES_COMMUNICATION_MATRIX_GROUP = "communication/matrix/group";
const std::string REQ_RES_COMMUNICATION_MATRIX_BANDWIDTH = "communication/matrix/bandwidthInfo";

// memory
const std::string REQ_RES_MEMORY_TYPE = "Memory/view/type";
const std::string REQ_RES_MEMORY_RESOURCE_TYPE = "Memory/view/resourceType";
const std::string REQ_RES_MEMORY_OPERATOR = "Memory/view/operator";
const std::string REQ_RES_MEMORY_VIEW = "Memory/view/memoryUsage";
const std::string REQ_RES_MEMORY_OPERATOR_MIN_MAX = "Memory/view/operator/size";
const std::string REQ_RES_MEMORY_STATIC_OP_MEMORY_GRAPH = "Memory/view/staticOpMemoryGraph";
const std::string REQ_RES_MEMORY_STATIC_OP_MEMORY_LIST = "Memory/view/staticOpMemoryList";

// Operator Request
const std::string REQ_RES_OPERATOR_CATEGORY_INFO = "operator/category";
const std::string REQ_RES_OPERATOR_COMPUTE_UNIT_INFO = "operator/compute_unit";
const std::string REQ_RES_OPERATOR_STATISTIC_INFO = "operator/statistic";
const std::string REQ_RES_OPERATOR_DETAIL_INFO = "operator/details";
const std::string REQ_RES_OPERATOR_MORE_INFO  = "operator/more_info";

// Operator Event
const std::string EVENT_PARSE_OPERATOR_STATUS = "parse/operatorCompleted";
const std::string EVENT_PARSE_OPERATOR_CLEAR = "parse/operatorClear";


// Source Request
const std::string REQ_RES_SOURCE_CODE_FILE = "source/code/file";
const std::string REQ_RES_SOURCE_API_LINE = "source/api/line";
const std::string REQ_RES_SOURCE_API_INSTRUCTIONS = "source/api/instructions";
const std::string REQ_RES_DETAILS_BASE_INFO = "source/details/baseInfo";
const std::string REQ_RES_DETAILS_COMPUTE_LOAD_INFO = "source/details/computeworkload";
const std::string REQ_RES_DETAILS_COMPUTE_MEMORY_GRAPH = "source/details/memoryGraph";
const std::string REQ_RES_DETAILS_COMPUTE_MEMORY_TABLE = "source/details/memoryTable";
const std::string REQ_RES_DETAILS_INTER_CORE_LOAD_GRAPH = "source/details/interCoreLoadAnalysis";
constexpr inline std::string_view REQ_RES_DETAILS_ROOFLINE = "source/details/roofline";

// Advisor Request/Response
const std::string REQ_RES_ADVISOR_AFFINITY_OPTIMIZER = "advisor/affinity_optimizer";
const std::string REQ_RES_ADVISOR_AFFINITY_API = "advisor/affinity_api";
const std::string REQ_RES_ADVISOR_OPERATORS_FUSION = "advisor/operator_fusion";
const std::string REQ_RES_ADVISOR_AICPU_OPERATORS = "advisor/aicpu_operator";
const std::string REQ_RES_ADVISOR_ACLNN_OPERATORS = "advisor/aclnn_operator";

// jupyter
const std::string EVENT_PARSE_JUPYTER_COMPLETED = "parse/jupyterCompleted";

const std::string KEY_BODY = "body";
#pragma endregion
} // end of namespace
}

#endif // DIC_PROTOCOL_DEFS_H