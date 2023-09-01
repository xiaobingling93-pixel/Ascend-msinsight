/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_GLOBAL_PROTOCOL_H
#define PROFILER_SERVER_GLOBAL_PROTOCOL_H

#include "ProtocolUtil.h"

namespace Dic {
namespace Protocol {
class GlobalProtocol : public ProtocolUtil {
public:
    GlobalProtocol() = default;
    ~GlobalProtocol() override = default;

private:
    void RegisterJsonToRequestFuncs() override;
    void RegisterResponseToJsonFuncs() override;
    void RegisterEventToJsonFuncs() override;

    // json to request
    static std::unique_ptr<Request> ToTokenCreateRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToTokenDestroyRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToTokenCheckRequest(const json_t &json, std::string &error);
    // response to json
    static std::optional<json_t> ToTokenCreateResponseJson(const Response &response);
    static std::optional<json_t> ToTokenDestroyResponseJson(const Response &response);
    static std::optional<json_t> ToTokenCheckResponseJson(const Response &response);
};
} // namespace Protocol
} // namespace Dic

#endif // PROFILER_SERVER_GLOBAL_PROTOCOL_H
