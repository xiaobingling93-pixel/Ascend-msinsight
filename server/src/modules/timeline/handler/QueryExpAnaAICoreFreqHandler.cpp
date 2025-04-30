/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "QueryExpAnaAICoreFreqHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
bool QueryExpAnaAICoreFreqHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    ExpAnaAICoreFreqRequest &request = dynamic_cast<ExpAnaAICoreFreqRequest &>(*requestPtr.get());
 
    std::unique_ptr<ExpAnaAICoreFreqResponse> responsePtr = std::make_unique<ExpAnaAICoreFreqResponse>();
    WsSession &session = *WsSessionManager::Instance().GetSession();
    ExpAnaAICoreFreqResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Query system view AI core freq failed to get connection.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    uint64_t maxFreq = 0;
    uint64_t minFreq = UINT64_MAX;
    std::vector<std::pair<uint64_t, uint64_t>> freqs;
    if (!database->QueryExpAnaAICoreFreqData(freqs, maxFreq, minFreq) || freqs.empty()) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get system view AI core freq table response data.");
    }
    for (auto freq : freqs) {
        maxFreq = std::max(maxFreq, freq.second);
        minFreq = std::min(minFreq, freq.second);
    }
    if (maxFreq != 0) {
        // 计算百分比需要乘以100
        response.body.percent = ((maxFreq - minFreq) / maxFreq) * 100;
    }
    if (response.body.percent > 5) { // 降频百分比超过5，需要提示用户关注
        response.body.hasProblem = true;
    }
 
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    return true;
}

} // Timeline
} // Module
} // Dic