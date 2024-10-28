/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_PARSE_CARDS_HANDLER_H
#define PROFILER_SERVER_PARSE_CARDS_HANDLER_H

#include <set>
#include <regex>
#include "TimelineRequestHandler.h"
#include "ParserFactory.h"
#include "FileParser.h"

namespace Dic {
namespace Module {
namespace Timeline {
class ParseCardsHandler : public TimelineRequestHandler {
public:
    ParseCardsHandler()
    {
        command = Protocol::REQ_RES_PARSE_CARDS;
        async = false;
    };
    ~ParseCardsHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_PARSE_CARDS_HANDLER_H
