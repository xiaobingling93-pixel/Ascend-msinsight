/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERY_CODE_FILE_HANDLER_H
#define PROFILER_SERVER_QUERY_CODE_FILE_HANDLER_H

#include "SourceRequestHandler.h"

namespace Dic {
namespace Module {
namespace Source {
class QueryCodeFileHandler : public SourceRequestHandler {
public:
    QueryCodeFileHandler()
    {
        command = Protocol::REQ_RES_SOURCE_CODE_FILE;
    }
    ~QueryCodeFileHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};

} // Source
} // Module
} // Dic

#endif // PROFILER_SERVER_QUERY_CODE_FILE_HANDLER_H
