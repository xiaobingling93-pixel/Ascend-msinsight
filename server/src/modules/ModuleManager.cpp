/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#include "ServerLog.h"
#include "GlobalModule.h"
#include "TimelineModule.h"
#include "SummaryModule.h"
#include "MemoryModule.h"
#include "CommunicationModule.h"
#include "ModuleManager.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
ModuleManager &ModuleManager::Instance()
{
    static ModuleManager instance;
    return instance;
}

ModuleManager::ModuleManager()
{
    Register();
}

ModuleManager::~ModuleManager()
{
    UnRegister();
}

void ModuleManager::Register()
{
    std::unique_lock<std::mutex> lock(mutex);
    moduleMap.clear();
    std::unique_ptr<GlobalModule> global = std::make_unique<GlobalModule>();
    std::unique_ptr<TimelineModule> timelineModule = std::make_unique<TimelineModule>();
    std::unique_ptr<SummaryModule> summaryModule = std::make_unique<SummaryModule>();
    std::unique_ptr<MemoryModule> memoryModule = std::make_unique<MemoryModule>();
    std::unique_ptr<CommunicationModule> communicationModule = std::make_unique<CommunicationModule>();
    global->RegisterRequestHandlers();
    timelineModule->RegisterRequestHandlers();
    memoryModule->RegisterRequestHandlers();
    summaryModule->RegisterRequestHandlers();
    communicationModule->RegisterRequestHandlers();
    moduleMap.emplace(ModuleType::GLOBAL, std::move(global));
    moduleMap.emplace(ModuleType::TIMELINE, std::move(timelineModule));
    moduleMap.emplace(ModuleType::SUMMARY, std::move(summaryModule));
    moduleMap.emplace(ModuleType::MEMORY, std::move(memoryModule));
    moduleMap.emplace(ModuleType::COMMUNICATION, std::move(communicationModule));
}

void ModuleManager::UnRegister()
{
    std::unique_lock<std::mutex> lock(mutex);
    moduleMap.clear();
}

void ModuleManager::OnDispatchModuleRequest(std::unique_ptr<Request> request)
{
    auto moduleName = request->moduleName;
    if (moduleMap.count(moduleName) == 0) {
        ServerLog::Error("Failed to dispatch to module, module = ", ENUM_TO_STR(moduleName).value(),
            ", token = ", StringUtil::AnonymousString(request->token), ", command = ", request->command);
        return;
    }
    ServerLog::Info("Dispatch to module, module = ", ENUM_TO_STR(moduleName).value(),
        ", token = ", StringUtil::AnonymousString(request->token), ", command = ", request->command,
        ", request id = ", request->id);
    moduleMap.at(moduleName)->OnRequest(std::move(request));
}
} // end of namespace Module
} // end of namespace Dic