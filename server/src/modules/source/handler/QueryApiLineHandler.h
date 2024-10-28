/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERY_API_LINE_HANDLER_H
#define PROFILER_SERVER_QUERY_API_LINE_HANDLER_H

#include "SourceRequestHandler.h"

namespace Dic {
namespace Module {
namespace Source {
class QueryApiLineHandler : public SourceRequestHandler {
public:
    QueryApiLineHandler()
    {
        command = Protocol::REQ_RES_SOURCE_API_LINE;
    }

    ~QueryApiLineHandler() override = default;

    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // Source
} // Module
} // Dic

#endif // PROFILER_SERVER_QUERY_API_LINE_HANDLER_H
