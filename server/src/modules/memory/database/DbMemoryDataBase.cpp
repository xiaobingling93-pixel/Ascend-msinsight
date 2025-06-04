/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "WsSender.h"
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
    return Database::OpenDb(dbPath, clearAllTable) && QueryMetaVersion();
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

std::string DbMemoryDataBase::BuildOperatorDetailSql(
    const std::string& startTimeString, const std::string& offsetTimeString)
{
    std::string sql;
    const std::string tempSql = "SELECT OP_MEMORY.rowid AS id, ";
    // 在 db 情况下 allocation_time release_time 不可能为 0，不用再判断
    sql += "NAME.value AS realName, "
        "ROUND(size / 1024.0, 2) as size, "
        "CASE WHEN allocation_time IS NULL THEN 'NA' "
        "ELSE ROUND((allocation_time - " + startTimeString +
            " - " + offsetTimeString + ") / (1000.0 * 1000.0), 3) END AS allocationTimestamp, "
        "CASE WHEN release_time IS NULL THEN 'NA' "
        "ELSE ROUND((release_time - " + startTimeString +
            " - " + offsetTimeString + ") / (1000.0 * 1000.0), 3) END AS releaseTimestamp, "
        "ROUND(duration / (1000.0 * 1000.0), 3) as duration, "
        "CASE WHEN active_release_time IS NULL THEN 'NA' "
        "ELSE ROUND((active_release_time - " + startTimeString +
            " - " + offsetTimeString + ") / (1000.0 * 1000.0), 3) END AS activeReleaseTime, "
        "ROUND(active_duration / (1000.0 * 1000.0), 3) as active_duration, "
        "ROUND(allocation_total_allocated / (1024.0 * 1024.0), 2) as allocation_allocated, "
        "ROUND(allocation_total_reserved / (1024.0 * 1024.0), 2) as allocation_reserve, "
        "ROUND(allocation_total_active / (1024.0 * 1024.0), 2) as allocation_active, "
        "ROUND(release_total_allocated / (1024.0 * 1024.0), 2) as release_allocated, "
        "ROUND(release_total_reserved / (1024.0 * 1024.0), 2) as release_reserve, "
        "ROUND(release_total_active / (1024.0 * 1024.0), 2) as release_active, stream_ptr as stream FROM ";
    sql = isLowCamel ? StringUtil::ToCamelCase(sql) : sql;
    sql += TABLE_OPERATOR_MEMORY + " JOIN STRING_IDS AS NAME ON NAME.id = OP_MEMORY.name"
        " WHERE device_id = ? AND realName LIKE ? ";
    return tempSql + sql;
}

bool DbMemoryDataBase::QueryOperatorDetail(Protocol::MemoryOperatorParams &requestParams,
                                           std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                                           std::vector<Protocol::MemoryOperator> &opDetails)
{
    std::string sql;
    const FileType type = DataBaseManager::Instance().GetFileType();
    const uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
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
    const uint64_t offsetTime = Timeline::TraceTime::Instance().GetOffsetByFileId(requestParams.rankId);
    if (type == FileType::PYTORCH) {
        sql = DbMemoryDataBase::BuildOperatorDetailSql(std::to_string(startTime),
            std::to_string(offsetTime));
    } else {
        ServerLog::Error("Memory tab does not support msprof data.");
        return false;
    }
    AddOperatorSql(requestParams, sql);
    return ExecuteOperatorDetail(requestParams, columnAttr, opDetails, sql);
}

bool DbMemoryDataBase::QueryEntireOperatorTable(Protocol::MemoryOperatorParams &requestParams,
    std::vector<Protocol::MemoryOperator> &opDetails, uint64_t offsetTime)
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
        sql += TABLE_OPERATOR_MEMORY +
            " JOIN STRING_IDS AS NAME ON NAME.id = OP_MEMORY.name WHERE device_id = ? ";
    } else {
        ServerLog::Error("Memory tab does not support msprof data.");
        return false;
    }
    return ExecuteQueryEntireOperatorTable(requestParams, opDetails, sql);
}

