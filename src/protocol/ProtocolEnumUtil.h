/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol enum utility declaration
 */

#ifndef DIC_PROTOCOL_ENUM_UTIL_H
#define DIC_PROTOCOL_ENUM_UTIL_H

#include <map>
#include <string>
#include <optional>

#include "ProtocolEnum.h"
#include "ProtocolBase.h"

namespace Dic {
namespace Protocol {
#pragma region << Enum String Map>>

template <typename ENUM> using EnumStrMap = std::map<ENUM, std::string>;

const EnumStrMap<ProtocolMessage::Type> PROTOCOL_MESSAGE_TYPE_ES = { { ProtocolMessage::Type::REQUEST, "request" },
                                                                     { ProtocolMessage::Type::RESPONSE, "response" },
                                                                     { ProtocolMessage::Type::EVENT, "event" } };

const EnumStrMap<Protocol::SceneType> SCENE_TYPE_ES = { { Protocol::SceneType::GLOBAL, "global" },
                                                        { Protocol::SceneType::DATABASE, "database" },
                                                        { Protocol::SceneType::TOOL, "tool" },
                                                        { Protocol::SceneType::LOG, "log" },
                                                        { Protocol::SceneType::HARMONY, "harmony" } };

const EnumStrMap<Protocol::LinkType> LINK_TYPE_ES = { { Protocol::LinkType::WEBSOCKET, "websocket" },
                                                      { Protocol::LinkType::SOCKET, "socket" } };

const EnumStrMap<Protocol::DeviceStatus> DEVICE_STATUS_ES = { { Protocol::DeviceStatus::OFFLINE, "Offline" },
                                                              { Protocol::DeviceStatus::ONLINE, "Online" } };

const EnumStrMap<Protocol::ProcessStatus> PROCESS_STATUS_ES = { { Protocol::ProcessStatus::ALIVE, "Alive" },
                                                                { Protocol::ProcessStatus::DEAD, "Dead" } };

const EnumStrMap<Protocol::ApplicationStatus> APPLICATION_STATUS_ES = { { Protocol::ApplicationStatus::INSTALLED,
                                                                          "Installed" },
                                                                        { Protocol::ApplicationStatus::UN_INSTALLED,
                                                                          "UnInstalled" },
                                                                        { Protocol::ApplicationStatus::RUNNING,
                                                                          "Running" } };

const EnumStrMap<Protocol::ProcessType> PROCESS_TYPE_ES = { { Protocol::ProcessType::SYSTEM_PROCESS, "System Process" },
                                                            { Protocol::ProcessType::MAIN_PROCESS, "Main Process" },
                                                            { Protocol::ProcessType::EXTENSION_PROCESS,
                                                              "Extension Process" },
                                                            { Protocol::ProcessType::RENDER_PROCESS,
                                                              "Render Process" } };

const EnumStrMap<Protocol::DeviceConnectType> DEVICE_CONNECT_TYPE_ES = {
    { Protocol::DeviceConnectType::USB, "USB" },
    { Protocol::DeviceConnectType::TCP, "TCP" },
};

#pragma endregion

#pragma region << EnumToStr Template Specialization>>

template <typename E> const std::optional<std::string> ENUM_TO_STR(const E &e)
{
    return std::nullopt;
}

template <typename E> const std::optional<E> STR_TO_ENUM(const std::string &s)
{
    return std::nullopt;
}

template <typename E, typename M> const std::optional<E> TryGetEnum(const M &map, const std::string &s)
{
    for (auto iter : map) {
        if (iter.second == s) {
            return iter.first;
        }
    }
    return std::nullopt;
}

// ProtocolMessage::Type
template <> inline const std::optional<std::string> ENUM_TO_STR<ProtocolMessage::Type>(const ProtocolMessage::Type &e)
{
    if (PROTOCOL_MESSAGE_TYPE_ES.count(e) == 0) {
        return std::nullopt;
    }
    return PROTOCOL_MESSAGE_TYPE_ES.at(e);
}

template <> inline const std::optional<ProtocolMessage::Type> STR_TO_ENUM<ProtocolMessage::Type>(const std::string &s)
{
    return TryGetEnum<ProtocolMessage::Type>(PROTOCOL_MESSAGE_TYPE_ES, s);
}

// Protocol::SceneType
template <> inline const std::optional<std::string> ENUM_TO_STR<Protocol::SceneType>(const Protocol::SceneType &e)
{
    if (SCENE_TYPE_ES.count(e) == 0) {
        return std::nullopt;
    }
    return SCENE_TYPE_ES.at(e);
}

template <> inline const std::optional<Protocol::SceneType> STR_TO_ENUM<Protocol::SceneType>(const std::string &s)
{
    return TryGetEnum<Protocol::SceneType>(SCENE_TYPE_ES, s);
}

// Protocol::LinkType
template <> inline const std::optional<std::string> ENUM_TO_STR<Protocol::LinkType>(const Protocol::LinkType &e)
{
    if (LINK_TYPE_ES.count(e) == 0) {
        return std::nullopt;
    }
    return LINK_TYPE_ES.at(e);
}

template <> inline const std::optional<Protocol::LinkType> STR_TO_ENUM<Protocol::LinkType>(const std::string &s)
{
    return TryGetEnum<Protocol::LinkType>(LINK_TYPE_ES, s);
}

// Protocol::DeviceStatus
template <> inline const std::optional<std::string> ENUM_TO_STR<Protocol::DeviceStatus>(const Protocol::DeviceStatus &e)
{
    if (DEVICE_STATUS_ES.count(e) == 0) {
        return std::nullopt;
    }
    return DEVICE_STATUS_ES.at(e);
}

template <> inline const std::optional<Protocol::DeviceStatus> STR_TO_ENUM<Protocol::DeviceStatus>(const std::string &s)
{
    return TryGetEnum<Protocol::DeviceStatus>(DEVICE_STATUS_ES, s);
}

template <>
inline const std::optional<std::string> ENUM_TO_STR<Protocol::ProcessStatus>(const Protocol::ProcessStatus &e)
{
    if (PROCESS_STATUS_ES.count(e) == 0) {
        return std::nullopt;
    }
    return PROCESS_STATUS_ES.at(e);
}

template <>
inline const std::optional<std::string> ENUM_TO_STR<Protocol::ApplicationStatus>(const Protocol::ApplicationStatus &e)
{
    if (APPLICATION_STATUS_ES.count(e) == 0) {
        return std::nullopt;
    }
    return APPLICATION_STATUS_ES.at(e);
}

template <> inline const std::optional<std::string> ENUM_TO_STR<Protocol::ProcessType>(const Protocol::ProcessType &e)
{
    if (PROCESS_TYPE_ES.count(e) == 0) {
        return std::nullopt;
    }
    return PROCESS_TYPE_ES.at(e);
}

template <>
inline const std::optional<Protocol::ProcessStatus> STR_TO_ENUM<Protocol::ProcessStatus>(const std::string &s)
{
    return TryGetEnum<Protocol::ProcessStatus>(PROCESS_STATUS_ES, s);
}

// Protocol::DeviceConnectType
template <>
inline const std::optional<std::string> ENUM_TO_STR<Protocol::DeviceConnectType>(const Protocol::DeviceConnectType &e)
{
    if (DEVICE_CONNECT_TYPE_ES.count(e) == 0) {
        return std::nullopt;
    }
    return DEVICE_CONNECT_TYPE_ES.at(e);
}

template <>
inline const std::optional<Protocol::DeviceConnectType> STR_TO_ENUM<Protocol::DeviceConnectType>(const std::string &s)
{
    return TryGetEnum<Protocol::DeviceConnectType>(DEVICE_CONNECT_TYPE_ES, s);
}

template <>
inline const std::optional<Protocol::ApplicationStatus> STR_TO_ENUM<Protocol::ApplicationStatus>(const std::string &s)
{
    return TryGetEnum<Protocol::ApplicationStatus>(APPLICATION_STATUS_ES, s);
}

template <> inline const std::optional<Protocol::ProcessType> STR_TO_ENUM<Protocol::ProcessType>(const std::string &s)
{
    return TryGetEnum<Protocol::ProcessType>(PROCESS_TYPE_ES, s);
}
#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_PROTOCOL_ENUM_UTIL_H
