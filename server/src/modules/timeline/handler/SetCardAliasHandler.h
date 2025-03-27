/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_SET_CARD_ALIAS_HANDLER_H
#define PROFILER_SERVER_SET_CARD_ALIAS_HANDLER_H

#include "TimelineRequestHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
class SetCardAliasHandler : public TimelineRequestHandler {
public:
    SetCardAliasHandler()
    {
        command = Protocol::REQ_RES_UNIT_SET_CARD_ALIAS;
    };
    ~SetCardAliasHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_SET_CARD_ALIAS_HANDLER_H
