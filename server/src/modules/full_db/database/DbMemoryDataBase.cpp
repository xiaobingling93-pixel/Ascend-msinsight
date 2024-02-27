/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "DbMemoryDataBase.h"
#include "WsSessionManager.h"
#include "ServerLog.h"
#include "TraceTime.h"
#include "TableDefs.h"

namespace Dic {
namespace Module {
namespace FullDb {
using namespace Dic::Server;
using namespace Dic::Module::Timeline;

std::map<std::string, Protocol::MemorySuccess> FullDb::DbMemoryDataBase::ranks = {};

bool DbMemoryDataBase::QueryOperatorDetail(Protocol::MemoryOperatorParams &requestParams,
                                           std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                                           std::vector<Protocol::MemoryOperator> &opDetails)
{
    std::string ascend;
    if (requestParams.order == "ascend") {
        ascend = "ASC";
    } else {
        ascend = "DESC";
    }
    std::string sql =
            "SELECT name, size, CASE WHEN allocation_time == 0 THEN 'NA' ELSE "
            "ROUND((allocation_time- ?) / (1000.0 * 1000.0), 2) END AS allocationTime, "
            "CASE WHEN release_time == 0 THEN 'NA' ELSE ROUND((release_time - ?) / (1000.0 * 1000.0), 2) "
            "END AS releaseTime, ROUND(duration / 1000.0, 2) as duration, "
            "CASE WHEN active_release_time == 0 THEN 'NA' ELSE ROUND((active_release_time - ?) / (1000.0 * 1000.0), 2) "
            "END AS activeReleaseTime, ROUND(active_duration / 1000.0, 2) as active_duration, "
            "allocation_total_allocated as allocation_allocated, allocation_total_reserved as allocation_reserve, "
            "allocation_total_active as allocation_active, release_total_allocated as release_allocated, "
            "release_total_reserved as release_reserve, "
            "release_total_active as release_active, stream_ptr as stream FROM " + TABLE_OPERATOR_MEMORY +
            " WHERE name LIKE ? AND rank_id == " + requestParams.rankId;

    if (requestParams.type == Protocol::MEMORY_STREAM_GROUP) {
        sql += " AND stream <> ''";
    }
    if (requestParams.startTime != -1) {
        sql += " AND allocationTime >= " + std::to_string(requestParams.startTime);
    }
    if (requestParams.endTime != -1) {
        sql += " AND allocationTime <= " + std::to_string(requestParams.endTime);
    }

    if (requestParams.minSize != -1) {
        sql += " AND size >= " + std::to_string(requestParams.minSize);
    }
    if (requestParams.maxSize != -1) {
        sql += " AND size <= " + std::to_string(requestParams.maxSize);
    }
    if (!requestParams.orderBy.empty()) {
        sql += " ORDER BY " + requestParams.orderBy + " " + ascend;
    }
    sql += " LIMIT ? offset ?";
    return ExecuteOperatorDetail(requestParams, columnAttr, opDetails, sql);
}

bool DbMemoryDataBase::QueryMemoryView(Protocol::MemoryComponentParams &requestParams,
                                       Protocol::MemoryViewData &operatorBody)
{
    std::string sql = "SELECT component, ROUND((time_stamp - ?) / (1000.0 * 1000.0), 2) as timestamp, "
                      "ROUND(total_allocated, 2) as total_allocated, ROUND(total_reserve, 2) as total_reserve, "
                      "ROUND(total_active, 2) as total_active, stream_ptr as stream FROM " + TABLE_MEMORY_RECORD +
                      " WHERE rank_id == " + requestParams.rankId;
    return ExecuteQueryMemoryView(requestParams, operatorBody, sql);
}

bool DbMemoryDataBase::QueryOperatorsTotalNum(Protocol::MemoryOperatorParams &requestParams, int64_t &totalNum)
{
    std::string sql = "SELECT count(*) as nums FROM " + TABLE_OPERATOR_MEMORY +
            " WHERE name LIKE ? AND rank_id == " + requestParams.rankId;

    if (requestParams.type == Protocol::MEMORY_STREAM_GROUP) {
        sql += " AND stream_ptr <> ''";
    }
    if (requestParams.startTime != -1) {
        sql += " AND ROUND((allocation_time - ?) / (1000.0 * 1000.0), 2) >= ? ";
    }
    if (requestParams.endTime != -1) {
        sql += " AND ROUND((allocation_time - ?) / (1000.0 * 1000.0), 2) <= ? ";
    }
    if (requestParams.minSize != -1) {
        sql += " AND size >= ? ";
    }
    if (requestParams.maxSize != -1) {
        sql += " AND size <= ? ";
    }
    return ExecuteOperatorsTotalNum(requestParams, totalNum, sql);
}

bool DbMemoryDataBase::QueryOperatorSize(double &min, double &max, std::string rankId)
{
    std::string sql = "SELECT min(size) as minSize, max(size) as maxSize FROM " + TABLE_OPERATOR_MEMORY +
            " WHERE rank_id == " + rankId;
    return ExecuteOperatorSize(min, max, sql);
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

