/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYMODELINFOHANDLER_H
#define PROFILER_SERVER_QUERYMODELINFOHANDLER_H

#include "SummaryRequestHandler.h"

namespace Dic {
namespace Module {
namespace Summary {
class QueryModelInfoHandler : public SummaryRequestHandler {
public:
    QueryModelInfoHandler()
    {
        command = Protocol::REQ_RES_IMPORT_EXPERT_DATA;
    }
    ~QueryModelInfoHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
}
}
}
#endif // PROFILER_SERVER_QUERYMODELINFOHANDLER_H