bool DbMemoryDataBase::QueryComponentDetail(Protocol::MemoryComponentParams &requestParams,
                                            std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                                            std::vector<Protocol::MemoryComponent> &componentDetails)
{
    std::string sql;
    FileType type = DataBaseManager::Instance().GetFileType();
    if (type == FileType::PYTORCH) {
        uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
        uint64_t offsetTime = Timeline::TraceTime::Instance().GetOffsetByFileId(requestParams.rankId);
        // 最内层SQL根据组件编号分组取出内存占用峰值大于100M的那些组件编号
        // 中间层SQL和最内层SQL的查询结果根据组件编号和内存占用值做一次连接
        // 做分组的原因是可能有多个时刻内存占用为峰值，只需要时刻最小的那个
        // 最外层SQL根据组件编号和组件名做一次连接
        sql = "SELECT t4.name AS componentColumn, ROUND(t3.size / (1024.0 * 1024.0), 2) AS totalReservedColumn,"
            " t3.timestamp_maxsize AS timestampColumn FROM "
            "(SELECT t1.moduleId AS id, t1.totalReserved AS size, MIN(ROUND((t1.timestampNs - " +
            std::to_string(NumberSafe::Add(startTime, offsetTime)) +
            ") / (1000.0 * 1000.0), 3)) AS timestamp_maxsize FROM " + TABLE_NPU_MODULE_MEM + " AS t1 JOIN " +
            "(SELECT moduleId, MAX(totalReserved) AS max_total_reserved FROM " + TABLE_NPU_MODULE_MEM +
            " GROUP BY moduleId HAVING max_total_reserved >= " + std::to_string(componentThresholdByte) +
            ") AS t2 ON t1.moduleId = t2.moduleId AND t1.totalReserved = t2.max_total_reserved "
            " WHERE t1.deviceId = ? "
            "GROUP BY t1.moduleId, t1.totalReserved) AS t3 JOIN ENUM_MODULE AS t4 ON t3.id = t4.id";
        if (!requestParams.order.empty() && !requestParams.orderBy.empty()) {
            sql += " ORDER BY " + requestParams.orderBy + "Column";
            if (requestParams.order == "ascend") {
                sql += " ASC ";
            } else {
                sql += " DESC ";
            }
        }
        sql += " LIMIT ? OFFSET ? ";
    } else {
        ServerLog::Error("Failed to query component detail: Memory tab does not support msprof data.");
        return false;
    }
    return ExecuteComponentDetail(requestParams, columnAttr, componentDetails, sql);
}

bool DbMemoryDataBase::QueryEntireComponentTable(Protocol::MemoryComponentParams &requestParams,
    std::vector<Protocol::MemoryComponent> &componentDetails, uint64_t offsetTime)
{
    std::string sql;
    FileType type = DataBaseManager::Instance().GetFileType();
    if (type == FileType::PYTORCH) {
        uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
        sql = "SELECT t4.name, ROUND(t3.size / (1024.0 * 1024.0), 2), t3.timestamp_maxsize FROM "
              "(SELECT t1.moduleId AS id, t1.totalReserved AS size, MIN(ROUND((t1.timestampNs - " +
              std::to_string(NumberSafe::Add(startTime, offsetTime)) +
              ") / (1000.0 * 1000.0), 3)) AS timestamp_maxsize FROM " + TABLE_NPU_MODULE_MEM + " AS t1 JOIN " +
              "(SELECT moduleId, MAX(totalReserved) AS max_total_reserved FROM " + TABLE_NPU_MODULE_MEM +
              " GROUP BY moduleId HAVING max_total_reserved >= " + std::to_string(componentThresholdByte) +
              ") AS t2 ON t1.moduleId = t2.moduleId AND t1.totalReserved = t2.max_total_reserved "
              "WHERE t1.deviceId = ? "
              "GROUP BY t1.moduleId, t1.totalReserved) AS t3 JOIN ENUM_MODULE AS t4 ON t3.id = t4.id ";
    } else {
        ServerLog::Error("Failed to query entire component table: Memory tab does not support msprof data.");
        return false;
    }
    return ExecuteQueryEntireComponentTable(requestParams, componentDetails, sql);
}

