/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "QueryCommunicationDetailInfoHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceTime.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;

std::vector<double> QueryCommunicationDetailInfoHandler::res;

bool QueryCommunicationDetailInfoHandler::GetResponseData(const Protocol::CommunicationDetailParams& params,
                                                          CommunicationDetailResponse &response)
{
    std::string threadName = "Group 0 Communication";
    std::string notOverlap = "Communication(Not Overlapped)";
    auto database = Timeline::DataBaseManager::Instance().GetTraceDatabase(params.rankId);
    int64_t opTrackId = database->GetTrackIdList(threadName);
    int64_t notOverlapTrackId = database->GetTrackIdList(notOverlap);
    response.totalNum = database->QueryCommunicationTotalNum(std::to_string(opTrackId));

    if (!database->GetCommunicationDetails(opTrackId, response.communication)) {
        ServerLog::Error("Failed to get communication detail.");
        return false;
    }
    for (Protocol::CommunicationDetail &detail: response.communication) {
        double duration = detail.totalDuration;
        double timestamp = detail.startTime + static_cast<double>(Timeline::TraceTime::Instance().GetStartTime());
        double totalTime = 0;
        res = database->QueryNotOverlapTime(notOverlapTrackId, timestamp, duration);
        for (double re: res) {
            totalTime += re;
        }
        detail.notOverlapDuration = totalTime;
        detail.overlapDuration = duration - totalTime;
    }
    OrderBy(params, response.communication);
    return true;
}

// ASC从小到大排序
bool CompareByKernel(CommunicationDetail detail1, CommunicationDetail detail2)
{
    if (detail1.communicationKernel != detail2.communicationKernel) {
        return detail1.communicationKernel < detail2.communicationKernel;
    } else {
        return detail1.startTime < detail2.startTime;
    }
}

bool CompareByStart(CommunicationDetail detail1, CommunicationDetail detail2)
{
    if (detail1.startTime != detail2.startTime) {
        return detail1.startTime < detail2.startTime;
    } else {
        return detail1.communicationKernel < detail2.communicationKernel;
    }
}

bool CompareByOverlapped(CommunicationDetail detail1, CommunicationDetail detail2)
{
    if (detail1.overlapDuration != detail2.overlapDuration) {
        return detail1.overlapDuration < detail2.overlapDuration;
    } else {
        return detail1.communicationKernel < detail2.communicationKernel;
    }
}

bool CompareByNotOverlapped(CommunicationDetail detail1, CommunicationDetail detail2)
{
    if (detail1.notOverlapDuration != detail2.notOverlapDuration) {
        return detail1.notOverlapDuration < detail2.notOverlapDuration;
    } else {
        return detail1.communicationKernel < detail2.communicationKernel;
    }
}

void QueryCommunicationDetailInfoHandler::OrderBy(const Protocol::CommunicationDetailParams& params,
                                                  std::vector<Protocol::CommunicationDetail> &details)
{
    if (params.orderBy == "communicationKernel") {
        std::sort(details.begin(), details.end(), CompareByKernel);
    } else if (params.orderBy == "startTime") {
        std::sort(details.begin(), details.end(), CompareByStart);
    } else if (params.orderBy == "totalDuration") {
        std::sort(details.begin(), details.end(), CompareByOverlapped);
    } else if (params.orderBy == "notOverlapDuration") {
        std::sort(details.begin(), details.end(), CompareByNotOverlapped);
    }
    if (params.order != "ascend") {
        std::reverse(details.begin(), details.end());
    }
    int32_t offset = (params.currentPage - 1) * params.pageSize;
    int32_t vectorSize = details.size();
    int32_t right = 0;
    if (params.currentPage * params.pageSize + 1 > vectorSize) {
        right = vectorSize;
    } else {
        right = params.currentPage * params.pageSize;
    }
    int32_t left = offset;
    details.erase(details.begin() + right, details.end());
    details.erase(details.begin(), details.begin() + left);
}

void QueryCommunicationDetailInfoHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    CommunicationDetailRequest &request = dynamic_cast<CommunicationDetailRequest &>(*requestPtr.get());
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(token),
                        ", command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<CommunicationDetailResponse> responsePtr = std::make_unique<CommunicationDetailResponse>();
    CommunicationDetailResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    if (!GetResponseData(request.params, response)) {
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

} // Timeline
} // Module
} // Dic