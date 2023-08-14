/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: Protocol request manager declaration
 */

#ifndef DIC_PROTOCOL_REQUEST_MANAGER_H
#define DIC_PROTOCOL_REQUEST_MANAGER_H

#include <string>
#include <map>
#include <functional>
#include <mutex>
#include <optional>
#include <memory>
#include "GlobalDefs.h"
#include "ProtocolRequest.h"

namespace Dic {
namespace Protocol {
class RequestManager {
public:
    static RequestManager &Instance()
    {
        static RequestManager instance;
        return instance;
    }
    static bool IsRequest(const json_t &jsonRequest);
    const std::string Command(const json_t &jsonRequest) const;
    const std::unique_ptr<Request> FromJson(const json_t &requestJson, std::string &error);
    const std::unique_ptr<Request> FromJson(const std::string &requestStr, std::string &error);

    using JsonToRequestFunc = std::function<std::unique_ptr<Request>(const json_t &, std::string &error)>;
    const std::optional<JsonToRequestFunc> GetJsonToRequestFunc(const std::string &command);

private:
    RequestManager();
    ~RequestManager();

    void Register();
    void RegisterJsonToRequestFuncs();
    void UnRegister();

    // Json to request
    // global
    static std::unique_ptr<Request> ToTokenCreateRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToTokenDestroyRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToTokenCheckRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToConfigGetRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToConfigSetRequest(const json_t &json, std::string &error);
    // harmony
    static std::unique_ptr<Request> ToHdcDeviceListRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToImportActionRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToUnitThreadTracesRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToUnitThreadsRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToThreadDetailRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToUnitFlowNameRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToUnitFlowRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToResetWindowRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToUnitChartRequest(const json_t &json, std::string &error);

    std::mutex mutex;
    std::map<std::string, JsonToRequestFunc> jsonToReqFactory;
};
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_PROTOCOL_REQUEST_MANAGER_H