/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: response manager declaration
 */

#ifndef DIC_PROTOCOL_RESPONSE_MANAGER_H
#define DIC_PROTOCOL_RESPONSE_MANAGER_H

#include <string>
#include <map>
#include <functional>
#include <mutex>
#include <optional>
#include <vector>
#include <memory>
#include "GlobalDefs.h"
#include "ProtocolResponse.h"
#include "ResponseUtil.h"

namespace Dic {
namespace Protocol {
class ResponseManager {
public:
    static ResponseManager &Instance()
    {
        static ResponseManager instance;
        return instance;
    }
    std::optional<json_t> ToJson(const Response &response, std::string &error);

    using ResponseToJsonFunc = std::function<std::optional<json_t>(const Response &)>;
    std::optional<ResponseManager::ResponseToJsonFunc> GetResponseToJsonFunc(const std::string &command);

private:
    ResponseManager();
    ~ResponseManager();

    void Register();
    void RegisterResponseToJsonFuncs();
    void UnRegister();

    // Response to json
    // global
    static std::optional<json_t> ToTokenCreateResponseJson(const Response &response);
    static std::optional<json_t> ToTokenDestroyResponseJson(const Response &response);
    static std::optional<json_t> ToTokenCheckResponseJson(const Response &response);
    static std::optional<json_t> ToConfigGetResponseJson(const Response &response);
    static std::optional<json_t> ToConfigSetResponseJson(const Response &response);
    // harmony
    static std::optional<json_t> ToHdcDeviceListResponseJson(const Response &response);

    std::mutex mutex;
    std::map<std::string, ResponseToJsonFunc> resToJsonFactory;
};
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_PROTOCOL_RESPONSE_MANAGER_H