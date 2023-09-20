/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_MEMORYPROTOCOL_H
#define PROFILER_SERVER_MEMORYPROTOCOL_H

#include "ProtocolUtil.h"

namespace Dic {
namespace Protocol {
class MemoryProtocol : public ProtocolUtil {
public:
    MemoryProtocol() = default;
    ~MemoryProtocol() override = default;

private:
    void RegisterJsonToRequestFuncs() override;
    void RegisterResponseToJsonFuncs() override;
    void RegisterEventToJsonFuncs() override {};

    // json to request
    static std::unique_ptr<Request> ToMemoryOperatorRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToMemoryViewRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToMemoryOperatorSizeRequest(const json_t &json, std::string &error);
    // response to json
    static std::optional<json_t> ToMemoryOperatorResponseJson(const Response &response);
    static std::optional<json_t> ToMemoryViewResponseJson(const Response &response);
    static std::optional<json_t> ToMemoryOperatorSizeResponseJson(const Response &response);
};

} // end of namespace Protocol
} // end of namespace Dic

#endif // PROFILER_SERVER_MEMORYPROTOCOL_H
