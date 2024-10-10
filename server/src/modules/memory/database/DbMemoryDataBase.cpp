/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "WsSessionManager.h"
#include "TraceTime.h"
#include "TableDefs.h"
#include "DataBaseManager.h"
#include "ProtocolDefs.h"
#include "DbMemoryDataBase.h"

namespace Dic {
namespace Module {
namespace FullDb {
using namespace Dic::Server;
using namespace Dic::Module::Timeline;

std::map<std::string, Protocol::MemorySuccess> FullDb::DbMemoryDataBase::ranks = {};

bool DbMemoryDataBase::OpenDb(const std::string &dbPath, bool clearAllTable)
{
    return Database::OpenDb(dbPath, clearAllTable) && GetMetaVersion();
}

bool DbMemoryDataBase::QueryMemoryType(std::string &type, std::vector<std::string> &graphId)
{
    return ExecuteMemoryType(graphId, type);
}

bool DbMemoryDataBase::QueryMemoryResourceType(std::string &type)
{
    type = "Pytorch";
    return true;
}

bool DbMemoryDataBase::QueryOperatorDetail(Protocol::MemoryOperatorParams &requestParams,
                                           std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                                           std::vector<Protocol::MemoryOperator> &opDetails)
{
    std::string sql = "";
    FileType type = DataBaseManager::Instance().GetFileType();
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    // 单位转换， KB -> B，并作溢出防护。
    if (requestParams.minSize != std::numeric_limits<int64_t>::min()) {
        if (requestParams.minSize > 0 && requestParams.minSize > std::numeric_limits<int64_t>::max() / KB_SIZE) {
            requestParams.minSize = std::numeric_limits<int64_t>::max();
        } else if (requestParams.minSize < 0 && requestParams.minSize < std::numeric_limits<int64_t>::min() / KB_SIZE) {
            requestParams.minSize = std::numeric_limits<int64_t>::min();
        } else {
            requestParams.minSize *= KB_SIZE;
        }
    }
    if (requestParams.maxSize != std::numeric_limits<int64_t>::max()) {
        if (requestParams.maxSize > 0 && requestParams.maxSize > std::numeric_limits<int64_t>::max() / KB_SIZE) {
            requestParams.maxSize = std::numeric_limits<int64_t>::max();
        } else if (requestParams.maxSize < 0 && requestParams.maxSize < std::numeric_limits<int64_t>::min() / KB_SIZE) {
            requestParams.maxSize = std::numeric_limits<int64_t>::min();
        } else {
            requestParams.maxSize *= KB_SIZE;
        }
    }
    uint64_t offsetTime = Timeline::TraceTime::Instance().GetOffsetByFileId(requestParams.rankId);
    if (type == FileType::PYTORCH) {
        sql += "SELECT NAME.value AS realName, ROUND(size / 1024.0, 2) as size, "
               " CASE WHEN allocation_time == 0 THEN 'NA' ELSE "
            "ROUND((allocation_time - " + std::to_string(startTime) + " - " + std::to_string(offsetTime) +
            ") / (1000.0 * 1000.0), 3) END AS allocationTimestamp, "
            "CASE WHEN release_time == 0 THEN 'NA' ELSE ROUND((release_time - " + std::to_string(startTime) +
            " - " + std::to_string(offsetTime) +
            ") / (1000.0 * 1000.0), 3) END AS releaseTimestamp, ROUND(duration / (1000.0 * 1000.0), 3) as duration, "
            "CASE WHEN active_release_time == 0 THEN 'NA' ELSE ROUND((active_release_time - " +
            std::to_string(startTime) + " - " + std::to_string(offsetTime) +
            ") / (1000.0 * 1000.0), 3) "
            "END AS activeReleaseTime, ROUND(active_duration / (1000.0 * 1000.0), 3) as active_duration, "
            "ROUND(allocation_total_allocated / (1024.0 * 1024.0), 2) as allocation_allocated, "
            " ROUND(allocation_total_reserved / (1024.0 * 1024.0), 2) as allocation_reserve, "
            "ROUND(allocation_total_active / (1024.0 * 1024.0), 2) as allocation_active, "
            " ROUND(release_total_allocated / (1024.0 * 1024.0), 2) as release_allocated, "
            "ROUND(release_total_reserved / (1024.0 * 1024.0), 2) as release_reserve, "
            "ROUND(release_total_active / (1024.0 * 1024.0), 2) as release_active, stream_ptr as stream FROM ";
        sql = isLowCamel ? StringUtil::ToCamelCase(sql) : sql;
        sql += TABLE_OPERATOR_MEMORY + " JOIN STRING_IDS AS NAME ON NAME.id = OP_MEMORY.name"
            " WHERE realName LIKE ? ";
    } else {
        ServerLog::Error("Memory tab does not support msprof data.");
        return false;
    }
    AddOperatorSql(requestParams, sql);
    return ExecuteOperatorDetail(requestParams, columnAttr, opDetails, sql);
}

bool DbMemoryDataBase::QueryEntireOperatorTable(std::vector<Protocol::MemoryTableColumnAttr> &columnattr,
                                                std::vector<Protocol::MemoryOperator> &opDetails, std::string rankId,
                                                uint64_t offsetTime)
{
    std::string sql = "";
    FileType type = DataBaseManager::Instance().GetFileType();
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    if (type == FileType::PYTORCH) {
        sql += "SELECT NAME.value AS realName, ROUND(size / 1024.0, 2) as size, "
               " CASE WHEN allocation_time == 0 THEN 'NA' ELSE "
            "ROUND((allocation_time - " + std::to_string(startTime) + " - " + std::to_string(offsetTime) +
            ") / (1000.0 * 1000.0), 3) END AS allocationTimestamp, "
            "CASE WHEN release_time == 0 THEN 'NA' ELSE ROUND((release_time - " + std::to_string(startTime) +
            " - " + std::to_string(offsetTime) +
            ") / (1000.0 * 1000.0), 3) END AS releaseTimestamp, ROUND(duration / (1000.0 * 1000.0), 3) as duration, "
            "CASE WHEN active_release_time == 0 THEN 'NA' ELSE ROUND((active_release_time - " +
            std::to_string(startTime) + " - " + std::to_string(offsetTime) + ") / (1000.0 * 1000.0), 3) "
            "END AS activeReleaseTime, ROUND(active_duration / (1000.0 * 1000.0), 3) as active_duration, "
            "ROUND(allocation_total_allocated / (1024.0 * 1024.0), 2) as allocation_allocated, "
            " ROUND(allocation_total_reserved / (1024.0 * 1024.0), 2) as allocation_reserve, "
            "ROUND(allocation_total_active / (1024.0 * 1024.0), 2) as allocation_active, "
            " ROUND(release_total_allocated / (1024.0 * 1024.0), 2) as release_allocated, "
            "ROUND(release_total_reserved / (1024.0 * 1024.0), 2) as release_reserve, "
            "ROUND(release_total_active / (1024.0 * 1024.0), 2) as release_active, stream_ptr as stream FROM ";
        sql = isLowCamel ? StringUtil::ToCamelCase(sql) : sql;
        sql += TABLE_OPERATOR_MEMORY + " JOIN STRING_IDS AS NAME ON NAME.id = OP_MEMORY.name";
    } else {
        ServerLog::Error("Memory tab does not support msprof data.");
        return false;
    }
    return ExecuteQueryEntireOperatorTable(columnattr, opDetails, sql, rankId);
}

bool DbMemoryDataBase::QueryMemoryView(Protocol::MemoryComponentParams &requestParams,
                                       Protocol::MemoryViewData &operatorBody, uint64_t offsetTime)
{
    std::string sql = "";
    FileType type = DataBaseManager::Instance().GetFileType();
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    if (type == FileType::PYTORCH) {
        sql += "select * from ( ";
        sql += "SELECT NAME.value AS component, ROUND((time_stamp - " +
            std::to_string(startTime) +
            " - " + std::to_string(offsetTime) +
            ") / (1000.0 * 1000.0), 3) as timestamp, "
            "ROUND(total_allocated / (1024.0 * 1024.0), 2) as total_allocated, "
            " ROUND(total_reserved / (1024.0 * 1024.0), 2) as total_reserve, "
            "ROUND(total_active / (1024.0 * 1024.0), 2) as total_active, stream_ptr as stream FROM ";
        sql = isLowCamel ? StringUtil::ToCamelCase(sql) : sql;
        sql += TABLE_MEMORY_RECORD + " JOIN STRING_IDS AS NAME ON NAME.id = MEMORY_RECORD.component ";
        sql += " UNION select 'APP' as component, ROUND((timestampNs - " + std::to_string(startTime) +
                " ) / (1000.0 * 1000.0), 2) as timestampNs, "
               " 0 as total_allocated,  ROUND((hbm + ddr) / (1024.0 * 1024.0), 2) as total_reserve, "
               " 0 as totalActive, '' as stream from NPU_MEM join STRING_IDS as ids on ids.id = type "
               " where value = 'app' ";
        sql += " ) where 1 = 1";
    } else {
        ServerLog::Error("Memory tab does not support msprof data.");
        return false;
    }
    std::vector<Protocol::ComponentDto> componentDtoVec;
    std::vector<std::string> streams;
    if (!ExecuteQueryMemoryViewExecuteSql(requestParams, componentDtoVec, streams, sql)) {
        return false;
    }
    return ExecuteQueryMemoryViewGetGraph(requestParams, componentDtoVec, streams, operatorBody);
}

bool DbMemoryDataBase::QueryOperatorsTotalNum(Protocol::MemoryOperatorParams &requestParams, int64_t &totalNum)
{
    std::string sql = "";
    FileType type = DataBaseManager::Instance().GetFileType();
    if (type == FileType::PYTORCH) {
        sql = "SELECT count(*) as nums FROM "
            " ("
            "   SELECT NAME.value as name, ";
        sql.append(isLowCamel ? "streamPtr, allocationTime, releaseTime," :
            "stream_ptr, allocation_time, release_time,");
        sql.append(" ROUND(size / 1024.0, 2) as size, size as realSize FROM OP_MEMORY JOIN STRING_IDS AS NAME ON "
            "   NAME.id = OP_MEMORY.name"
            ") "
            " WHERE name LIKE ? ");
    } else {
        ServerLog::Error("Memory tab does not support msprof data.");
        return false;
    }

    if (requestParams.type == Protocol::MEMORY_STREAM_GROUP) {
        sql.append(isLowCamel ? " AND streamPtr <> ''" : " AND stream_ptr <> ''");
    }
    if (requestParams.startTime != -1 && requestParams.endTime != -1) {
        sql.append(" AND ((").append(isLowCamel ? "releaseTime" : "release_time")
                .append(" IS NULL OR ROUND((")
                .append(isLowCamel ? "releaseTime" : "release_time").append(" - ?) / (1000.0 * 1000.0), 3) >= ? )")
                .append(" AND ROUND((").append(isLowCamel ? "allocationTime" : "allocation_time")
                .append(" - ?) / (1000.0 * 1000.0), 3) <= ?) ");
    }
    if (requestParams.minSize != std::numeric_limits<int64_t>::min()) {
        sql += " AND realSize >= ? ";
    }
    if (requestParams.maxSize != std::numeric_limits<int64_t>::max()) {
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
    } else {
        ServerLog::Error("Memory tab does not support msprof data.");
        return false;
    }
    return ExecuteOperatorSize(min, max, sql);
}

bool DbMemoryDataBase::QueryStaticOperatorsTotalNum(Protocol::StaticOperatorListParams &requestParams,
                                                    int64_t &totalNum)
{
    return false;
}

bool DbMemoryDataBase::QueryStaticOperatorList(Protocol::StaticOperatorListParams &requestParams,
                                               std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                                               std::vector<Protocol::StaticOperatorItem> &opDetails)
{
    return false;
}

bool DbMemoryDataBase::QueryEntireStaticOperatorTable(Protocol::StaticOperatorListParams& requestParams,
                                                      std::vector<Protocol::MemoryTableColumnAttr>& columnAttr,
                                                      std::vector<Protocol::StaticOperatorItem>& opDetails)
{
    return false;
}

bool DbMemoryDataBase::QueryStaticOperatorGraph(Protocol::StaticOperatorGraphParams &requestParams,
                                                Protocol::StaticOperatorGraphItem &graphItem)
{
    return false;
}

void DbMemoryDataBase::ParserEnd(std::string rankId, bool result)
{
    if (!result) {
        return;
    }

    Server::ServerLog::Info("[Memory]Parser ends, Rank ID: ", rankId);
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

// 输入rankId为空时，会清空历史结果
void DbMemoryDataBase::ParseCallBack(const std::string &fileId, bool result, const std::string &msg)
{
    Server::WsSession *session = Server::WsSessionManager::Instance().GetSession();
    if (session == nullptr) {
        Server::ServerLog::Error("[Memory]Failed to get session.");
        return;
    }

    if (fileId.empty()) {
        ranks.clear();
        auto event = std::make_unique<Protocol::ModuleResetEvent>();
        event->moduleName = Protocol::MODULE_MEMORY;
        event->result = true;
        event->reset = true;
        session->OnEvent(std::move(event));
    } else {
        auto event = std::make_unique<Protocol::ParseMemoryCompletedEvent>();
        event->moduleName = Protocol::MODULE_TIMELINE;
        event->result = result;
        event->isCluster = true;
        std::vector<Protocol::MemorySuccess> memoryResult;
        for (const auto& pair : ranks) {
            memoryResult.push_back(pair.second);
        }
        event->memoryResult = memoryResult;
        session->OnEvent(std::move(event));
    }
}

std::map<std::string, Protocol::MemorySuccess> DbMemoryDataBase::GetRanks()
{
    return ranks;
}

void DbMemoryDataBase::Reset()
{
    ServerLog::Info("Memory reset. Wait task completed.");
    ranks.clear();
    ServerLog::Info("Memory task completed.");
    auto databaseList = Timeline::DataBaseManager::Instance().GetAllMemoryDatabase();
    for (auto &db: databaseList) {
        auto database = dynamic_cast<DbMemoryDataBase*>(db);
        if (database != nullptr) {
            database->CloseDb();
        }
    }
    Timeline::DataBaseManager::Instance().Clear(Timeline::DatabaseType::MEMORY);
}

}
}
}

