/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "MetaDataCacheManager.h"

namespace Dic::Module::Timeline {
void MetaDataCacheManager::Clear()
{
    hcclGroupInfoMap.clear();
}

void MetaDataCacheManager::AddParallelGroupInfo(const std::vector<ParallelGroupInfo>& parallelGroupInfoList)
{
    for (const auto &item: parallelGroupInfoList) {
        if (item.group.empty()) {
            continue;
        }
        hcclGroupInfoMap[item.group] = item;
    }
}

std::optional<ParallelGroupInfo> MetaDataCacheManager::GetParallelGroupInfo(const std::string &group)
{
    auto res = hcclGroupInfoMap.find(group);
    if (res != hcclGroupInfoMap.end()) {
        return {res->second};
    }
    return {};
}
}