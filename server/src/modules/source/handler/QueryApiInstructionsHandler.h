/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERY_API_INSTRUCTIONS_H
#define PROFILER_SERVER_QUERY_API_INSTRUCTIONS_H


#include "SourceRequestHandler.h"

namespace Dic {
namespace Module {
namespace Source {
class QueryApiInstructionsHandler : public SourceRequestHandler {
public:
    QueryApiInstructionsHandler()
    {
        command = Protocol::REQ_RES_SOURCE_API_INSTRUCTIONS;
    }

    ~QueryApiInstructionsHandler() override = default;

    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // Source
} // Module
} // Dic
#endif // PROFILER_SERVER_QUERY_API_INSTRUCTIONS_H
