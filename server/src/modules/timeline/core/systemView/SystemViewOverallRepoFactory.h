/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_SYSTEMVIEWOVERALLREPOFACTORY_H
#define PROFILER_SERVER_SYSTEMVIEWOVERALLREPOFACTORY_H
#include <memory>
#include "DataBaseManager.h"
#include "SystemViewOverallRepoInterface.h"
namespace Dic::Module::Timeline {
class SystemViewOverallRepoFactory {
public:
    static std::shared_ptr<SystemViewOverallRepoFactory> Instance()
    {
        static std::shared_ptr<SystemViewOverallRepoFactory> instance =
            std::make_shared<SystemViewOverallRepoFactory>();
        return instance;
    }
    SystemViewOverallRepoFactory();
    SystemViewOverallRepoFactory(const SystemViewOverallRepoFactory &) = delete;
    SystemViewOverallRepoFactory &operator=(const SystemViewOverallRepoFactory &) = delete;
    SystemViewOverallRepoFactory(SystemViewOverallRepoFactory &&) = delete;
    SystemViewOverallRepoFactory &operator=(SystemViewOverallRepoFactory &&) = delete;
    ~SystemViewOverallRepoFactory();
    void Reset();
    std::shared_ptr<SystemViewOverallRepoInterface> GetSystemViewOverallRepo(DataType type);
private:
    std::unordered_map<DataType, std::shared_ptr<SystemViewOverallRepoInterface>> systemViewOverallRepoMap;
};
}

#endif // PROFILER_SERVER_SYSTEMVIEWOVERALLREPOFACTORY_H
