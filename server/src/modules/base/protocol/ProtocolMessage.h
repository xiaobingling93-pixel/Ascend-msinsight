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
        errorMsg = "pagesize: " + std::to_string(pageSize) + " is invalid";
        return false;
    }
    if (currentPage <= MIN_CURRENT_PAGE || currentPage > MAX_CURRENT_PAGE) {
        errorMsg = "current page: " + std::to_string(currentPage) + " is invalid";
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
    // request: { "params": { ... }; response/event { "body": {  ...} }
    std::optional<int> resultCallbackId;
};

struct Request : public ProtocolMessage {
    explicit Request(const std::string &command) : command(command)
    {
        type = ProtocolMessage::Type::REQUEST;
    }

    explicit Request(std::string_view command) : command(std::string(command))
    {
        type = ProtocolMessage::Type::REQUEST;
    }

    ~Request() override = default;
    // 当前选中项目名，如果没有选中的话，内容为空
    std::string projectName;
    std::string command;
    // arguments will be placed into specified request

    std::tuple<bool, std::string> ParamVaild() const
    {
        return {true, ""};
    }
};
// arguments will be placed into specified request
inline bool CheckStrParamValid(const std::string &param,
    std::string &errorMsg)
{
    constexpr unsigned maxStrLength = 500;
    if (param.size() > maxStrLength) {
        errorMsg = "Parameter length exceeds the upper limit " + std::to_string(maxStrLength) + ".";
        return false;
    }
    if (!StringUtil::ValidateCommandFilePathParam(param)) {
        errorMsg = "Parameter contains illegal character. Illegal characters are: " +
            StringUtil::GetIllegalCharacter() + " and newline character.";
        return false;
    }
    return true;
}

// 和CheckStrParamValid不同，该函数认为空字符串是合法的
inline bool CheckStrParamValidEmptyAllowed(const std::string &param,
    std::string &errorMsg)
{
    constexpr unsigned maxStrLength = 500;
    if (param.size() > maxStrLength) {
        errorMsg = "Parameter length exceeds the upper limit " + std::to_string(maxStrLength) + ".";
        return false;
    }
    if (!StringUtil::ValidateStringParam(param)) {
        errorMsg = "Parameter contains illegal character. Illegal characters are: " +
            StringUtil::GetIllegalCharacter() + " and newline character.";
        return false;
    }
    return true;
}

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
