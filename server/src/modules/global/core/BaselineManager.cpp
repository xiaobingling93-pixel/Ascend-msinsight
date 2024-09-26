/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <string>
#include "ParserStatusManager.h"
#include "BaselineManager.h"
namespace Dic::Module::Global {
BaselineManager &BaselineManager::Instance()
{
    static BaselineManager instance;
    return instance;
}

std::string BaselineManager::GetBaselineId()
{
    uint64_t count = 0;
    const uint64_t maxCount = 180;
    while (true) {
        if (count > maxCount) {
            return "";
        }
        {
            std::shared_lock<std::shared_mutex> sharedLock(sharedMutex);
            if (!std::empty(baselineRankId) &&
                Protocol::ParserStatusManager::Instance().IsKernelAndMemoryFinished(baselineRankId)) {
                return baselineRankId;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ++count;
    }
}

bool BaselineManager::IsBaselineId(const std::string &rankId)
{
    std::shared_lock<std::shared_mutex> sharedLock(sharedMutex);
    if (std::empty(rankId) || std::empty(baselineRankId)) {
        return false;
    }
    return baselineRankId == rankId;
}

void BaselineManager::SetBaselineInfo(const BaselineInfo &baselineInfo)
{
    std::unique_lock<std::shared_mutex> uniqueLock(sharedMutex);
    baselineRankId = baselineInfo.rankId;
    baselineHost = baselineInfo.host;
    baselineCardName = baselineInfo.cardName;
}

void BaselineManager::Reset()
{
    std::unique_lock<std::shared_mutex> uniqueLock(sharedMutex);
    baselineRankId.clear();
    baselineHost.clear();
    baselineCardName.clear();
}
}
