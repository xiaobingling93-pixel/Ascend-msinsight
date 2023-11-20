/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "JsonUtil.h"
#include "OperatorProtocolRequest.h"
#include "OperatorProtocolUtil.h"
#include "ProtocolDefs.h"
#include "OperatorProtocol.h"

namespace Dic::Protocol {

    void OperatorProtocol::RegisterJsonToRequestFuncs()
    {
        jsonToReqFactory.emplace(REQ_RES_OPERATOR_CATEGORY_INFO, ToOperatorCategoryInfoRequest);
        jsonToReqFactory.emplace(REQ_RES_OPERATOR_COMPUTE_UNIT_INFO, ToOperatorComputeUnitInfoRequest);
        jsonToReqFactory.emplace(REQ_RES_OPERATOR_STATISTIC_INFO, ToOperatorStatisticInfoRequest);
        jsonToReqFactory.emplace(REQ_RES_OPERATOR_DETAIL_INFO, ToOperatorDetailInfoRequest);
        jsonToReqFactory.emplace(REQ_RES_OPERATOR_MORE_INFO, ToOperatorMoreInfoRequest);
    }

    void OperatorProtocol::RegisterResponseToJsonFuncs()
    {
        resToJsonFactory.emplace(REQ_RES_OPERATOR_CATEGORY_INFO, ToOperatorCategoryInfoResponse);
        resToJsonFactory.emplace(REQ_RES_OPERATOR_COMPUTE_UNIT_INFO, ToOperatorComputeUnitInfoResponse);
        resToJsonFactory.emplace(REQ_RES_OPERATOR_STATISTIC_INFO, ToOperatorStatisticInfoResponse);
        resToJsonFactory.emplace(REQ_RES_OPERATOR_DETAIL_INFO, ToOperatorDetailInfoResponse);
        resToJsonFactory.emplace(REQ_RES_OPERATOR_MORE_INFO, ToOperatorMoreInfoResponse);
    }

    void OperatorProtocol::RegisterEventToJsonFuncs()
    {
        eventToJsonFactory.emplace(EVENT_PARSE_OPERATOR_STATUS, ToOperatorParseStatusEvent);
    }

    std::unique_ptr<Request> OperatorProtocol::ToOperatorCategoryInfoRequest(const json_t &json, std::string &error)
    {
        std::unique_ptr<OperatorCategoryInfoRequest> reqPtr = std::make_unique<OperatorCategoryInfoRequest>();
        if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
            error = "Failed to set request base info of Category Info, command is: " + reqPtr->command;
            return nullptr;
        }
        JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.group, json["params"], "group");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.topK, json["params"], "topK");
        return reqPtr;
    }

    std::unique_ptr<Request> OperatorProtocol::ToOperatorComputeUnitInfoRequest(const json_t &json, std::string &error)
    {
        std::unique_ptr<OperatorComputeUnitInfoRequest> reqPtr = std::make_unique<OperatorComputeUnitInfoRequest>();
        if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
            error = "Failed to set request base info of Compute Unit Info, command is: " + reqPtr->command;
            return nullptr;
        }
        JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.group, json["params"], "group");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.topK, json["params"], "topK");
        return reqPtr;
    }

    std::unique_ptr<Request> OperatorProtocol::ToOperatorStatisticInfoRequest(const json_t &json, std::string &error)
    {
        std::unique_ptr<OperatorStatisticInfoRequest> reqPtr = std::make_unique<OperatorStatisticInfoRequest>();
        if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
            error = "Failed to set request base info of Statistic Info, command is: " + reqPtr->command;
            return nullptr;
        }
        JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.group, json["params"], "group");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.topK, json["params"], "topK");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.current, json["params"], "current");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.pageSize, json["params"], "pageSize");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.orderBy, json["params"], "orderBy");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.order, json["params"], "order");
        return reqPtr;
    }

    std::unique_ptr<Request> OperatorProtocol::ToOperatorDetailInfoRequest(const json_t &json, std::string &error)
    {
        std::unique_ptr<OperatorDetailInfoRequest> reqPtr = std::make_unique<OperatorDetailInfoRequest>();
        if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
            error = "Failed to set request base info of Detail Info, command is: " + reqPtr->command;
            return nullptr;
        }
        JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.group, json["params"], "group");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.topK, json["params"], "topK");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.current, json["params"], "current");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.pageSize, json["params"], "pageSize");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.orderBy, json["params"], "orderBy");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.order, json["params"], "order");
        return reqPtr;
    }

    std::unique_ptr<Request> OperatorProtocol::ToOperatorMoreInfoRequest(const json_t &json, std::string &error)
    {
        std::unique_ptr<OperatorMoreInfoRequest> reqPtr = std::make_unique<OperatorMoreInfoRequest>();
        if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
            error = "Failed to set request base info of More Info, command is: " + reqPtr->command;
            return nullptr;
        }
        JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.group, json["params"], "group");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.topK, json["params"], "topK");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.opType, json["params"], "opType");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.opName, json["params"], "opName");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.shape, json["params"], "shape");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.current, json["params"], "current");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.pageSize, json["params"], "pageSize");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.orderBy, json["params"], "orderBy");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.order, json["params"], "order");
        return reqPtr;
    }

    std::optional<json_t> OperatorProtocol::ToOperatorCategoryInfoResponse(const Response &response)
    {
        return ToResponseJson<OperatorCategoryInfoResponse>(
                dynamic_cast<const OperatorCategoryInfoResponse &>(response));
    }

    std::optional<json_t> OperatorProtocol::ToOperatorComputeUnitInfoResponse(const Response &response)
    {
        return ToResponseJson<OperatorComputeUnitInfoResponse>(
                dynamic_cast<const OperatorComputeUnitInfoResponse &>(response));
    }

    std::optional<json_t> OperatorProtocol::ToOperatorStatisticInfoResponse(const Response &response)
    {
        return ToResponseJson<OperatorStatisticInfoResponse>(
                dynamic_cast<const OperatorStatisticInfoResponse &>(response));
    }

    std::optional<json_t> OperatorProtocol::ToOperatorDetailInfoResponse(const Response &response)
    {
        return ToResponseJson<OperatorDetailInfoResponse>(dynamic_cast<const OperatorDetailInfoResponse &>(response));
    }

    std::optional<json_t> OperatorProtocol::ToOperatorMoreInfoResponse(const Response &response)
    {
        return ToResponseJson<OperatorMoreInfoResponse>(dynamic_cast<const OperatorMoreInfoResponse &>(response));
    }

    std::optional<json_t> OperatorProtocol::ToOperatorParseStatusEvent(const Event &event)
    {
        return ToEventJson<OperatorParseStatusEvent>(dynamic_cast<const OperatorParseStatusEvent &>(event));
    }
}