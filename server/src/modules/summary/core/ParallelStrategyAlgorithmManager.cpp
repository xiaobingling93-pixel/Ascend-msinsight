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

bool ParallelStrategyAlgorithmManager::AddAlgorithm(const std::string& projectName,
                                                    const std::shared_ptr<BaseParallelStrategyAlgorithm>& algPtr)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    // 若已存在该project，则添加失败
    auto it = algorithmMap.find(projectName);
    if (it != algorithmMap.end()) {
        return false;
    }
    algorithmMap.emplace(projectName, algPtr);
    return true;
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
    const std::string &projectName, std::string &err)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (algorithmMap.count(projectName) == 0) {
        err = "Failed to get algorithm by project name";
        return nullptr;
    }
    ServerLog::Info("Success to get algorithm by project name.");
    return algorithmMap.at(projectName);
}

}