/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_OPERATORPROTOCOLREQUEST_H
#define PROFILER_SERVER_OPERATORPROTOCOLREQUEST_H

#include <string>
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"

namespace Dic::Protocol {

    enum class QueryType {
        CATEGORY,
        COMPUTE_UNIT
    };

    // 算子视图饼图的请求参数
    struct OperatorDurationReqParams {
        std::string rankId;
        std::string group; // Operator、Operator Type、Input Shape
        int64_t topK = -1;
    };

    // 算子视图表格数据的请求参数
    struct OperatorStatisticReqParams {
        std::string rankId;
        std::string group; // Operator、Operator Type、Input Shape
        int64_t topK;
        int64_t current;
        int64_t pageSize;
        std::string orderBy;
        std::string order;
    };

    // Operator Type、Operator Name + Input Shape类型时See More的请求参数
    struct OperatorMoreInfoReqParams {
        std::string rankId;
        std::string group;
        int64_t topK;
        std::string opType;
        std::string opName;
        std::string shape;
        int64_t current;
        int64_t pageSize;
        std::string orderBy;
        std::string order;
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
