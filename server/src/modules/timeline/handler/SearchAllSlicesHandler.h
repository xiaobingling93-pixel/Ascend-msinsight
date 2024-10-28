/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SEARCH_ALL_SLICES_HANDLER_H
#define PROFILER_SERVER_SEARCH_ALL_SLICES_HANDLER_H

#include "TimelineRequestHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
class SearchAllSlicesHandler : public TimelineRequestHandler {
public:
    SearchAllSlicesHandler()
    {
        command = Protocol::REQ_RES_SEARCH_ALL_SLICES;
    };
    ~SearchAllSlicesHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_SEARCH_ALL_SLICES_HANDLER_H
