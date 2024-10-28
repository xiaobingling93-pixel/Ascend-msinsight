/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_GROUP_HANDLER_H
#define PROFILER_SERVER_GROUP_HANDLER_H

#include "CommunicationRequestHandler.h"

namespace Dic {
namespace Module {
namespace Communication {
class GroupHandler : public CommunicationRequestHandler {
public:
    GroupHandler()
    {
        command = Protocol::REQ_RES_COMMUNICATION_MATRIX_GROUP;
    };
    ~GroupHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Communication
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_GROUP_HANDLER_H
