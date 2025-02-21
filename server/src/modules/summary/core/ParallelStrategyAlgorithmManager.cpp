/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "ServerLog.h"
#include "StringUtil.h"
#include "ParallelStrategyAlgorithmManager.h"
using namespace Dic::Server;
namespace Dic::Module {
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

void ParallelStrategyAlgorithmManager::AddOrUpdateAlgorithm(const std::string& projectName,
    const std::shared_ptr<BaseParallelStrategyAlgorithm>& algPtr, const ParallelStrategyConfig& config)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    // 若已存在该project，则更新config
    auto it = algorithmMap.find(projectName);
    if (it != algorithmMap.end()) {
        algorithmMap.at(projectName)->ClearStrategyConfigCache();
        algorithmMap.at(projectName)->SetStrategyConfig(config);
        Server::ServerLog::Info("Algorithm already exist. Update parallel strategy config for this program.");
        return;
    }
    algorithmMap.emplace(projectName, algPtr);
    algorithmMap.at(projectName)->SetStrategyConfig(config);
    Server::ServerLog::Info("Success to add algorithm to parallel strategy manager.");
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