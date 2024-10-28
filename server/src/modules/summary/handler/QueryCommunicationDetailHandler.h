/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */


#ifndef PROFILER_SERVER_QUERY_COMMUNICATIONDETAIL_HANDLER_H
#define PROFILER_SERVER_QUERY_COMMUNICATIONDETAIL_HANDLER_H

#include "SummaryRequestHandler.h"

namespace Dic {
namespace Module {
namespace Summary {
class QueryCommunicationDetailHandler : public SummaryRequestHandler {
public:
    QueryCommunicationDetailHandler()
    {
        command = Protocol::REQ_RES_COMMUNICATION_DETAIL;
    }
    ~QueryCommunicationDetailHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};

} // Summary
} // Module
} // Dic

#endif // PROFILER_SERVER_QUERY_COMMUNICATIONDETAIL_HANDLER_H
