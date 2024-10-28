/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_REMOTE_DELETE_HANDLER_H
#define PROFILER_SERVER_REMOTE_DELETE_HANDLER_H

#include "TimelineRequestHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
class RemoteDeleteHandler : public TimelineRequestHandler {
public:
    RemoteDeleteHandler()
    {
        command = Protocol::REQ_RES_REMOTE_DELETE;
        async = false;
    };
    ~RemoteDeleteHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;

private:
    static void GetUpdateTime(RemoteDeleteBody &body);
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_REMOTE_DELETE_HANDLER_H
