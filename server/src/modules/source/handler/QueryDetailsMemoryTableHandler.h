/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYDETAILSMEMORYTABLEHANDLER_H
#define PROFILER_SERVER_QUERYDETAILSMEMORYTABLEHANDLER_H
#include "SourceRequestHandler.h"

namespace Dic {
namespace Module {
namespace Source {
class QueryDetailsMemoryTableHandler : public SourceRequestHandler {
public:
    QueryDetailsMemoryTableHandler()
    {
        command = Protocol::REQ_RES_DETAILS_COMPUTE_MEMORY_TABLE;
    }
    ~QueryDetailsMemoryTableHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
}
}
}
#endif // PROFILER_SERVER_QUERYDETAILSMEMORYTABLEHANDLER_H
