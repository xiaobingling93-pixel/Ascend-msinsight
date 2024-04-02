/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "TraceDatabaseHelper.h"


namespace Dic::Module::Timeline {

void TraceDatabaseHelper::QueryTaskInfoById(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                            const Protocol::ThreadDetailParams &requestParams,
                                            Protocol::UnitThreadDetailBody &responseBody,
                                            std::map<std::string, std::string> &stringCache)
{
    auto processType = GetProcessType(requestParams.metaType);
    auto resultSet = QueryTaskCacheInfoById(stmt, requestParams);
    std::vector<std::string> types = {"inputShapes", "inputDataTypes", "inputFormats",
                                      "outputShapes", "outputDataTypes", "outputFormats"};
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    if (resultSet.operator bool() && resultSet->Next()) {
        if (processType == PROCESS_TYPE::ASCEND_HARDWARE) {
            responseBody.data.inputShapes = stringCache[resultSet->GetString("inputShapes")];
            responseBody.data.inputDataTypes = stringCache[resultSet->GetString("inputDataTypes")];
            responseBody.data.inputFormats = stringCache[resultSet->GetString("inputFormats")];
            responseBody.data.outputShapes = stringCache[resultSet->GetString("outputShapes")];
            responseBody.data.outputDataTypes = stringCache[resultSet->GetString("outputDataTypes")];
            responseBody.data.outputFormats = stringCache[resultSet->GetString("outputFormats")];
        }
        for (auto &item: resultSet->GetColumns()) {
            if (std::find(types.begin(), types.end(), item.first) != types.end()) {
                continue;
            }
            JsonUtil::AddConstMember(json, item.first, stringCache[resultSet->GetString(item.second)], allocator);
        }
    }

    resultSet = QueryTaskStrInfoById(stmt, requestParams);
    if (resultSet.operator bool() && resultSet->Next()) {
        for (auto &item: resultSet->GetColumns()) {
            JsonUtil::AddConstMember(json, item.first, resultSet->GetString(item.second), allocator);
        }
    }
    responseBody.data.args = JsonUtil::JsonDump(json);
}

std::unique_ptr<SqliteResultSet> TraceDatabaseHelper::QueryTaskStrInfoById(
    std::unique_ptr<SqlitePreparedStatement> &stmt, const Protocol::ThreadDetailParams &requestParams)
{
    auto processType = GetProcessType(requestParams.metaType);
    std::string sql;
    std::unique_ptr<SqliteResultSet> resultSet;
    switch (processType) {
        case PROCESS_TYPE::ASCEND_HARDWARE:
            sql = "SELECT block_dim, mixBlockDim "
                  " FROM TASK main join COMPUTE_TASK_INFO CTI on main.globalTaskId = CTI.globalTaskId"
                  "  where main.ROWID = ?";
            return ExecuteQuery(stmt, sql, requestParams.id);
        case PROCESS_TYPE::HCCL:
            sql = "SELECT com.planeId, com.notifyId ,com.srcRank ,com.dstRank, com.size, com.opId "
                  "      FROM TASK main join COMMUNICATION_TASK_INFO com on main.globalTaskId = com.globalTaskId "
                  " where main.ROWID = ?";
            return ExecuteQuery(stmt, sql, requestParams.id);
        case PROCESS_TYPE::API:
            sql = " select group_concat(stack, ';\n') as 'Call stack', sequenceNumber from ( "
                  "    SELECT stack, sequenceNumber FROM PYTORCH_API main "
                  "             left join PYTORCH_CALLCHAINS call on call.id = main.callchainId "
                  "    where main.ROWID = ? order by stackDepth )";
            return ExecuteQuery(stmt, sql, requestParams.id);
        case PROCESS_TYPE::CANN_API:
            sql = "SELECT t.name as apiType, connectionId FROM CANN_API main join ENUM_API_TYPE t on t.id = type "
                  " where main.ROWID = ?";
            return ExecuteQuery(stmt, sql, requestParams.id);
        default:
            return resultSet;
    }
}

std::unique_ptr<SqliteResultSet> TraceDatabaseHelper::QueryTaskCacheInfoById(
    std::unique_ptr<SqlitePreparedStatement> &stmt, const Protocol::ThreadDetailParams &requestParams)
{
    auto processType = GetProcessType(requestParams.metaType);
    std::string sql;
    std::unique_ptr<SqliteResultSet> resultSet;
    switch (processType) {
        case PROCESS_TYPE::ASCEND_HARDWARE:
            sql = "SELECT inputShapes, inputDataTypes, inputFormats, outputShapes, outputDataTypes,"
                  " outputFormats, CTI.taskType, opType FROM TASK main join COMPUTE_TASK_INFO CTI"
                  " on main.globalTaskId = CTI.globalTaskId  where main.ROWID = ?";
            return ExecuteQuery(stmt, sql, requestParams.id);
        case PROCESS_TYPE::HCCL:
            sql = "SELECT com.taskType, com.groupName ,com.rdmaType,com.transportType, com.dataType, com.linkType "
                  "      FROM TASK main join COMMUNICATION_TASK_INFO com on main.globalTaskId = com.globalTaskId "
                  " where main.ROWID = ?";
            return ExecuteQuery(stmt, sql, requestParams.id);
        case PROCESS_TYPE::API:
            sql = "SELECT inputDtypes, inputShapes FROM PYTORCH_API main "
                  " where main.ROWID = ?";
            return ExecuteQuery(stmt, sql, requestParams.id);
        case PROCESS_TYPE::CANN_API:
        default:
            return resultSet;
    }
}

}