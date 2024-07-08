/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#ifndef PROFILER_SERVER_FILEDEF_H
#define PROFILER_SERVER_FILEDEF_H

#include <string>
#include <vector>
namespace Dic {

const std::string NAME = "Name";
const std::string SIZE = "Size(KB)";
const std::string DURATION = "Duration(us)";

const std::string COMPONENT = "Component";
const std::string TIMESTAMP = "Timestamp(us)";
const std::string DEVICETYPE = "Device Type";
const std::string DEVICE_ID = "Device_id";
const std::string DEVICE = "Device";
const std::string STEP_ID = "Step Id";
const std::string MODEL_ID = "Model ID";

const std::string ASCEND_PROFILER_OUTPUT = "ASCEND_PROFILER_OUTPUT";
const std::string MINDSTUDIO_PROFILER_OUTPUT = "mindstudio_profiler_output";
const std::string CLUSTER_ANALYSIS_OUTPUT = "cluster_analysis_output";

const std::string DEVICE_DIR_PREFIX = "device_";
const std::string DEVICE_DIR_REG = R"(^device_\d+$)";

const std::string PROFILER_INFO_FILE_PREFIX = "profiler_info_";
const std::string PROFILER_INFO_FILE_REG = R"(^profiler_info_\d+\.json$)";

const std::string MSPROF_SLICE_FILE_REG = R"(^msprof_slice_[0-9_]+\.json$)";

const std::string JSON_FILE_SUFFIX = ".json";
const std::string DB_FILE_SUFFIX = ".db";

const std::string DATABASE_FILE_NAME = "mindstudio_insight_data.db";

const std::string SLICE_STR = "_slice";

const long long MAX_FILE_SIZE_2G = static_cast<long long>(1024 * 1024 * 1024) * 2;
const long long MAX_FILE_SIZE_10G = static_cast<long long>(1024 * 1024 * 1024) * 10;
const int KB_TO_MB = 1024;
}

#endif // PROFILER_SERVER_FILEDEF_H
