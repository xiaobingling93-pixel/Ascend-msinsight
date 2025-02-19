/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_OPERATORPROTOCOLREQUEST_H
#define PROFILER_SERVER_OPERATORPROTOCOLREQUEST_H

#include <string>
#include <vector>
#include "ProtocolDefs.h"
#include "OperatorProtocol.h"
#include "OperatorGroupConverter.h"

namespace Dic::Protocol {

    enum class QueryType {
        CATEGORY,
        COMPUTE_UNIT
    };

    // 算子视图饼图的请求参数
    struct OperatorDurationReqParams {
        std::string rankId;
        std::string group; // Operator、Operator Type、Input Shape
        int64_t topK{0};
    };

    // 算子视图表格数据的请求参数
    struct OperatorStatisticReqParams {
        bool isCompare{false};
        std::string rankId;
        std::string group; // Operator、Operator Type、Input Shape
        int64_t topK{0};
        int64_t current;
        int64_t pageSize;
        std::string orderBy;
        std::string order;
        std::vector<std::pair<std::string, std::string>> filters;
        bool CommonCheck(std::string &errorMsg)
        {
            // 查询值 小于-1,是异常值不需要再走查询，减少耗时
            if (this->topK < -1) {
                errorMsg = "[Operator]Failed to check topK in Query Op Statistic Info.";
                return false;
            }
            if (!CheckPageValid(this->pageSize, this->current, errorMsg)) {
                return false;
            }
            if (!CheckStrParamValid(rankId, errorMsg)) {
                errorMsg = std::string("[Operator]Failed to check rankId in Query Op Statistic Info.") + errorMsg;
                return false;
            }
            if (!this->orderBy.empty()) {
                if (OperatorProtocol::GetStatisticColumName(this->orderBy).empty() &&
                    OperatorProtocol::GetDetailColumName(this->orderBy).empty()) {
                    errorMsg = "[Operator]Failed to check orderBy in Query Op Statistic Info.";
                    return false;
                }
                this->orderBy = OperatorProtocol::GetStatisticColumName(this->orderBy) != "" ?
                                OperatorProtocol::GetStatisticColumName(this->orderBy) :
                                OperatorProtocol::GetDetailColumName(this->orderBy);
            }
            return true;
        }
        bool StatisticGroupCheck(std::string &errorMsg)
        {
            OperatorGroupConverter::OperatorGroup operatorGroup = Protocol::OperatorGroupConverter::ToEnum(this->group);
            if (operatorGroup != OperatorGroupConverter::OperatorGroup::OP_TYPE_GROUP &&
                operatorGroup != OperatorGroupConverter::OperatorGroup::COMMUNICATION_TYPE_GROUP &&
                operatorGroup != OperatorGroupConverter::OperatorGroup::OP_INPUT_SHAPE_GROUP) {
                errorMsg = "[Operator]Wrong group type in Query Op Statistic Info.";
                return false;
            }
            return true;
        }
    };

    // Operator Type、Operator Name + Input Shape类型时See More的请求参数
    struct OperatorMoreInfoReqParams {
        std::string rankId;
        std::string group;
        int64_t topK;
        std::string opType;
        std::string opName;
        std::string shape;
        std::string accCore;
        int64_t current;
        int64_t pageSize;
        std::string orderBy;
        std::string order;
        std::vector<std::pair<std::string, std::string>> filters;
    };

    // 获取算子按类型耗时信息
    struct OperatorCategoryInfoRequest : public Request {
        OperatorCategoryInfoRequest() : Request(REQ_RES_OPERATOR_CATEGORY_INFO) {};
        OperatorDurationReqParams params;
    };

    // 获取算子按计算单元耗时信息
    struct  OperatorComputeUnitInfoRequest : public Request {
        OperatorComputeUnitInfoRequest() : Request(REQ_RES_OPERATOR_COMPUTE_UNIT_INFO) {};
        OperatorDurationReqParams params;
    };

    // 获取算子按Op Type或者Op Name + Input Shape统计信息
    struct OperatorStatisticInfoRequest : public Request {
        OperatorStatisticInfoRequest() : Request(REQ_RES_OPERATOR_STATISTIC_INFO) {};
        OperatorStatisticReqParams params;
    };

    // 获取算子全量信息
    struct OperatorDetailInfoRequest : public Request {
        OperatorDetailInfoRequest() : Request(REQ_RES_OPERATOR_DETAIL_INFO) {};
        OperatorStatisticReqParams params;
    };

    // Op Type或者Op Name + Input Shape中See More请求
    struct OperatorMoreInfoRequest : public Request {
        OperatorMoreInfoRequest() : Request(REQ_RES_OPERATOR_MORE_INFO) {};
        OperatorMoreInfoReqParams params;
    };
}

#endif // PROFILER_SERVER_OPERATORPROTOCOLREQUEST_H
