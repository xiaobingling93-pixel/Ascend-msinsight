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
const int64_t MIN_PAGESIZE = 0;
const int64_t MAX_PAGESIZE = 1000;
const int64_t MIN_CURRENT_PAGE = 0;
const int64_t MAX_CURRENT_PAGE = 10000000000;
inline bool CheckPageValid(int64_t pageSize, int64_t currentPage, std::string &errorMsg)
{
    if (pageSize <= MIN_PAGESIZE || pageSize > MAX_PAGESIZE) {
        errorMsg = "pageSize:" + std::to_string(pageSize) + " is invaild";
        return false;
    }
    if (currentPage <= MIN_CURRENT_PAGE || currentPage > MAX_CURRENT_PAGE) {
        errorMsg = "currentPage" + std::to_string(currentPage) + " is invaild";
        return false;
    }
    return true;
}

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
    virtual ~ProtocolMessage() = default;
    unsigned int id = 0;
    ProtocolMessage::Type type = Type::NONE;
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

    ~Request() override = default;
    std::string command;
    // arguments will be placed into specified request
};

struct Response : public ProtocolMessage {
    explicit Response(const std::string &command) : command(command)
    {
        type = ProtocolMessage::Type::RESPONSE;
    }
    ~Response() override = default;
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

    ~Event() override = default;
    std::string event;
    bool result = false;
    // body will be placed into specified event
};

#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_PROTOCOL_BASE_H
