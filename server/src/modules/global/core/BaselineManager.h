/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_BASELINEMANAGER_H
#define PROFILER_SERVER_BASELINEMANAGER_H

#include <string>
#include <vector>
#include "SystemMemoryDatabaseDef.h"


namespace Dic {
namespace Module {
namespace Global {
class BaselineManager {
public:
    static BaselineManager &Instance();
    BaselineManager(const BaselineManager &) = delete;
    BaselineManager &operator=(const BaselineManager &) = delete;
    BaselineManager(BaselineManager &&) = delete;
    BaselineManager &operator=(BaselineManager &&) = delete;
    std::string GetBaselineId();
    bool IsBaselineId(const std::string &rankId);
    void SetBaselineInfo(const BaselineInfo &baselineInfo);
    void Reset();

private:
    BaselineManager() = default;
    ~BaselineManager() = default;
    std::string baselineRankId;
    std::string baselineHost;
    std::string baselineCardName;
    std::shared_mutex sharedMutex;
};
}
}
}

#endif // PROFILER_SERVER_BASELINEMANAGER_H
