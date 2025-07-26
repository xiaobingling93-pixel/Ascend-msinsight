/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_RLPROTOCOL_H
#define PROFILER_SERVER_RLPROTOCOL_H

#include "ProtocolMessage.h"

namespace Dic::Protocol {
class RLProtocol : public ProtocolUtil {
public:
    RLProtocol() = default;
    ~RLProtocol() override = default;
private:
    void RegisterJsonToRequestFuncs() override;
    void RegisterResponseToJsonFuncs() override;
    void RegisterEventToJsonFuncs() override;

    // json to request
    static std::unique_ptr<Request> ToRLPipelineRequest(const json_t &json, std::string &error);

    // response to json
    static std::optional<document_t> ToRLPipelineResponse(const Response &response);
};
}
#endif // PROFILER_SERVER_RLPROTOCOL_H
