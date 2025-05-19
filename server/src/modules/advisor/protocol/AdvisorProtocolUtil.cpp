/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */


#include "AdvisorProtocolUtil.h"
#include "AdvisorProtocolFromRequestJson.h"
#include "AdvisorProtocolToResponseJson.h"

namespace Dic::Protocol {
void AdvisorProtocolUtil::RegisterJsonToRequestFuncs()
{
    jsonToReqFactory.emplace(REQ_RES_ADVISOR_AFFINITY_OPTIMIZER,
                             AdvisorProtocolFromRequestJson::ToAffinityOptimizerRequest);
    jsonToReqFactory.emplace(REQ_RES_ADVISOR_AFFINITY_API, AdvisorProtocolFromRequestJson::ToAffinityAPIRequest);
    jsonToReqFactory.emplace(REQ_RES_ADVISOR_OPERATORS_FUSION, AdvisorProtocolFromRequestJson::ToOperatorFusionRequest);
    jsonToReqFactory.emplace(REQ_RES_ADVISOR_AICPU_OPERATORS, AdvisorProtocolFromRequestJson::ToAICpuOperatorRequest);
    jsonToReqFactory.emplace(REQ_RES_ADVISOR_ACLNN_OPERATORS, AdvisorProtocolFromRequestJson::ToAclnnOperatorRequest);
    jsonToReqFactory.emplace(REQ_RES_ADVISOR_OPERATOR_DISPATCH,
                             AdvisorProtocolFromRequestJson::ToOperatorDispatchRequest);
}

void AdvisorProtocolUtil::RegisterResponseToJsonFuncs()
{
    resToJsonFactory.emplace(REQ_RES_ADVISOR_AFFINITY_OPTIMIZER,
                             AdvisorProtocolToResponseJson::ToAffinityOptimizerResponse);
    resToJsonFactory.emplace(REQ_RES_ADVISOR_AFFINITY_API, AdvisorProtocolToResponseJson::ToAffinityAPIResponse);
    resToJsonFactory.emplace(REQ_RES_ADVISOR_OPERATORS_FUSION, AdvisorProtocolToResponseJson::ToOperatorFusionResponse);
    resToJsonFactory.emplace(REQ_RES_ADVISOR_AICPU_OPERATORS, AdvisorProtocolToResponseJson::ToAICpuOperatorResponse);
    resToJsonFactory.emplace(REQ_RES_ADVISOR_ACLNN_OPERATORS, AdvisorProtocolToResponseJson::ToAclnnOperatorResponse);
    resToJsonFactory.emplace(REQ_RES_ADVISOR_OPERATOR_DISPATCH,
                             AdvisorProtocolToResponseJson::ToOperatorDispatchResponse);
}

void AdvisorProtocolUtil::RegisterEventToJsonFuncs() {}
}