/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ServerLog.h"
#include "JsonUtil.h"
#include "GlobalProtocol.h"
#include "TimelineProtocol.h"
#include "ProtocolEnumUtil.h"
#include "MemoryProtocol.h"
#include "SummaryProtocol.h"
#include "ProtocolManager.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Server;
ProtocolManager::ProtocolManager()
{
    Register();
}

ProtocolManager::~ProtocolManager()
{
    UnRegister();
}

void ProtocolManager::Register()
{
    std::unique_lock<std::mutex> lock(mutex);
    protocolMap.clear();
    auto globalProtocol = std::make_unique<GlobalProtocol>();
    globalProtocol->Register();
    auto timelineProtocol = std::make_unique<TimelineProtocol>();
    timelineProtocol->Register();
    auto memoryProtocol = std::make_unique<MemoryProtocol>();
    memoryProtocol->Register();
    auto summaryProtocol = std::make_unique<SummaryProtocol>();
    summaryProtocol->Register();
    protocolMap.emplace(ModuleType::GLOBAL, std::move(globalProtocol));
    protocolMap.emplace(ModuleType::TIMELINE, std::move(timelineProtocol));
    protocolMap.emplace(ModuleType::MEMORY, std::move(memoryProtocol));
    protocolMap.emplace(ModuleType::SUMMARY, std::move(summaryProtocol));
}

void ProtocolManager::UnRegister()
{
    std::unique_lock<std::mutex> lock(mutex);
    protocolMap.clear();
}

std::unique_ptr<Request> ProtocolManager::FromJson(const std::string &requestStr, std::string &error)
{
    json_t requestJson;
    try {
        requestJson = json_t::parse(requestStr);
    } catch (json_t::parse_error &) {
        ServerLog::Warn("Failed to parse request json. ", requestStr);
        return nullptr;
    }
    if (!JsonUtil::IsJsonKeyValid(requestJson, "moduleName")) {
        ServerLog::Warn("Failed to get module type from json. ", requestStr);
        return nullptr;
    }
    auto moduleName = STR_TO_ENUM<ModuleType>(requestJson["moduleName"]);
    if (!moduleName.has_value()) {
        ServerLog::Warn("Failed to get module type from json. ", requestStr);
        return nullptr;
    }
    std::unique_lock<std::mutex> lock(mutex);
    if (protocolMap.count(moduleName.value()) == 0) {
        ServerLog::Warn("Failed to get request module protocol. module type:", static_cast<int>(moduleName.value()));
        return nullptr;
    }
    return protocolMap.at(moduleName.value())->FromJson(requestJson, error);
}

std::optional<json_t> ProtocolManager::ToJson(const Response &response, std::string &error)
{
    auto moduleName = response.moduleName;
    std::unique_lock<std::mutex> lock(mutex);
    if (protocolMap.count(moduleName) == 0) {
        ServerLog::Warn("Failed to get response module protocol. module type:", static_cast<int>(moduleName));
        return nullptr;
    }
    return protocolMap.at(moduleName)->ToJson(response, error);
}

std::optional<json_t> ProtocolManager::ToJson(const Event &event, std::string &error)
{
    auto moduleName = event.moduleName;
    std::unique_lock<std::mutex> lock(mutex);
    if (protocolMap.count(moduleName) == 0) {
        ServerLog::Warn("Failed to get event module protocol. module type:", static_cast<int>(moduleName));
        return nullptr;
    }
    return protocolMap.at(moduleName)->ToJson(event, error);
}
}
}