/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYDETAILSBASEINFOHANDLER_H
#define PROFILER_SERVER_QUERYDETAILSBASEINFOHANDLER_H

#include "SourceRequestHandler.h"

namespace Dic {
namespace Module {
namespace Source {
class QueryDetailsBaseInfoHandler : public SourceRequestHandler {
public:
    QueryDetailsBaseInfoHandler()
    {
        command = Protocol::REQ_RES_DETAILS_BASE_INFO;
    }
    ~QueryDetailsBaseInfoHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
}
}
}
#endif // PROFILER_SERVER_QUERYDETAILSBASEINFOHANDLER_H
