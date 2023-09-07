/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef DIC_SUMMARY_PROTOCOL_SUMMARY_REQUEST_H
#define DIC_SUMMARY_PROTOCOL_SUMMARY_REQUEST_H

#include <string>
#include <optional>
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"
#include <vector>

namespace Dic {
namespace Protocol {
struct SummaryTopRankParams {
    int limit;
    std::vector<std::string> stepIdList;
    std::vector<std::string> rankIdList;
    std::string orderBy;
};

struct SummaryTopRankRequest : public Request {
    SummaryTopRankRequest() : Request(REQ_RES_SUMMARY_QUERY_TOP_DATA) {};
    SummaryTopRankParams params;
};

struct SummaryStatisticParams {
    std::string rankId;
    std::string timeFlag;
    std::string stepId;
};

struct SummaryStatisticRequest : public Request {
    SummaryStatisticRequest() : Request(REQ_RES_SUMMARY_STATISTIC) {};
    SummaryStatisticParams params;
};

struct ComputeDetailParams {
    std::string rankId;
    std::string timeFlag;
    double currentPage = 0;
    double pageSize = 0;
    std::string orderBy;
    std::string order;
};

struct ComputeDetailRequest : public Request {
    ComputeDetailRequest() : Request(REQ_RES_COMPUTE_DETAIL) {};
    ComputeDetailParams params;
};

struct CommunicationDetailParams {
    std::string rankId;
    double currentPage = 0;
    double pageSize = 0;
    std::string orderBy;
    std::string order;
};

struct CommunicationDetailRequest : public Request {
    CommunicationDetailRequest() : Request(REQ_RES_COMMUNICATION_DETAIL) {};
    CommunicationDetailParams params;
};
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_SUMMARY_PROTOCOL_SUMMARY_REQUEST_H