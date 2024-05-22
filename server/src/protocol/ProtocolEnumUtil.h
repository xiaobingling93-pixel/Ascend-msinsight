/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol enum utility declaration
 */

#ifndef DIC_PROTOCOL_ENUM_UTIL_H
#define DIC_PROTOCOL_ENUM_UTIL_H

#include <map>
#include <string>
#include <optional>

#include "ProtocolDefs.h"
#include "ProtocolEnum.h"
#include "ProtocolMessage.h"

namespace Dic {
namespace Protocol {
#pragma region << Enum String Map>>

template <typename ENUM> using EnumStrMap = std::map<ENUM, std::string>;

const EnumStrMap<ProtocolMessage::Type> PROTOCOL_MESSAGE_TYPE_ES = { { ProtocolMessage::Type::REQUEST, REQUEST_NAME },
                                                                     { ProtocolMessage::Type::RESPONSE, RESPONSE_NAME },
                                                                     { ProtocolMessage::Type::EVENT, EVENT_NAME } };

const EnumStrMap<Protocol::ModuleType> MODULE_TYPE_ES = {{Protocol::ModuleType::UNKNOWN,  MODULE_UNKNOWN },
                                                         {Protocol::ModuleType::GLOBAL,   MODULE_GLOBAL },
                                                         {Protocol::ModuleType::TIMELINE, MODULE_TIMELINE },
                                                         {Protocol::ModuleType::SUMMARY, MODULE_SUMMARY },
                                                         {Protocol::ModuleType::COMMUNICATION, MODULE_COMMUNICATION },
                                                         {Protocol::ModuleType::MEMORY, MODULE_MEMORY },
                                                         {Protocol::ModuleType::OPERATOR, MODULE_OPERATOR },
                                                         {Protocol::ModuleType::SOURCE, MODULE_SOURCE },
                                                         {Protocol::ModuleType::ADVISOR, MODULE_ADVISOR },
                                                         {Protocol::ModuleType::JUPYTER, MODULE_JUPYTER}};

const EnumStrMap<Protocol::LinkType> LINK_TYPE_ES = { { Protocol::LinkType::WEBSOCKET, "websocket" },
                                                      { Protocol::LinkType::SOCKET, "socket" } };

const EnumStrMap<PROCESS_TYPE> PROCESS_TYPE_ES = { { PROCESS_TYPE::ASCEND_HARDWARE, "ASCEND HARDWARE" },
                                                   { PROCESS_TYPE::HCCL, "HCCL" },
                                                   { PROCESS_TYPE::OVERLAP_ANALYSIS, "OVERLAP_ANALYSIS"},
                                                   { PROCESS_TYPE::CANN_API, "CANN_API" },
                                                   { PROCESS_TYPE::API, "PYTORCH_API" },
                                                   { PROCESS_TYPE::LLC, "LLC" },
                                                   { PROCESS_TYPE::PCIE, "PCIE" },
                                                   { PROCESS_TYPE::ROCE, "RoCE" },
                                                   { PROCESS_TYPE::ROH, "RoH" },
                                                   { PROCESS_TYPE::NIC, "NIC" },
                                                   { PROCESS_TYPE::SAMPLE_PMU, "SAMPLE_PMU_TIMELINE" },
                                                   { PROCESS_TYPE::HCCS, "HCCS" },
                                                   { PROCESS_TYPE::DDR, "DDR" },
                                                   { PROCESS_TYPE::ACC_PMU, "ACC_PMU" },
                                                   { PROCESS_TYPE::AI_CORE, "AICORE_FREQ" },
                                                   { PROCESS_TYPE::NPU_MEM, "NPU_MEM" },
                                                   { PROCESS_TYPE::STARS_SOC, "SOC_BANDWIDTH_LEVEL" },
                                                   { PROCESS_TYPE::HBM, "HBM" } };

#pragma endregion

#pragma region << EnumToStr Template Specialization>>

template <typename E> std::optional<std::string> ENUM_TO_STR(const E &e)
{
    return std::nullopt;
}

template <typename E> std::optional<E> STR_TO_ENUM(const std::string &s)
{
    return std::nullopt;
}

template <typename E, typename M> std::optional<E> TryGetEnum(const M &map, const std::string &s)
{
    for (const auto &iter : map) {
        if (iter.second == s) {
            return iter.first;
        }
    }
    return std::nullopt;
}

// ProtocolMessage::Type
template <> inline std::optional<std::string> ENUM_TO_STR<ProtocolMessage::Type>(const ProtocolMessage::Type &e)
{
    if (PROTOCOL_MESSAGE_TYPE_ES.count(e) == 0) {
        return std::nullopt;
    }
    return PROTOCOL_MESSAGE_TYPE_ES.at(e);
}

template <> inline std::optional<ProtocolMessage::Type> STR_TO_ENUM<ProtocolMessage::Type>(const std::string &s)
{
    return TryGetEnum<ProtocolMessage::Type>(PROTOCOL_MESSAGE_TYPE_ES, s);
}

// Protocol::ModuleType
template <> inline std::optional<std::string> ENUM_TO_STR<Protocol::ModuleType>(const Protocol::ModuleType &e)
{
    if (MODULE_TYPE_ES.count(e) == 0) {
        return std::nullopt;
    }
    return MODULE_TYPE_ES.at(e);
}

template <> inline std::optional<Protocol::ModuleType> STR_TO_ENUM<Protocol::ModuleType>(const std::string &s)
{
    return TryGetEnum<Protocol::ModuleType>(MODULE_TYPE_ES, s);
}

// Protocol::LinkType
template <> inline std::optional<std::string> ENUM_TO_STR<Protocol::LinkType>(const Protocol::LinkType &e)
{
    if (LINK_TYPE_ES.count(e) == 0) {
        return std::nullopt;
    }
    return LINK_TYPE_ES.at(e);
}

template <> inline std::optional<Protocol::LinkType> STR_TO_ENUM<Protocol::LinkType>(const std::string &s)
{
    return TryGetEnum<Protocol::LinkType>(LINK_TYPE_ES, s);
}

template <> inline std::optional<std::string> ENUM_TO_STR<PROCESS_TYPE>(const PROCESS_TYPE &e)
{
    if (PROCESS_TYPE_ES.count(e) == 0) {
        return std::nullopt;
    }
    return PROCESS_TYPE_ES.at(e);
}

template <> inline std::optional<PROCESS_TYPE> STR_TO_ENUM<PROCESS_TYPE>(const std::string &s)
{
    return TryGetEnum<PROCESS_TYPE>(PROCESS_TYPE_ES, s);
}
#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_PROTOCOL_ENUM_UTIL_H
