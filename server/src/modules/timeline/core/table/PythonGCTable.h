/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
 
#ifndef PROFILER_SERVER_PYTHON_GC_TABLE_H
#define PROFILER_SERVER_PYTHON_GC_TABLE_H
#include "TextTableColum.h"
#include "Table.h"
namespace Dic::Module::Timeline {
struct PythonGCPO {
    uint64_t id = 0;
    uint64_t timestamp = 0;
    uint64_t endTime = 0;
};
class PythonGCTable : public Table<PythonGCPO> {
public:
    PythonGCTable() = default;
    ~PythonGCTable() override = default;
 
protected:
    const std::unordered_map<std::string_view, assign> &GetAssignMap() override
    {
        static std::unordered_map<std::string_view, assign> assignMap = {
            { PythonGCColumn::ID, SetId },
            { PythonGCColumn::TIMESTAMP, SetTimestamp },
            { PythonGCColumn::ENDTIME, SetEndTime },
        };
        return assignMap;
    }
 
    const std::string &GetTableName() override
    {
        static std::string tableName = "GC_RECORD";
        return tableName;
    }
    static void SetId(PythonGCPO &pythonGCPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetTimestamp(PythonGCPO &pythonGCPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetEndTime(PythonGCPO &pythonGCPO, const std::unique_ptr<SqliteResultSet> &resultSet);
};
}
#endif // PROFILER_SERVER_PYTHON_GC_TABLE_H