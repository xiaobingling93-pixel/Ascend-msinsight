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

const std::string COMPONENT = "Component";
const std::string TIMESTAMP = "Timestamp(us)";
const std::string DEVICE = "Device Type";
const std::string TOTAL_ALLOCATED = "Total Allocated(MB)";
const std::string TOTAL_RESERVED = "Total Reserved(MB)";
const std::vector<std::string> RECORD_CSV = {COMPONENT, TIMESTAMP, DEVICE, TOTAL_ALLOCATED, TOTAL_RESERVED};

const long long MAX_FILE_SIZE_2G = (long long)(1024 * 1024 * 1024) * 2;
const long long MAX_FILE_SIZE_10G = (long long)(1024 * 1024 * 1024) * 10;
}

#endif // PROFILER_SERVER_FILEDEF_H
