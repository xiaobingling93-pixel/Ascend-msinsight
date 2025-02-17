/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_METADATACACHEMANAGER_H
#define PROFILER_SERVER_METADATACACHEMANAGER_H
#include <map>
#include <string>
#include <optional>
#include "DomainObject.h"

namespace Dic::Module::Timeline {
class MetaDataCacheManager {
public:
    static MetaDataCacheManager &Instance()
    {
        static MetaDataCacheManager metaDataCacheManager;
        return metaDataCacheManager;
    }

    void Clear();
    void AddParallelGroupInfo(const std::vector<ParallelGroupInfo>& parallelGroupInfoList);
    std::optional<ParallelGroupInfo> GetParallelGroupInfo(const std::string& group);
private:
    MetaDataCacheManager() = default;
    ~MetaDataCacheManager() = default;
    std::map<std::string, ParallelGroupInfo> hcclGroupInfoMap;
};
}
#endif // PROFILER_SERVER_METADATACACHEMANAGER_H
