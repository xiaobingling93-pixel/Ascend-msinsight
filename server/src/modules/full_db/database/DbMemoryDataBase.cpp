/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "DbMemoryDataBase.h"
#include "WsSessionManager.h"
#include "ServerLog.h"
#include "TraceTime.h"
#include "TableDefs.h"
#include "DataBaseManager.h"

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
    std::string sql = "";
    FileType type = DataBaseManager::Instance().GetFileType();
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    if (type == FileType::PYTORCH) {
        sql += "SELECT NAME.value AS realName, ROUND(size / 1024.0, 2) as size, "
               " CASE WHEN allocation_time == 0 THEN 'NA' ELSE "
            "ROUND((allocation_time - " + std::to_string(startTime) +
            ") / (1000.0 * 1000.0), 2) END AS allocationTime, "
            "CASE WHEN release_time == 0 THEN 'NA' ELSE ROUND((release_time - " + std::to_string(startTime) +
            ") / (1000.0 * 1000.0), 2) END AS release_time, ROUND(duration / (1000.0 * 1000.0), 2) as duration, "
            "CASE WHEN active_release_time == 0 THEN 'NA' ELSE ROUND((active_release_time - " +
            std::to_string(startTime) + ") / (1000.0 * 1000.0), 2) "
            "END AS activeReleaseTime, ROUND(active_duration / (1000.0 * 1000.0), 2) as active_duration, "
            "ROUND(allocation_total_allocated / (1024.0 * 1024.0), 11) as allocation_allocated, "
            " ROUND(allocation_total_reserved / (1024.0 * 1024.0), 11) as allocation_reserve, "
            "ROUND(allocation_total_active / (1024.0 * 1024.0), 11) as allocation_active, "
            " ROUND(release_total_allocated / (1024.0 * 1024.0), 11) as release_allocated, "
            "ROUND(release_total_reserved / (1024.0 * 1024.0), 11) as release_reserve, "
            "ROUND(release_total_active / (1024.0 * 1024.0), 11) as release_active, stream_ptr as stream FROM " +
            TABLE_OPERATOR_MEMORY + " JOIN STRING_IDS AS NAME ON NAME.id = OP_MEMORY.name"
            " WHERE realName LIKE ? ";
    }
    AddOperatorSql(requestParams, sql);
    return ExecuteOperatorDetail(requestParams, columnAttr, opDetails, sql);
}

bool DbMemoryDataBase::QueryMemoryView(Protocol::MemoryComponentParams &requestParams,
                                       Protocol::MemoryViewData &operatorBody)
{
    std::string sql = "";
    FileType type = DataBaseManager::Instance().GetFileType();
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    if (type == FileType::PYTORCH) {
        sql += "SELECT NAME.value AS component, ROUND((time_stamp - " +
            std::to_string(startTime) +
            ") / (1000.0 * 1000.0), 2) as timestamp, "
            "ROUND(total_allocated / (1024.0 * 1024.0), 11) as total_allocated, "
            " ROUND(total_reserved / (1024.0 * 1024.0), 11) as total_reserve, "
            "ROUND(total_active / (1024.0 * 1024.0), 11) as total_active, stream_ptr as stream FROM " +
            TABLE_MEMORY_RECORD + " JOIN STRING_IDS AS NAME ON NAME.id = MEMORY_RECORD.component where 1=1 ";
    }
    return ExecuteQueryMemoryView(requestParams, operatorBody, sql);
}

bool DbMemoryDataBase::QueryOperatorsTotalNum(Protocol::MemoryOperatorParams &requestParams, int64_t &totalNum)
{
    std::string sql = "";
    FileType type = DataBaseManager::Instance().GetFileType();
    if (type == FileType::PYTORCH) {
        sql = "SELECT count(*) as nums FROM "
            " ("
            "   SELECT NAME.value as name, stream_ptr, allocation_time,"
            " ROUND(size / 1024.0, 2) as size, size as realSize FROM OP_MEMORY JOIN STRING_IDS AS NAME ON "
            "   NAME.id = OP_MEMORY.name"
            ") "
            " WHERE name LIKE ? ";
    }

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
        sql += " AND realSize >= ? ";
    }
    if (requestParams.maxSize != -1) {
        sql += " AND realSize <= ? ";
    }
    return ExecuteOperatorsTotalNum(requestParams, totalNum, sql);
}

bool DbMemoryDataBase::QueryOperatorSize(double &min, double &max, std::string rankId)
{
    FileType type = DataBaseManager::Instance().GetFileType();
    std::string sql = "";
    if (type == FileType::PYTORCH) {
        sql += "SELECT ROUND(min(size)/ 1024.0, 2) as minSize, "
               " ROUND(max(size)/ 1024.0, 2) as maxSize FROM " + TABLE_OPERATOR_MEMORY;
    }
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

