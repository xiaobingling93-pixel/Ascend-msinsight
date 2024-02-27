/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "DbMemoryDataBase.h"
#include "WsSessionManager.h"
#include "ServerLog.h"

namespace Dic {
namespace Module {
namespace FullDb {

std::map<std::string, Protocol::MemorySuccess> FullDb::DbMemoryDataBase::ranks = {};

bool DbMemoryDataBase::QueryOperatorDetail(Protocol::MemoryOperatorParams &requestParams,
                                           std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                                           std::vector<Protocol::MemoryOperator> &opDetails)
{
    return false;
}

bool DbMemoryDataBase::QueryMemoryView(Protocol::MemoryComponentParams &requestParams,
                                       Protocol::MemoryViewData &operatorBody)
{
    return false;
}

bool DbMemoryDataBase::QueryOperatorsTotalNum(Protocol::MemoryOperatorParams &requestParams, int64_t &totalNum)
{
    return false;
}

bool DbMemoryDataBase::QueryOperatorSize(double &min, double &max)
{
    return false;
}

void DbMemoryDataBase::ParserEnd(std::string rankId, bool result)
{
    if (!result) {
        return;
    }
    Server::ServerLog::Error(&"[Memory] ParserEnd"[ranks.size()]);
    if (ranks.count(rankId) == 0) {
        Protocol::MemorySuccess success;
        success.rankId = rankId;
        success.parseSuccess = true;
        success.hasFile = true;
        ranks.emplace(rankId, success);
    } else {
        ranks[rankId].parseSuccess = true;
        ranks[rankId].hasFile = true;
    }
}

void DbMemoryDataBase::ParseCallBack(const std::string &token, const std::string &fileId, bool result,
                                     const std::string &msg)
{
    Server::WsSession *session = Server::WsSessionManager::Instance().GetSession(token);
    if (session == nullptr) {
        Server::ServerLog::Error("[Memory]Failed to get session token");
        return;
    }

    auto event = std::make_unique<Protocol::ParseMemoryCompletedEvent>();
    event->moduleName = Protocol::ModuleType::TIMELINE;
    event->token = token;
    event->result = result;
    event->isCluster = true;
    std::vector<Protocol::MemorySuccess> memoryResult;
    for (const auto &[rank, info]: ranks) {
        memoryResult.push_back(info);
    }
    event->memoryResult = memoryResult;
    session->OnEvent(std::move(event));
}

std::map<std::string, Protocol::MemorySuccess> DbMemoryDataBase::GetRanks()
{
    return ranks;
}

}
}
}

