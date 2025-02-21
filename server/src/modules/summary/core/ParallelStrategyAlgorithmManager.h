/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_PARALLELSTRATEGYALGORITHMMANAGER_H
#define PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_PARALLELSTRATEGYALGORITHMMANAGER_H

#include <memory>
#include <mutex>
#include <map>
#include "BaseParallelStrategyAlgorithm.h"
namespace Dic::Module {

const std::string MEGATRON_DP_PP = "megatron-lm(tp-cp-ep-dp-pp)";
const std::string MEGATRON_PP_DP = "megatron-lm(tp-cp-pp-ep-dp)";

class ParallelStrategyAlgorithmManager {
public:
    static ParallelStrategyAlgorithmManager &Instance();
    void Reset();
    void AddOrUpdateAlgorithm(const std::string& projectName,
        const std::shared_ptr<BaseParallelStrategyAlgorithm>& algPtr, const ParallelStrategyConfig& config);
    bool DeleteAlgorithm(const std::string &projectName);
    std::shared_ptr<BaseParallelStrategyAlgorithm> GetAlgorithmByProjectName(const std::string &projectName);
    ParallelStrategyConfig GetParallelStrategyConfig(const std::string &key);
private:
    ParallelStrategyAlgorithmManager() = default;
    ~ParallelStrategyAlgorithmManager() = default;

    std::recursive_mutex mutex;
    std::map<std::string, std::shared_ptr<BaseParallelStrategyAlgorithm>> algorithmMap;
};
}
#endif // PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_PARALLELSTRATEGYALGORITHMMANAGER_H
