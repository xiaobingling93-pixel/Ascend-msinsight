/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_IEPROTOCOL_H
#define PROFILER_SERVER_IEPROTOCOL_H
#include "ProtocolMessage.h"

namespace Dic {
namespace Protocol {
class IEProtocol : public ProtocolUtil {
public:
    IEProtocol() = default;

    ~IEProtocol() override = default;

private:
    void RegisterJsonToRequestFuncs() override;

    void RegisterResponseToJsonFuncs() override;

    void RegisterEventToJsonFuncs() override;

    static std::unique_ptr<Request> ToIEUsageViewRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToIETableRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToIEGroupRequest(const json_t &json, std::string &error);

    static std::optional<document_t> ToIEUsageViewResponseJson(const Response &response);
    static std::optional<document_t> ToIETableViewResponseJson(const Response &response);
    static std::optional<document_t> ToIEGroupResponseJson(const Response &response);

    static std::optional<document_t> ToParseIECompletedEventJson(const Event &event);
};
}
}
#endif // PROFILER_SERVER_IEPROTOCOL_H
