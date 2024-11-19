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
    void RegisterEventToJsonFuncs() override;

    // json to request
    static std::unique_ptr<Request> ToMemoryTypeRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToMemoryResourceTypeRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToMemoryOperatorRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToMemoryComponentRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToMemoryViewRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToMemoryOperatorSizeRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToMemoryStaticOperatorGraphRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToMemoryStaticOperatorListRequest(const json_t &json, std::string &error);
    // response to json
    static std::optional<document_t> ToMemoryTypeResponseJson(const Response &response);
    static std::optional<document_t> ToMemoryResourceTypeResponseJson(const Response &response);
    static std::optional<document_t> ToMemoryOperatorResponseJson(const Response &response);
    static std::optional<document_t> ToMemoryComponentResponseJson(const Response &response);
    static std::optional<document_t> ToMemoryViewResponseJson(const Response &response);
    static std::optional<document_t> ToMemoryOperatorSizeResponseJson(const Response &response);
    static std::optional<document_t> ToMemoryStaticOperatorGraphResponseJson(const Response &response);
    static std::optional<document_t> ToMemoryStaticOperatorListResponseJson(const Response &response);
};

} // end of namespace Protocol
} // end of namespace Dic

#endif // PROFILER_SERVER_MEMORYPROTOCOL_H
