/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_BASE_PROTOCOL_H
#define PROFILER_SERVER_BASE_PROTOCOL_H

#include <string>
#include <map>
#include <functional>
#include <mutex>
#include <optional>
#include <memory>
#include "GlobalDefs.h"
#include "ProtocolBase.h"

namespace Dic {
namespace Protocol {
class BaseProtocol {
public:
    BaseProtocol() = default;
    virtual ~BaseProtocol() = default;

    void Register();
    void UnRegister();

    std::unique_ptr<Request> FromJson(const json_t &requestJson, std::string &error);
    std::optional<json_t> ToJson(const Response &response, std::string &error);
    std::optional<json_t> ToJson(const Event &event, std::string &error);

private:
    virtual void RegisterJsonToRequestFuncs() = 0;
    virtual void RegisterResponseToJsonFuncs() = 0;
    virtual void RegisterEventToJsonFuncs() = 0;

    // request
    static bool IsRequest(const json_t &jsonRequest);
    static std::string Command(const json_t &jsonRequest) ;
    using JsonToRequestFunc = std::function<std::unique_ptr<Request>(const json_t &, std::string &error)>;
    std::optional<JsonToRequestFunc> GetJsonToRequestFunc(const std::string &command);

    // response
    using ResponseToJsonFunc = std::function<std::optional<json_t>(const Response &)>;
    std::optional<ResponseToJsonFunc> GetResponseToJsonFunc(const std::string &command);

    // event
    using EventToJsonFunc = std::function<std::optional<json_t>(const Event &)>;
    std::optional<EventToJsonFunc> GetEventToJsonFunc(const std::string &event);

protected:
    std::mutex mutex;
    std::map<std::string, JsonToRequestFunc> jsonToReqFactory;
    std::map<std::string, ResponseToJsonFunc> resToJsonFactory;
    std::map<std::string, EventToJsonFunc> eventToJsonFactory;
};
} // namespace Protocol
} // namespace Dic

#endif // PROFILER_SERVER_BASE_PROTOCOL_H
