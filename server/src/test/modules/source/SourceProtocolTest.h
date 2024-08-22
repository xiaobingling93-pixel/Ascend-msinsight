/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SOURCEPROTOCOLTEST_H
#define PROFILER_SERVER_SOURCEPROTOCOLTEST_H

#include "string"

const std::string JSON = R"()";
const std::string TO_INTER_CORE_LOAD_GRAPH_JSON = R"(
{
  "id": 288,
  "moduleName": "source",
  "type": "request",
  "command": "source/details/interCoreLoadAnalysis"
}
)";
const std::string TO_MEMORY_TABLE_REQ_JSON = R"(
{
  "id": 288,
  "moduleName": "source",
  "type": "request",
  "command": "source/details/memoryTable",
  "params": {
    "blockId": "0",
    "showAs": "request"
  }
}
)";
const std::string TO_MEMORY_GRAPH_REQ_JSON = R"(
{
  "id": 287,
  "moduleName": "source",
  "type": "request",
  "command": "source/details/memoryGraph",
  "params": {
    "blockId": "0",
    "showAs": "request"
  }
}
)";
const std::string TO_LOAD_INFO_REQ_JSON = R"(
{
  "id": 286,
  "moduleName": "source",
  "type": "request",
  "command": "source/details/computeworkload"
}
)";
const std::string TO_BASE_INFO_REQ_JSON = R"(
{
  "id": 281,
  "moduleName": "source",
  "type": "request",
  "command": "source/details/baseInfo"
}
)";
const std::string TO_API_INSTR_REQ_JSON = R"(
{
  "id": 4777,
  "moduleName": "source",
  "type": "request",
  "command": "source/api/instructions"
}
)";
const std::string TO_API_LINE_REQ_JSON = R"(
{
  "id": 4776,
  "moduleName": "source",
  "type": "request",
  "command": "source/api/line",
  "params": {
    "sourceName": "xxx.cpp",
    "coreName": "core0.cubecore0"
  }
}
)";
const std::string TO_CODE_FILE_REQ_JSON = R"(
{
  "id": 4772,
  "moduleName": "source",
  "type": "request",
  "command": "source/code/file",
  "params": {
    "sourceName": "/home/xxx.cpp"
  }
}
)";

#endif // PROFILER_SERVER_SOURCEPROTOCOLTEST_H
