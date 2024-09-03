/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <string>
#include "BaselineManager.h"
namespace Dic::Module::Global {
BaselineManager &BaselineManager::Instance()
{
    static BaselineManager instance;
    return instance;
}

std::string BaselineManager::GetBaselineId()
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    return baselineRankId;
}

bool BaselineManager::IsBaselineId(const std::string &rankId)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (std::empty(rankId) || std::empty(baselineRankId)) {
        return false;
    }
    return baselineRankId == rankId;
}

void BaselineManager::SetBaselineInfo(const BaselineInfo &baselineInfo)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    baselineRankId = baselineInfo.rankId;
    baselineHost = baselineInfo.host;
    baselineCardName = baselineInfo.cardName;
}

void BaselineManager::Reset()
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    baselineRankId.clear();
    baselineHost.clear();
    baselineCardName.clear();
}
}
