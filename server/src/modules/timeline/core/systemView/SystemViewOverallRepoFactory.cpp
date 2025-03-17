/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "SystemViewOverallDbRepo.h"
#include "SystemViewOverallTextRepo.h"
#include "SystemViewOverallRepoFactory.h"
using namespace Dic::Server;
namespace Dic::Module::Timeline {
SystemViewOverallRepoFactory::SystemViewOverallRepoFactory()
{
    systemViewOverallRepoMap.emplace(DataType::TEXT, std::make_shared<SystemViewOverallTextRepo>());
    systemViewOverallRepoMap.emplace(DataType::DB, std::make_shared<SystemViewOverallDbRepo>());
}

SystemViewOverallRepoFactory::~SystemViewOverallRepoFactory()
{
    systemViewOverallRepoMap.clear();
}

void SystemViewOverallRepoFactory::Reset()
{
    systemViewOverallRepoMap.clear();
}

std::shared_ptr<SystemViewOverallRepoInterface> SystemViewOverallRepoFactory::GetSystemViewOverallRepo(DataType type)
{
    if (systemViewOverallRepoMap.find(type) == systemViewOverallRepoMap.end()) {
        ServerLog::Error("Can't find repository for system view overall.");
        return nullptr;
    }
    return systemViewOverallRepoMap.at(type);
}
}