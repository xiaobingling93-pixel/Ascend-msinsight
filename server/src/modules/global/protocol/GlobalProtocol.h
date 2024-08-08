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
    static std::unique_ptr<Request> ToFilesGetRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToHeartCheckRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToProjectExplorerUpdateRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToProjectExplorerInfoGetRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToProjectExplorerInfoDeleteRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToProjectValidCheckRequest(const json_t &json, std::string &error);
    // response to json
    static std::optional<document_t> ToFilesGetResponseJson(const Response &response);
    static std::optional<document_t> ToTokenHeartCheckResponseJson(const Response &response);
    static std::optional<document_t> ToProjectExplorerInfoUpdateResponseJson(const Response &response);
    static std::optional<document_t> ToProjectExplorerInfoGetResponseJson(const Response &response);
    static std::optional<document_t> ToProjectExplorerInfoDeleteResponseJson(const Response &response);
    static std::optional<document_t> ToProjectValidCheckResponseJson(const Response &response);
};
} // namespace Protocol
} // namespace Dic

#endif // PROFILER_SERVER_GLOBAL_PROTOCOL_H
