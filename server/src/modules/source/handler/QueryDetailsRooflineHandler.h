// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.


#ifndef PROFILER_SERVER_QUERYDETAILSROOFLINEHANDLER_H
#define PROFILER_SERVER_QUERYDETAILSROOFLINEHANDLER_H

#include "SourceRequestHandler.h"

namespace Dic::Module::Source {
class QueryDetailsRooflineHandler : public SourceRequestHandler {
public:
    QueryDetailsRooflineHandler()
    {
        command = Protocol::REQ_RES_DETAILS_ROOFLINE;
    }

    ~QueryDetailsRooflineHandler() override = default;

    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
}


#endif // PROFILER_SERVER_QUERYDETAILSROOFLINEHANDLER_H
