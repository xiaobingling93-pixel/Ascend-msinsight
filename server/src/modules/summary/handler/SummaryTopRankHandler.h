/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_SUMMARYTOPRANKHANDLER_HANDLER_H
#define PROFILER_SERVER_SUMMARYTOPRANKHANDLER_HANDLER_H

#include <set>
#include <regex>
#include "ProtocolMessage.h"
#include "ProtocolDefs.h"
#include "SummaryRequestHandler.h"

namespace Dic {
namespace Module {
namespace Summary {
class SummaryTopRankHandler : public SummaryRequestHandler {
public:
    SummaryTopRankHandler()
    {
        command = Protocol::REQ_RES_SUMMARY_QUERY_TOP_DATA;
    };
    ~SummaryTopRankHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_SUMMARYTOPRANKHANDLER_HANDLER_H
