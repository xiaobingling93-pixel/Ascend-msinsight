/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYACLNNOPADVISORHANDLER_H
#define PROFILER_SERVER_QUERYACLNNOPADVISORHANDLER_H

#include "AdvisorRequestHandler.h"

namespace Dic::Module::Advisor {

class QueryAclnnOpAdvisorHandler : public AdvisorRequestHandler {
public:
    QueryAclnnOpAdvisorHandler()
    {
        command = Protocol::REQ_RES_ADVISOR_ACLNN_OPERATORS;
    }
    ~QueryAclnnOpAdvisorHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};

} // Dic::Module::Advisor

#endif // PROFILER_SERVER_QUERYACLNNOPADVISORHANDLER_H
