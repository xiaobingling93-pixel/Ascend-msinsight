/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYDETAILSLOADINFOHANDLER_H
#define PROFILER_SERVER_QUERYDETAILSLOADINFOHANDLER_H

#include "SourceRequestHandler.h"

namespace Dic {
namespace Module {
namespace Source {
class QueryDetailsLoadInfoHandler : public SourceRequestHandler {
public:
    QueryDetailsLoadInfoHandler()
    {
        command = Protocol::REQ_RES_DETAILS_COMPUTE_LOAD_INFO;
    }
    ~QueryDetailsLoadInfoHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
}
}
}
#endif // PROFILER_SERVER_QUERYDETAILSLOADINFOHANDLER_H
