/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol defines declaration
 */

#ifndef DIC_PROTOCOL_DEFS_H
#define DIC_PROTOCOL_DEFS_H

#include <string>

namespace Dic {
namespace Protocol {
#pragma region << Scene>>
const std::string SCENE_UNKNOWN = "unknown";
const std::string SCENE_GLOBAL = "global";
const std::string SCENE_DATABASE = "database";
const std::string SCENE_TOOL = "tool";
const std::string SCENE_LOG = "log";
// mix scene
const std::string SCENE_HARMONY = "harmony";
#pragma endregion

#pragma region << Base>>
const std::string REQUEST_NAME = "request";
const std::string RESPONSE_NAME = "response";
const std::string EVENT_NAME = "event";
#pragma endregion

#pragma region << Event Name>>
const std::string EVENT_INITIALIZED = "initialized";
// harmony
const std::string EVENT_DEVICE_CHANGED = "deviceChanged";
#pragma endregion

#pragma region << Request / Response Command>>
// global
const std::string REQ_RES_TOKEN_CREATE = "token.create";
const std::string REQ_RES_TOKEN_DESTROY = "token.destroy";
const std::string REQ_RES_TOKEN_CHECK = "token.check";
const std::string REQ_RES_CONFIG_GET = "config.get";
const std::string REQ_RES_CONFIG_SET = "config.set";

// template
const std::string REQ_RES_HDC_DEVICE_LIST = "hdc.listDevice";
const std::string REQ_RES_IMPORT_ACTION = "import/action";
const std::string REQ_RES_UNIT_THREAD_TRACES = "unit/threadTraces";
const std::string REQ_RES_UNIT_THREADS = "unit/threads";
const std::string REQ_RES_UNIT_THREAD_DETAIL = "unit/threadDetail";
const std::string REQ_RES_UNIT_FLOW_NAME = "unit/flowName";
const std::string REQ_RES_UNIT_FLOW = "unit/flow";
const std::string REQ_RES_RESET_WINDOW = "reset/window";
const std::string REQ_RES_UNIT_CHART = "unit/chart";
#pragma endregion
} // end of namespace
}

#endif // DIC_PROTOCOL_DEFS_H