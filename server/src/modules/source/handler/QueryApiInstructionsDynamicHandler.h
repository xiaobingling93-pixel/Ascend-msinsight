/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERY_API_INSTRUCTIONS_DYNAMIC_H
#define PROFILER_SERVER_QUERY_API_INSTRUCTIONS_DYNAMIC_H


#include "SourceRequestHandler.h"
#include "SourceProtocolResponse.h"
#include "SourceProtocolRequest.h"

namespace Dic {
namespace Module {
namespace Source {
class QueryApiInstructionsDynamicHandler : public SourceRequestHandler {
public:
    QueryApiInstructionsDynamicHandler()
    {
        command = Protocol::REQ_RES_SOURCE_API_INSTRUCTIONS_DYNAMIC;
    }

    ~QueryApiInstructionsDynamicHandler() override = default;

    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
protected:
    void SetResponseBody(SourceApiInstrDynamicResponse &response, SourceApiInstrDynamicRequest &request);
    template<typename T>
    void TransformColumnData(const std::unordered_map<std::string, std::vector<T>> &source,
                             std::unordered_map<std::string, T> &target);
};
} // Source
} // Module
} // Dic
#endif // PROFILER_SERVER_QUERY_API_INSTRUCTIONS_DYNAMIC_H