bool DbMemoryDataBase::QueryMemoryView(Protocol::MemoryViewParams &requestParams,
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
            "ROUND(total_active / (1024.0 * 1024.0), 2) as total_active, stream_ptr as stream, device_id FROM ";
        sql = isLowCamel ? StringUtil::ToCamelCase(sql) : sql;
        sql += TABLE_MEMORY_RECORD + " JOIN STRING_IDS AS NAME ON NAME.id = MEMORY_RECORD.component ";
        sql += " UNION ALL select 'APP' as component, ROUND((timestampNs - " + std::to_string(startTime) +
                " ) / (1000.0 * 1000.0), 2) as timestampNs, "
               " 0 as total_allocated,  ROUND((hbm + ddr) / (1024.0 * 1024.0), 2) as total_reserve, "
               " 0 as totalActive, '' as stream, deviceId from NPU_MEM join STRING_IDS as ids on ids.id = type "
               " where value = 'app' ";
        sql += " ) where device_id = ? ";
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
        sql.append(" ROUND(size / 1024.0, 2) as size, size as realSize, OP_MEMORY.device_id"
            " FROM OP_MEMORY JOIN STRING_IDS AS NAME ON "
            "   NAME.id = OP_MEMORY.name"
            ") "
            " WHERE device_id = ? AND name LIKE ? ");
    } else {
        ServerLog::Error("Memory tab does not support msprof data.");
        return false;
    }

    if (requestParams.type == Protocol::MEMORY_STREAM_GROUP) {
        sql.append(isLowCamel ? " AND streamPtr <> ''" : " AND stream_ptr <> ''");
    }
    if (requestParams.startTime != -1 && requestParams.endTime != -1) {
        const std::string ALLOCATION_TIME_KEY = isLowCamel ? "allocationTime" : "allocation_time";
        const std::string RELEASE_TIME_KEY = isLowCamel ? "releaseTime" : "release_time";
        if (requestParams.isOnlyShowAllocatedOrReleasedWithinInterval) {
            /*
             * 只显示在时间区间内分配或释放内存的数据
             * 参数：
             * 1. startTime + offsetTime
             * 2. startTime
             * 3. endTime
             * 4. startTime + offsetTime
             * 5. startTime
             * 6. endTime
             */
            sql.append(" AND ((").append(ALLOCATION_TIME_KEY).append(" IS NOT NULL AND ")
                .append("ROUND((").append(ALLOCATION_TIME_KEY)
                .append(" - ?) / (1000.0 * 1000.0), 3) BETWEEN ? AND ? ");
            sql.append(") OR (").append(RELEASE_TIME_KEY).append(" IS NOT NULL AND ")
                .append("ROUND((").append(RELEASE_TIME_KEY)
                .append(" - ?) / (1000.0 * 1000.0), 3) BETWEEN ? AND ?)) ");
        } else {
            /*
             * 显示全部的数据
             * 参数：
             * 1. startTime + offsetTime
             * 2. startTime
             * 3. startTime + offsetTime
             * 4. endTime
             */
            // 在 db 情况下 allocation_time release_time 不可能为 0，不用再判断
            sql.append(" AND ((").append(RELEASE_TIME_KEY)
                .append(" IS NULL OR ").append("ROUND((")
                .append(RELEASE_TIME_KEY).append(" - ?) / (1000.0 * 1000.0), 3) >= ? )")
                .append(" AND ROUND((").append(ALLOCATION_TIME_KEY)
                .append(" - ?) / (1000.0 * 1000.0), 3) <= ?) ");
        }
    }
    if (requestParams.minSize != std::numeric_limits<int64_t>::min()) {
        sql += " AND realSize >= ? ";
    }
    if (requestParams.maxSize != std::numeric_limits<int64_t>::max()) {
        sql += " AND realSize <= ? ";
    }
    return ExecuteOperatorsTotalNum(requestParams, totalNum, sql);
}

