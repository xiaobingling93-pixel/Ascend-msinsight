/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */
#include "pch.h"
#include "GlobalProtocol.h"
#include "TimelineProtocol.h"
#include "ProtocolEnumUtil.h"
#include "MemoryProtocol.h"
#include "SummaryProtocol.h"
#include "CommunicationProtocol.h"
#include "OperatorProtocol.h"
#include "SourceProtocol.h"
#include "AdvisorProtocolUtil.h"
#include "JupyterProtocol.h"
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
    auto communicationProtocol = std::make_unique<CommunicationProtocol>();
    communicationProtocol->Register();
    auto operatorProtocol = std::make_unique<OperatorProtocol>();
    operatorProtocol->Register();
    auto sourceProtocol = std::make_unique<SourceProtocol>();
    sourceProtocol->Register();
    auto advisorProtocol = std::make_unique<AdvisorProtocolUtil>();
    advisorProtocol->Register();
    auto jupyterProtocol = std::make_unique<JupyterProtocol>();
    jupyterProtocol->Register();
    protocolMap.emplace(ModuleType::GLOBAL, std::move(globalProtocol));
    protocolMap.emplace(ModuleType::TIMELINE, std::move(timelineProtocol));
    protocolMap.emplace(ModuleType::MEMORY, std::move(memoryProtocol));
    protocolMap.emplace(ModuleType::SUMMARY, std::move(summaryProtocol));
    protocolMap.emplace(ModuleType::COMMUNICATION, std::move(communicationProtocol));
    protocolMap.emplace(ModuleType::OPERATOR, std::move(operatorProtocol));
    protocolMap.emplace(ModuleType::SOURCE, std::move(sourceProtocol));
    protocolMap.emplace(ModuleType::ADVISOR, std::move(advisorProtocol));
    protocolMap.emplace(ModuleType::JUPYTER, std::move(jupyterProtocol));
}

void ProtocolManager::UnRegister()
{
    std::unique_lock<std::mutex> lock(mutex);
    protocolMap.clear();
}

std::unique_ptr<Request> ProtocolManager::FromJson(const std::string &requestStr, std::string &error)
{
    auto requestJson = JsonUtil::TryParse(requestStr, error);
    if (!requestJson.has_value()) {
        ServerLog::Warn("Failed to parse request json. ", requestStr);
        return nullptr;
    }
    if (!JsonUtil::IsJsonKeyValid(requestJson.value(), "moduleName")) {
        ServerLog::Warn(R"(Json Key "moduleName" is invaild, request=)", requestStr);
        return nullptr;
    }
    auto moduleName = STR_TO_ENUM<ModuleType>(JsonUtil::GetString(requestJson.value(), "moduleName"));
    if (!moduleName.has_value()) {
        ServerLog::Warn("Unknown module name, request=", requestStr);
        return nullptr;
    }
    std::unique_lock<std::mutex> lock(mutex);
    if (protocolMap.count(moduleName.value()) == 0) {
        ServerLog::Warn("Failed to get request module protocol. module type:", static_cast<int>(moduleName.value()));
        return nullptr;
    }
    return protocolMap.at(moduleName.value())->FromJson(requestJson.value(), error);
}

std::optional<document_t> ProtocolManager::ToJson(const Response &response, std::string &error)
{
    auto moduleName = response.moduleName;
    std::unique_lock<std::mutex> lock(mutex);
    if (protocolMap.count(moduleName) == 0) {
        ServerLog::Warn("Failed to get response module protocol. module type:", static_cast<int>(moduleName));
        return std::nullopt;
    }
    return protocolMap.at(moduleName)->ToJson(response, error);
}

std::optional<document_t> ProtocolManager::ToJson(const Event &event, std::string &error)
{
    auto moduleName = event.moduleName;
    std::unique_lock<std::mutex> lock(mutex);
    if (protocolMap.count(moduleName) == 0) {
        ServerLog::Warn("Failed to get event module protocol. module type:", static_cast<int>(moduleName));
        return std::nullopt;
    }
    return protocolMap.at(moduleName)->ToJson(event, error);
}
}
}