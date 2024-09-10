/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "pch.h"
#include "OperatorProtocolRequest.h"
#include "OperatorProtocolUtil.h"
#include "ProtocolDefs.h"
#include "TimelineProtocol.h"
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
        eventToJsonFactory.emplace(EVENT_PARSE_OPERATOR_CLEAR, ToOperatorParseClearEvent);
        eventToJsonFactory.emplace(EVENT_MODULE_RESET, TimelineProtocol::ToModuleResetEventJson);
    }

    std::unique_ptr<Request> OperatorProtocol::ToOperatorCategoryInfoRequest(const json_t &json, std::string &error)
    {
        std::unique_ptr<OperatorCategoryInfoRequest> reqPtr = std::make_unique<OperatorCategoryInfoRequest>();
        if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
            error = "Failed to set request base info of Category Info.";
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
            error = "Failed to set request base info of Compute Unit Info.";
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
            error = "Failed to set request base info of Statistic Info.";
            return nullptr;
        }
        JsonUtil::SetByJsonKeyValue(reqPtr->params.isCompare, json["params"], "isCompare");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.group, json["params"], "group");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.topK, json["params"], "topK");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.current, json["params"], "current");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.pageSize, json["params"], "pageSize");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.orderBy, json["params"], "orderBy");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.order, json["params"], "order");
        ToOperatorInfoRequestFilters<OperatorStatisticInfoRequest>(reqPtr, json, error);
        return reqPtr;
    }

    std::unique_ptr<Request> OperatorProtocol::ToOperatorDetailInfoRequest(const json_t &json, std::string &error)
    {
        std::unique_ptr<OperatorDetailInfoRequest> reqPtr = std::make_unique<OperatorDetailInfoRequest>();
        if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
            error = "Failed to set request base info of Detail Info.";
            return nullptr;
        }
        JsonUtil::SetByJsonKeyValue(reqPtr->params.isCompare, json["params"], "isCompare");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.group, json["params"], "group");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.topK, json["params"], "topK");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.current, json["params"], "current");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.pageSize, json["params"], "pageSize");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.orderBy, json["params"], "orderBy");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.order, json["params"], "order");
        ToOperatorInfoRequestFilters<OperatorDetailInfoRequest>(reqPtr, json, error);
        return reqPtr;
    }

    std::unique_ptr<Request> OperatorProtocol::ToOperatorMoreInfoRequest(const json_t &json, std::string &error)
    {
        std::unique_ptr<OperatorMoreInfoRequest> reqPtr = std::make_unique<OperatorMoreInfoRequest>();
        if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
            error = "Failed to set request base info of More Info.";
            return nullptr;
        }
        JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.group, json["params"], "group");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.topK, json["params"], "topK");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.opType, json["params"], "opType");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.opName, json["params"], "opName");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.shape, json["params"], "shape");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.accCore, json["params"], "accCore");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.current, json["params"], "current");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.pageSize, json["params"], "pageSize");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.orderBy, json["params"], "orderBy");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.order, json["params"], "order");
        ToOperatorInfoRequestFilters<OperatorMoreInfoRequest>(reqPtr, json, error);
        return reqPtr;
    }

    template <typename T>
    void OperatorProtocol::ToOperatorInfoRequestFilters(std::unique_ptr<T> &reqPtr,
                                                        const json_t &json, std::string &error)
    {
        std::map<std::string, std::string> colNameMap = {
                {"opType", "op_type"},
                {"type", "op_type"},
                {"opName", "name"},
                {"accCore", "accelerator_core"},
                {"name", "name"}
        };
        if (json["params"].HasMember("filters") && json["params"]["filters"].IsArray()) {
            for (const auto &filter : json["params"]["filters"].GetArray()) {
                if (filter.IsString()) {
                    std::pair<std::string, std::string> pFilter("", "");
                    auto fil = JsonUtil::TryParse(filter.GetString(), error);
                    pFilter.first = colNameMap[JsonUtil::GetString(fil->GetObj(), "columnName")];
                    pFilter.second = JsonUtil::GetString(fil->GetObj(), "value");
                    reqPtr->params.filters.emplace_back(pFilter);
                }
            }
        }
    }

    std::optional<document_t> OperatorProtocol::ToOperatorCategoryInfoResponse(const Response &response)
    {
        return ToResponseJson<OperatorCategoryInfoResponse>(
                dynamic_cast<const OperatorCategoryInfoResponse &>(response));
    }

    std::optional<document_t> OperatorProtocol::ToOperatorComputeUnitInfoResponse(const Response &response)
    {
        return ToResponseJson<OperatorComputeUnitInfoResponse>(
                dynamic_cast<const OperatorComputeUnitInfoResponse &>(response));
    }

    std::optional<document_t> OperatorProtocol::ToOperatorStatisticInfoResponse(const Response &response)
    {
        return ToResponseJson<OperatorStatisticInfoResponse>(
                dynamic_cast<const OperatorStatisticInfoResponse &>(response));
    }

    std::optional<document_t> OperatorProtocol::ToOperatorDetailInfoResponse(const Response &response)
    {
        return ToResponseJson<OperatorDetailInfoResponse>(dynamic_cast<const OperatorDetailInfoResponse &>(response));
    }

    std::optional<document_t> OperatorProtocol::ToOperatorMoreInfoResponse(const Response &response)
    {
        return ToResponseJson<OperatorMoreInfoResponse>(dynamic_cast<const OperatorMoreInfoResponse &>(response));
    }

    std::optional<document_t> OperatorProtocol::ToOperatorParseStatusEvent(const Event &event)
    {
        return ToEventJson<OperatorParseStatusEvent>(dynamic_cast<const OperatorParseStatusEvent &>(event));
    }

    std::optional<document_t> OperatorProtocol::ToOperatorParseClearEvent(const Event &event)
    {
        return ToEventJson<OperatorParseClearEvent>(dynamic_cast<const OperatorParseClearEvent &>(event));
    }

    std::string OperatorProtocol::GetStatisticColumName(const std::string& originName)
    {
        std::map<std::string, std::string> relation {
                {"opType", "op_type"},
                {"opName", "name"},
                {"inputShape", "input_shapes"},
                {"accCore", "accelerator_core"},
                {"totalTime", "total_time"},
                {"count", "cnt"},
                {"avgTime", "avg_time"},
                {"maxTime", "max_time"},
                {"minTime", "min_time"}
        };
        if (originName.empty() || relation.count(originName) == 0) {
            return "";
        }
        return relation[originName];
    }

    std::string OperatorProtocol::GetDetailColumName(const std::string& originName)
    {
        std::map<std::string, std::string> relation {
                {"rank", "rank_id"},
                {"step", "step_id"},
                {"name", "name"},
                {"type", "op_type"},
                {"accCore", "accelerator_core"},
                {"startTime", "start_time"},
                {"duration", "duration"},
                {"waitTime", "wait_time"},
                {"blockDim", "block_dim"},
                {"inputShape", "input_shapes"},
                {"inputType", "input_data_types"},
                {"inputFormat", "input_formats"},
                {"outputShape", "output_shapes"},
                {"outputType", "output_data_types"},
                {"outputFormat", "output_formats"}
        };
        if (originName.empty() || relation.count(originName) == 0) {
            return "";
        }
        return relation[originName];
    }
}