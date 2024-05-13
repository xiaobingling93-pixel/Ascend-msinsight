/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_ADVISORPROTOCOLREQUEST_H
#define PROFILER_SERVER_ADVISORPROTOCOLREQUEST_H

#include "ProtocolDefs.h"
#include "ProtocolMessage.h"
namespace Dic::Protocol {
struct APITypeParams {
    std::string rankId;
    uint32_t currentPage{};
    uint32_t pageSize{};
    std::string orderBy;
    std::string orderType;
};

struct AffinityOptimizerRequest : public Request {
    AffinityOptimizerRequest() : Request(REQ_RES_ADVISOR_AFFINITY_OPTIMIZER) {};
    APITypeParams params;
};

struct AffinityAPIRequest : public Request {
    AffinityAPIRequest() : Request(REQ_RES_ADVISOR_AFFINITY_API) {};
    APITypeParams params;
};

struct OperatorFusionRequest : public Request {
    OperatorFusionRequest() : Request(REQ_RES_ADVISOR_OPERATORS_FUSION) {};
    APITypeParams params;
};

struct AICpuOperatorRequest : public Request {
    AICpuOperatorRequest() : Request(REQ_RES_ADVISOR_AICPU_OPERATORS) {};
    APITypeParams params;
};

struct AclnnOperatorRequest : public Request {
    AclnnOperatorRequest() : Request(REQ_RES_ADVISOR_ACLNN_OPERATORS) {};
    APITypeParams params;
};

}
#endif // PROFILER_SERVER_ADVISORPROTOCOLREQUEST_H
