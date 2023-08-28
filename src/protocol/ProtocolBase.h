/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol declaration
 */

#ifndef DIC_PROTOCOL_BASE_H
#define DIC_PROTOCOL_BASE_H

#include <string>
#include <optional>
#include "ProtocolEnum.h"

namespace Dic {
namespace Protocol {
#pragma region << Base Protocol>>

struct ErrorMessage {
    int code = 0;
    std::string message;
};

struct ProtocolMessage {
    enum class Type : int {
        REQUEST = 0,
        RESPONSE,
        EVENT,
        NONE
    };
    virtual ~ProtocolMessage() {}
    unsigned int id = 0;
    ProtocolMessage::Type type;
    // request: { "moduleName": xxx, "params": { ...} }; response/event {"moduleName": xxx, "body": { ...} }
    ModuleType moduleName = ModuleType::UNKNOWN;
    // request: { "params": { "token": xxx ...} }; response/event { "body": { "token": xxx ...} }
    std::string token;
    std::optional<int> resultCallbackId;
};

struct Request : public ProtocolMessage {
    explicit Request(const std::string &command) : command(command)
    {
        type = ProtocolMessage::Type::REQUEST;
    }

    ~Request() override {}
    std::string command;
    // arguments will be placed into specified request
};

struct Response : public ProtocolMessage {
    explicit Response(const std::string &command) : command(command)
    {
        type = ProtocolMessage::Type::RESPONSE;
    }
    ~Response() override {}
    unsigned int requestId = 0;
    bool result = false;
    std::string command;
    std::optional<ErrorMessage> error;
    // body will be placed into specified response
};

struct Event : public ProtocolMessage {
    explicit Event(const std::string &e) : event(e)
    {
        type = ProtocolMessage::Type::EVENT;
    }

    ~Event() override {}
    std::string event;
    bool result = false;
    // body will be placed into specified event
};

#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_PROTOCOL_BASE_H
