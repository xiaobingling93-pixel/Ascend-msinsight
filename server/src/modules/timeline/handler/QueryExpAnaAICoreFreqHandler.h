/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/

#ifndef PROFILER_SERVER_QUERYEXPANAAICOREFREQHANDLER_H
#define PROFILER_SERVER_QUERYEXPANAAICOREFREQHANDLER_H

#include "TimelineRequestHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
class QueryExpAnaAICoreFreqHandler : public TimelineRequestHandler {
public:
    QueryExpAnaAICoreFreqHandler()
    {
        command = Protocol::REQ_RES_EXPERT_ANALYSIS_AICORE_FREQ;
    };

    ~QueryExpAnaAICoreFreqHandler() override = default;

    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_QUERYEXPANAAICOREFREQHANDLER_H
