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
    const uint64_t percentLimit = 5;
    ExpAnaAICoreFreqRequest &request = dynamic_cast<ExpAnaAICoreFreqRequest &>(*requestPtr.get());
 
    std::unique_ptr<ExpAnaAICoreFreqResponse> responsePtr = std::make_unique<ExpAnaAICoreFreqResponse>();
    WsSession &session = *WsSessionManager::Instance().GetSession();
    ExpAnaAICoreFreqResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Query system view AI core freq failed to get connection.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    std::string deviceId = DataBaseManager::Instance().GetDeviceIdFromRankId(request.params.rankId, "timeline");
    if (deviceId.empty()) {
        ServerLog::Error("Query system view AI core freq failed to get deviceId.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    request.params.deviceId = deviceId;
    uint64_t maxFreq = 0;
    uint64_t minFreq = UINT64_MAX;
    std::vector<std::pair<uint64_t, uint64_t>> freqs;
    if (!database->QueryExpAnaAICoreFreqData(request.params, response.body, freqs, maxFreq, minFreq)
        || freqs.empty()) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get system view AI core freq table response data.");
    }
    auto dbType = Timeline::DataBaseManager::Instance().GetDataType();
    response.body.rankId = dbType == Timeline::DataType::TEXT ? request.params.rankId : database->GetDbPath();
    if (maxFreq != 0) {
        // 计算百分比需要乘以100
        response.body.percent = static_cast<uint64_t>(((float)(maxFreq - minFreq) / (float)maxFreq) * 100);
    }
    if (response.body.percent > percentLimit) { // 降频百分比超过5，需要提示用户关注
        response.body.hasProblem = true;
    }
 
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    return true;
}

} // Timeline
} // Module
} // Dic