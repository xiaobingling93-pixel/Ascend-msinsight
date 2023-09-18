/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYCOMMUNICATIONDETAILINFOHANDLER_H
#define PROFILER_SERVER_QUERYCOMMUNICATIONDETAILINFOHANDLER_H

#include "TimelineRequestHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
class QueryCommunicationDetailInfoHandler : public TimelineRequestHandler {
public:
    QueryCommunicationDetailInfoHandler()
    {
        command = Protocol::REQ_RES_COMMUNICATION_DETAIL;
    }
    ~QueryCommunicationDetailInfoHandler() override = default;

    void HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;

    static bool GetResponseData(const Protocol::CommunicationDetailParams& params,
                                CommunicationDetailResponse &response);
    static void OrderBy(const Protocol::CommunicationDetailParams& params,
                        std::vector<Protocol::CommunicationDetail> &details);
private:
    static std::vector<double> res;
};

} // Timeline
} // Module
} // Dic

#endif // PROFILER_SERVER_QUERYCOMMUNICATIONDETAILINFOHANDLER_H
