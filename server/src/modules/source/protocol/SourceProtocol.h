/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 */

#ifndef PROFILER_SERVER_SOURCE_PROTOCOL_H
#define PROFILER_SERVER_SOURCE_PROTOCOL_H

#include "ProtocolUtil.h"

namespace Dic {
namespace Protocol {
class SourceProtocol : public ProtocolUtil {
public:
    SourceProtocol() = default;
    ~SourceProtocol() override = default;

private:
    void RegisterJsonToRequestFuncs() override;
    void RegisterResponseToJsonFuncs() override;
    void RegisterEventToJsonFuncs() override;

    // json to request
    static std::unique_ptr<Request> ToCodeFileRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToApiLineRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToApiInstrRequest(const json_t &json, std::string &error);

    // response to json
    static std::optional<document_t> ToCodeFileResponse(const Response &response);
    static std::optional<document_t> ToApiLineResponse(const Response &response);
    static std::optional<document_t> ToApiInstrResponse(const Response &response);
};

}
}
#endif // PROFILER_SERVER_SOURCE_PROTOCOL_H
