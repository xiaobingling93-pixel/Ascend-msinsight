/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SOURCEPROTOCOLTEST_H
#define PROFILER_SERVER_SOURCEPROTOCOLTEST_H

#include "string"

const std::string JSON = R"()";
const std::string TO_API_INSTR_REQ_JSON = R"(
{
  "id": 4777,
  "moduleName": "source",
  "type": "request",
  "command": "source/api/instructions",
  "params": {
    "token": "eQoptm^_fwSPlX^X"
  }
}
)";
const std::string TO_API_LINE_REQ_JSON = R"(
{
  "id": 4776,
  "moduleName": "source",
  "type": "request",
  "command": "source/api/line",
  "params": {
    "token": "eQoptm^_fwSPlX^X",
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
