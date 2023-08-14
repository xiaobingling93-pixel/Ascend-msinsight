//
// * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//

#ifndef PROFILER_SERVER_QUERY_FLOW_NAME_HANDLER_H
#define PROFILER_SERVER_QUERY_FLOW_NAME_HANDLER_H

#include "AscendRequestHandler.h"

namespace Dic {
namespace Scene {
class QueryFlowNameHandler : public AscendRequestHandler {
public:
    QueryFlowNameHandler()
    {
        command = Protocol::REQ_RES_UNIT_FLOW_NAME;
    };
    ~QueryFlowNameHandler() override = default;
    void HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Scene
} // end of namespace Dic


#endif // PROFILER_SERVER_QUERY_FLOW_NAME_HANDLER_H