bool DbMemoryDataBase::QueryComponentsTotalNum(Protocol::MemoryComponentParams &requestParams, int64_t &totalNum)
{
    std::string sql;
    FileType type = DataBaseManager::Instance().GetFileType();
    if (type == FileType::PYTORCH) {
        sql = "SELECT count(*) FROM (SELECT t2.name FROM " + TABLE_NPU_MODULE_MEM +
            " AS t1 JOIN ENUM_MODULE AS t2 ON t1.moduleId = t2.id WHERE deviceId = ? "
            " GROUP BY t2.name HAVING MAX(t1.totalReserved) >= " +
            std::to_string(componentThresholdByte) + ") AS t3";
    } else {
        ServerLog::Error("Failed to query components total num: Memory tab does not support msprof data.");
        return false;
    }
    return ExecuteComponentTotalNum(requestParams, totalNum, sql);
}

bool DbMemoryDataBase::QueryOperatorSize(Protocol::MemoryOperatorSizeParams &requestParams, double &min, double &max)
{
    FileType type = DataBaseManager::Instance().GetFileType();
    std::string sql = "";
    if (type == FileType::PYTORCH) {
        sql += "SELECT ROUND(min(size)/ 1024.0, 2) as minSize, "
               " ROUND(max(size)/ 1024.0, 2) as maxSize FROM " + TABLE_OPERATOR_MEMORY + " WHERE device_id = ? ";
    } else {
        ServerLog::Error("Memory tab does not support msprof data.");
        return false;
    }
    return ExecuteOperatorSize(requestParams, min, max, sql);
}

// DB格式不支持静态图内存数据
bool DbMemoryDataBase::QueryStaticOperatorSize(Protocol::StaticOperatorSizeParams &requestParams, double &min,
                                               double &max)
{
    return false;
}

// DB格式不支持静态图内存数据
bool DbMemoryDataBase::QueryStaticOperatorsTotalNum(Protocol::StaticOperatorListParams &requestParams,
                                                    int64_t &totalNum)
{
    return false;
}

// DB格式不支持静态图内存数据
bool DbMemoryDataBase::QueryStaticOperatorList(Protocol::StaticOperatorListParams &requestParams,
                                               std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                                               std::vector<Protocol::StaticOperatorItem> &opDetails)
{
    return false;
}

// DB格式不支持静态图内存数据
bool DbMemoryDataBase::QueryEntireStaticOperatorTable(Protocol::StaticOperatorListParams& requestParams,
                                                      std::vector<Protocol::StaticOperatorItem>& opDetails)
{
    return false;
}

// DB格式不支持静态图内存数据
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
void DbMemoryDataBase::ParseCallBack(const std::string &rankId,
                                     const std::string &fileId,
                                     bool result,
                                     const std::string &msg)
{
    if (rankId.empty()) {
        ranks.clear();
        auto event = std::make_unique<Protocol::ModuleResetEvent>();
        event->moduleName = Protocol::MODULE_MEMORY;
        event->result = true;
        event->reset = true;
        SendEvent(std::move(event));
    } else {
        auto event = std::make_unique<Protocol::ParseMemoryCompletedEvent>();
        event->moduleName = Protocol::MODULE_TIMELINE;
        event->result = result;
        event->isCluster = true;
        event->fileId = fileId;
        std::vector<Protocol::MemorySuccess> memoryResult;
        memoryResult.push_back(ranks[rankId]);
        event->memoryResult = memoryResult;
        SendEvent(std::move(event));
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

std::string DbMemoryDataBase::QueryDeviceId()
{
    FileType type = DataBaseManager::Instance().GetFileType();
    std::string sql = "";
    if (type == FileType::PYTORCH) {
        sql += "SELECT deviceId FROM " + TABLE_OPERATOR_MEMORY + " LIMIT 1 ";
    } else {
        return "";
    }
    return ExecuteQueryDeviceId(sql);
};

}
}
}

