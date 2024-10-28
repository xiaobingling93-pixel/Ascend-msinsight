/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/
#include "QueryOneKernelHandler.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "ProjectExplorerManager.h"
#include "TraceTime.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;

bool QueryOneKernelHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    KernelRequest &request = dynamic_cast<KernelRequest &>(*requestPtr.get());
    std::unique_ptr<OneKernelResponse> responsePtr = std::make_unique<OneKernelResponse>();
    OneKernelResponse &response = *responsePtr.get();
    WsSession &session = *WsSessionManager::Instance().GetSession();
    SetBaseResponse(request, response);
    uint64_t minTimestamp = TraceTime::Instance().GetStartTime();
    std::string warnMsg;
    SetResponseResult(response, false);
    if (!request.params.CheckParams(minTimestamp, warnMsg)) {
        ServerLog::Warn(warnMsg);
        SetResponseResult(response, false, warnMsg);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    if (request.projectName.empty()) {
        ServerLog::Error("project name is empty");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        database = Timeline::DataBaseManager::Instance().GetTraceDatabaseWithOutHost(request.params.rankId);
        if (database == nullptr) {
            ServerLog::Error("Query one kernel failed to get connection.");
            session.OnResponse(std::move(responsePtr));
            return false;
        }
    }
    if (!database->QueryKernelDepthAndThread(request.params, response.body, TraceTime::Instance().GetStartTime())) {
        ServerLog::Error("Failed to query the operator response data.");
    }

    // 判断是否具备集群数据并且包含集群文件
    if (!Global::ProjectExplorerManager::Instance().IsClusterData(request.projectName)) {
        session.OnResponse(std::move(responsePtr));
        return false;
    }

    // 根据通信算子name,startTime查询step、group
    auto clusterDatabase = DataBaseManager::Instance().GetReadClusterDatabase();
    if (database == nullptr) {
        ServerLog::Error("Query one kernel failed to get cluster connection.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    if (!clusterDatabase->QueryIterationAndCommunicationGroup(request.params, response.body,
        TraceTime::Instance().GetStartTime())) {
        ServerLog::Error("Failed to query the operator group and step response data.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }

    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
    return true;
}

} // Timeline
} // Module
} // Dic