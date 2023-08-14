//
// * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//

#ifndef PROFILER_SERVER_QUERY_THREAD_TRACES_HANDLER_H
#define PROFILER_SERVER_QUERY_THREAD_TRACES_HANDLER_H

#include "AscendRequestHandler.h"

namespace Dic {
namespace Scene {
class QueryThreadTracesHandler : public AscendRequestHandler {
public:
    QueryThreadTracesHandler()
    {
        command = Protocol::REQ_RES_UNIT_THREAD_TRACES;
    };
    ~QueryThreadTracesHandler() override = default;
    void HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Scene
} // end of namespace Dic


#endif // PROFILER_SERVER_QUERY_THREAD_TRACES_HANDLER_H
