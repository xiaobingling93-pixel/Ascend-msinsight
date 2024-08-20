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

    bool IsSelectBaseline();
    void ResetBaseline();
    bool InitBaselineData(const std::string &projectName, const std::string &filePath, std::string &errorMsg,
                          std::string &rankId);
    std::string GetBaselineId();
    static bool IsBaselineId(const std::string &rankId);

private:
    BaselineManager() = default;
    ~BaselineManager() = default;

    int64_t parseFileId = -1;
    const inline static std::string baselineMark = "baseline";
    std::string status;
    std::recursive_mutex mutex;
};
}
}
}

#endif // PROFILER_SERVER_BASELINEMANAGER_H
