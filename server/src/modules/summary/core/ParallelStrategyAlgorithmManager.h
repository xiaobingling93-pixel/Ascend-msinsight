/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_PARALLELSTRATEGYALGORITHMMANAGER_H
#define PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_PARALLELSTRATEGYALGORITHMMANAGER_H

#include <memory>
#include <mutex>
#include <map>
#include "BaseParallelStrategyAlgorithm.h"
namespace Dic::Module::Summary {

class ParallelStrategyAlgorithmManager {
public:
    static ParallelStrategyAlgorithmManager &Instance();
    void Reset();
    bool AddOrUpdateAlgorithm(const std::string& projectName, const ParallelStrategyConfig& config,
        std::string& errMsg);
    bool DeleteAlgorithm(const std::string &projectName);
    std::shared_ptr<BaseParallelStrategyAlgorithm> GetAlgorithmByProjectName(const std::string &projectName);
    ParallelStrategyConfig GetParallelStrategyConfig(const std::string &key);
private:
    ParallelStrategyAlgorithmManager() = default;
    ~ParallelStrategyAlgorithmManager() = default;
    static bool IsSameAlgorithm(const std::string& algorithm1, const std::string& algorithm2);

    std::recursive_mutex mutex;
    std::map<std::string, std::shared_ptr<BaseParallelStrategyAlgorithm>> algorithmMap;
};
}
#endif // PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_PARALLELSTRATEGYALGORITHMMANAGER_H
