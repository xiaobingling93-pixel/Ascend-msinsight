/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "ServerLog.h"
#include "StringUtil.h"
#include "MegatronParallelStrategyAlgorithm.h"
#include "MindSpeedParallelStrategyAlgorithm.h"
#include "ParallelStrategyAlgorithmManager.h"
using namespace Dic::Server;
namespace Dic::Module::Summary {
ParallelStrategyAlgorithmManager &ParallelStrategyAlgorithmManager::Instance()
{
    static ParallelStrategyAlgorithmManager instance;
    return instance;
}

void ParallelStrategyAlgorithmManager::Reset()
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    algorithmMap.clear();
}

bool ParallelStrategyAlgorithmManager::IsSameAlgorithm(const std::string& algorithm1, const std::string& algorithm2)
{
    if (algorithm1 == algorithm2 || (StringUtil::Contains(algorithm1, MEGATRON_ALG) &&
        StringUtil::Contains(algorithm2, MEGATRON_ALG))) {
        return true;
    }
    return false;
}

bool ParallelStrategyAlgorithmManager::AddOrUpdateAlgorithm(const std::string& projectName,
    const ParallelStrategyConfig& config, std::string& errMsg)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    auto it = algorithmMap.find(projectName);
    if (it != algorithmMap.end()) {
        ParallelStrategyConfig oldConfig = algorithmMap.at(projectName)->GetStrategyConfig();
        // 若已存在该project，且算法类别相同, 则更新config
        if (IsSameAlgorithm(oldConfig.algorithm, config.algorithm)) {
            algorithmMap.at(projectName)->ClearStrategyConfigCache();
            algorithmMap.at(projectName)->SetStrategyConfig(config);
            Server::ServerLog::Info("Algorithm already exist. Update parallel strategy config for this program.");
            return true;
        }
        DeleteAlgorithm(projectName);
    }
    // 若不存在, 或算法类不同，则添加相应算法类
    if (StringUtil::Contains(StringUtil::ToLower(config.algorithm), MEGATRON_ALG)) {
        algorithmMap.emplace(projectName, std::make_shared<MegatronParallelStrategyAlgorithm>());
    } else if (StringUtil::Contains(StringUtil::ToLower(config.algorithm), MINDSPEED_ALG)) {
        algorithmMap.emplace(projectName, std::make_shared<MindSpeedParallelStrategyAlgorithm>());
    } else {
        errMsg = "Failed to add algorithm to manager when set parallel config. Unexpected algorithm.";
        return false;
    }
    algorithmMap.at(projectName)->SetStrategyConfig(config);
    Server::ServerLog::Info("Success to add algorithm to parallel strategy manager.");
    return true;
}

ParallelStrategyConfig ParallelStrategyAlgorithmManager::GetParallelStrategyConfig(const std::string &key)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    ParallelStrategyConfig config;
    auto it = algorithmMap.find(key);
    if (it != algorithmMap.end()) {
        config = algorithmMap.at(key)->GetStrategyConfig();
    }
    Server::ServerLog::Warn("Fail to get parallel strategy config.");
    return config;
}

bool ParallelStrategyAlgorithmManager::DeleteAlgorithm(const std::string &projectName)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    // 若不存在该project，则删除失败
    auto it = algorithmMap.find(projectName);
    if (it == algorithmMap.end()) {
        return false;
    }
    algorithmMap.erase(projectName);
    return true;
}

std::shared_ptr<BaseParallelStrategyAlgorithm> ParallelStrategyAlgorithmManager::GetAlgorithmByProjectName(
    const std::string &projectName)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (algorithmMap.count(projectName) == 0) {
        return nullptr;
    }
    ServerLog::Info("Success to get algorithm by project name.");
    return algorithmMap.at(projectName);
}
}