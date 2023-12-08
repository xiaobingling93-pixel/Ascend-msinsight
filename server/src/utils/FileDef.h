/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#ifndef PROFILER_SERVER_FILEDEF_H
#define PROFILER_SERVER_FILEDEF_H

#include <string>
#include <vector>
namespace Dic {

const std::string NAME = "Name";
const std::string ALLOCATION_TIME = "Allocation Time(us)";
const std::string RELEASE_TIME = "Release Time(us)";
const std::string SIZE = "Size(KB)";
const std::string DURATION = "Duration(us)";
const std::vector<std::string> OPERATOR_CSV = {NAME, ALLOCATION_TIME, RELEASE_TIME, SIZE, DURATION};
const std::vector<std::string> OPERATOR_CSV_MSPROF = {NAME, ALLOCATION_TIME, SIZE, DURATION};

const std::string COMPONENT = "Component";
const std::string TIMESTAMP = "Timestamp(us)";
const std::string DEVICETYPE = "Device Type";
const std::string DEVICE_ID = "Device_id";
const std::string DEVICE = "Device";
const std::string TOTAL_ALLOCATED_MB = "Total Allocated(MB)";
const std::string TOTAL_RESERVED_MB = "Total Reserved(MB)";
const std::string TOTAL_ALLOCATED_KB = "Total Allocated(KB)";
const std::string TOTAL_RESERVED_KB = "Total Reserved(KB)";
const std::vector<std::string> RECORD_CSV = {COMPONENT, TIMESTAMP, DEVICETYPE, TOTAL_ALLOCATED_MB, TOTAL_RESERVED_MB};
const std::vector<std::string> RECORD_CSV_MSPROF = {
    COMPONENT, TIMESTAMP, DEVICE, TOTAL_ALLOCATED_KB, TOTAL_RESERVED_KB
};

const std::string ASCEND_PROFILER_OUTPUT = "ASCEND_PROFILER_OUTPUT";
const std::string MINDSTUDIO_PROFILER_OUTPUT = "mindstudio_profiler_output";

const long long MAX_FILE_SIZE_2G = (long long)(1024 * 1024 * 1024) * 2;
const long long MAX_FILE_SIZE_10G = (long long)(1024 * 1024 * 1024) * 10;
const int KB_TO_MB = 1024;
}

#endif // PROFILER_SERVER_FILEDEF_H
