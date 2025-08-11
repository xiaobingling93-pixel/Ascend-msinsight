// Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
#ifndef PROFILER_SERVER_RLMSTXCONFIGREADER_H
#define PROFILER_SERVER_RLMSTXCONFIGREADER_H

#include <string>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include "JsonUtil.h"
#include "RLDomainObject.h"

namespace Dic::Module::RL {
class RLMstxConfigReader {
public:
    explicit RLMstxConfigReader();

    std::vector<RLMstxConfig> ReadConfigFile();

    void SetConfigPath(const std::string &path);

private:
    bool ConfigFormatValid(document_t &doc);

    static bool MstxConfigCheck(const json_t &mstxConfigJson);

    static bool TaskConfigCheck(const json_t &taskConfJson);

    static bool MicroBatchConfigCheck(const json_t &microBatchJson);

    RLMstxConfig ParseMstxConfig(const json_t &config);

    TaskConfig ParseTaskConfig(const json_t &taskConfJson);

    MicroBatchConfig ParseMicroBatchConfig(const json_t &microBatchConfJson);

    std::string configPath;
};

} // Dic
// Module
// RL

#endif // PROFILER_SERVER_RLMSTXCONFIGREADER_H
