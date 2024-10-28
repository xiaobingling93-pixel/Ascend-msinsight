/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */
#include "pch.h"
#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "BandwidthHandler.h"

namespace Dic {
namespace Module {
namespace Communication {
using namespace Dic;
using namespace Dic::Server;

bool BandwidthHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    Protocol::BandwidthDataRequest &request = dynamic_cast<Protocol::BandwidthDataRequest &>(*requestPtr.get());
    std::unique_ptr<Protocol::BandwidthDataResponse> responsePtr =
            std::make_unique<Protocol::BandwidthDataResponse>();
    BandwidthDataResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    WsSession &session = *WsSessionManager::Instance().GetSession();

    auto database = Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
    if (!database->QueryBandwidthData(request.params, response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get communication bandwidth data.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // Communication
} // Module
} // Dic