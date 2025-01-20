/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERY_API_LINE_DYNAMIC_HANDLER_H
#define PROFILER_SERVER_QUERY_API_LINE_DYNAMIC_HANDLER_H

#include "SourceRequestHandler.h"
#include "SourceProtocolRequest.h"
#include "SourceProtocolResponse.h"

namespace Dic {
namespace Module {
namespace Source {
class QueryApiLineDynamicHandler : public SourceRequestHandler {
public:
    QueryApiLineDynamicHandler()
    {
        command = Protocol::REQ_RES_SOURCE_API_LINE;
    }

    ~QueryApiLineDynamicHandler() override = default;

    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
protected:
    void SetResponseBody(SourceApiLineDynamicResponse &response, SourceApiLineDynamicRequest &request);
    template<typename T>
    void TransformColumnData(const std::unordered_map<std::string, std::vector<T>> &source,
                             std::unordered_map<std::string, T> &target);
};
} // Source
} // Module
} // Dic

#endif // PROFILER_SERVER_QUERY_API_LINE_DYNAMIC_HANDLER_H
