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
    std::string sql = "SELECT * FROM (";
    FileType type = DataBaseManager::Instance().GetFileType();
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    sql += "SELECT NAME.value AS name, ALLOCATE.size, CASE WHEN ALLOCATE.timestampNs == 0 THEN 'NA' ELSE "
        "ROUND((ALLOCATE.timestampNs - " +
        std::to_string(startTime) + ") / (1000.0 * 1000.0), 2) END AS allocationTime, "
        "CASE WHEN RELEASE_.timestampNs == 0 THEN 'NA' ELSE ROUND((RELEASE_.timestampNs - " +
        std::to_string(startTime) + ") / (1000.0 * 1000.0), 2) "
        "END AS release_time, ROUND(RELEASE_.timestampNs - ALLOCATE.timestampNs / 1000.0, 2) as duration, "
        "'NA' AS activeReleaseTime, 'NA' as active_duration, "
        "ALLOCATE.totalAllocate as allocation_allocated, ALLOCATE.totalReserve as allocation_reserve, "
        "'NA' as allocation_active, RELEASE_.totalAllocate as release_allocated, "
        "RELEASE_.totalReserve as release_reserve, "
        "'NA' as release_active, ALLOCATE.addr as stream FROM " + TABLE_GE_MEMORY + " AS ALLOCATE  "
        " JOIN  NPU_OP_MEM AS RELEASE_ ON RELEASE_.type = (SELECT id FROM ENUM_MEMORY WHERE name = 'release') AND "
        " RELEASE_.addr = ALLOCATE.addr "
        " JOIN STRING_IDS AS NAME ON NAME.id = ALLOCATE.operatorName"
        " WHERE ALLOCATE.type == (SELECT id FROM ENUM_MEMORY WHERE name = 'allocate')";
    if (type == FileType::MS_PROF) {
        sql += " AND ALLOCATE.deviceId = " + requestParams.rankId;
    } else if (type == FileType::PYTORCH) {
        sql += "   UNION "
            "SELECT NAME.value AS name, size, CASE WHEN allocation_time == 0 THEN 'NA' ELSE "
            "ROUND((allocation_time - " + std::to_string(startTime) +
            ") / (1000.0 * 1000.0), 2) END AS allocationTime, "
            "CASE WHEN release_time == 0 THEN 'NA' ELSE ROUND((release_time - " + std::to_string(startTime) +
            ") / (1000.0 * 1000.0), 2) END AS release_time, ROUND(duration / 1000.0, 2) as duration, "
            "CASE WHEN active_release_time == 0 THEN 'NA' ELSE ROUND((active_release_time - " +
            std::to_string(startTime) + ") / (1000.0 * 1000.0), 2) "
            "END AS activeReleaseTime, ROUND(active_duration / 1000.0, 2) as active_duration, "
            "allocation_total_allocated as allocation_allocated, allocation_total_reserved as allocation_reserve, "
            "allocation_total_active as allocation_active, release_total_allocated as release_allocated, "
            "release_total_reserved as release_reserve, "
            "release_total_active as release_active, stream_ptr as stream FROM " +
            TABLE_OPERATOR_MEMORY + " JOIN STRING_IDS AS NAME ON NAME.id = OP_MEMORY.name";
    }
    sql += ") WHERE name LIKE ? ";
    AddOperatorSql(requestParams, sql);
    return ExecuteOperatorDetail(requestParams, columnAttr, opDetails, sql);
}

bool DbMemoryDataBase::QueryMemoryView(Protocol::MemoryComponentParams &requestParams,
                                       Protocol::MemoryViewData &operatorBody)
{
    std::string sql = "";
    FileType type = DataBaseManager::Instance().GetFileType();
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    sql = "SELECT NAME.value AS component, ROUND((timestampNs - " + std::to_string(startTime) +
        ") / (1000.0 * 1000.0), 2) as timestamp, "
        "ROUND(totalAllocate, 2) as total_allocated, ROUND(totalReserve, 2) as total_reserve, "
        "'NA' as total_active, addr as stream FROM " +
        TABLE_GE_MEMORY + " JOIN STRING_IDS AS NAME ON NAME.id = NPU_OP_MEM.component ";
    if (type == FileType::MS_PROF) {
        sql += " AND NPU_OP_MEM.deviceId = " + requestParams.rankId;
    } else if (type == FileType::PYTORCH) {
        sql += "   UNION "
            "SELECT NAME.value AS component, ROUND((time_stamp - " +
            std::to_string(startTime) +
            ") / (1000.0 * 1000.0), 2) as timestamp, "
            "ROUND(total_allocated, 2) as total_allocated, ROUND(total_reserved, 2) as total_reserve, "
            "ROUND(total_active, 2) as total_active, stream_ptr as stream FROM " +
            TABLE_MEMORY_RECORD + " JOIN STRING_IDS AS NAME ON NAME.id = MEMORY_RECORD.component";
    }
    return ExecuteQueryMemoryView(requestParams, operatorBody, sql);
}

bool DbMemoryDataBase::QueryOperatorsTotalNum(Protocol::MemoryOperatorParams &requestParams, int64_t &totalNum)
{
    std::string sql = "";
    FileType type = DataBaseManager::Instance().GetFileType();
    if (type == FileType::MS_PROF) {
        sql = "SELECT count(*) as nums FROM "
            " ("
            "   SELECT NAME.value as name, addr as stream_ptr, timestampNs as allocation_time, size, deviceId FROM " +
                TABLE_GE_MEMORY +
            "   JOIN STRING_IDS AS NAME ON NAME.id = NPU_OP_MEM.operatorName"
            ") "
            " WHERE name LIKE ? AND deviceId = " + requestParams.rankId;
    } else if (type == FileType::PYTORCH) {
        sql = "SELECT count(*) as nums FROM "
            " ("
            "   SELECT NAME.value as name, addr as stream_ptr, timestampNs as allocation_time, size FROM NPU_OP_MEM "
            "   JOIN STRING_IDS AS NAME ON NAME.id = NPU_OP_MEM.operatorName"
            "   UNION ALL "
            "   SELECT NAME.value as name, stream_ptr, allocation_time, size FROM OP_MEMORY JOIN STRING_IDS AS NAME ON "
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
        sql += " AND size >= ? ";
    }
    if (requestParams.maxSize != -1) {
        sql += " AND size <= ? ";
    }
    return ExecuteOperatorsTotalNum(requestParams, totalNum, sql);
}

bool DbMemoryDataBase::QueryOperatorSize(double &min, double &max, std::string rankId)
{
    std::string sql = "IF EXISTS (SELECT * FROM sys.tables WHERE name = 'OP_MEMORY') "
                      "BEGIN "
                      "SELECT min(size) as minSize, max(size) as maxSize FROM ( "
                      "  SELECT size FROM " + TABLE_GE_MEMORY +
                      "  UNION "
                      "  SELECT size FROM " + TABLE_OPERATOR_MEMORY +
                      ") AS combined_table; "
                      "END "
                      "ELSE "
                      "BEGIN "
                      "SELECT min(size) as minSize, max(size) as maxSize FROM NPU_OP_MEM "
                      "END";
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

