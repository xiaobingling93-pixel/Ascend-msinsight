/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#ifndef MSINSIGHT_PROTOCOL_UTIL_H
#define MSINSIGHT_PROTOCOL_UTIL_H
#include <string>
#include <map>
#include <functional>
#include <mutex>
#include <optional>
#include <memory>
#include "rapidjson.h"
#include "document.h"
#include "writer.h"
#include "stringbuffer.h"
#include <algorithm>
namespace Dic {
using json_t = rapidjson::Value;
using document_t = rapidjson::Document;
using namespace rapidjson;
const static int UNKNOW_ERROR = 1001;
const std::string MODULE_UNKNOWN = "unknown";
namespace Protocol {
struct ErrorMessage {
    int code = 0;
    std::string message;
};
struct ProtocolMessage {
    enum class Type : int {
        REQUEST = 0, RESPONSE, EVENT, NONE
    };
    virtual ~ProtocolMessage() = default;
    unsigned int id = 0;
    ProtocolMessage::Type type = Type::NONE;
    std::string moduleName = MODULE_UNKNOWN;
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
    std::string projectName;
    std::string command;
    std::string fileId;
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
};
struct Event : public ProtocolMessage {
    explicit Event(const std::string &e) : event(e)
    {
        type = ProtocolMessage::Type::EVENT;
    }
    ~Event() override = default;
    std::string event;
    bool result = false;
};
class ProtocolUtil {
public:
    ProtocolUtil() = default;
    virtual ~ProtocolUtil() = default;

    void Register();
    void UnRegister();

    std::unique_ptr<Request> FromJson(const json_t &requestJson, std::string &error);
    std::optional<document_t> ToJson(const Response &response, std::string &error);
    std::optional<document_t> ToJson(const Event &event, std::string &error);

    // set base info
    // request
    static bool SetRequestBaseInfo(Request &request, const json_t &json);
    // response
    static void SetResponseJsonBaseInfo(const Response &response, document_t &json);
    // event
    static void SetEventJsonBaseInfo(const Event &event, document_t &json);

protected:
    std::mutex mutex;
    using JsonToRequestFunc = std::function<std::unique_ptr<Request>(const json_t &, std::string &error)>;
    using ResponseToJsonFunc = std::function<std::optional<document_t>(const Response &)>;
    using EventToJsonFunc = std::function<std::optional<document_t>(const Event &)>;
    std::map<std::string, JsonToRequestFunc> jsonToReqFactory;
    std::map<std::string, ResponseToJsonFunc> resToJsonFactory;
    std::map<std::string, EventToJsonFunc> eventToJsonFactory;

private:
    virtual void RegisterJsonToRequestFuncs() = 0;
    virtual void RegisterResponseToJsonFuncs() = 0;
    virtual void RegisterEventToJsonFuncs() = 0;

    // request
    static bool IsRequest(const json_t &jsonRequest);
    static std::string Command(const json_t &jsonRequest);
    std::optional<JsonToRequestFunc> GetJsonToRequestFunc(const std::string &command);
    // response
    std::optional<ResponseToJsonFunc> GetResponseToJsonFunc(const std::string &command);
    // event
    std::optional<EventToJsonFunc> GetEventToJsonFunc(const std::string &event);
};
} // namespace Protocol
} // namespace Dic

#endif // MSINSIGHT_PROTOCOL_UTIL_H
