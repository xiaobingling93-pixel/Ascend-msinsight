/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */
#ifndef PROFILER_SERVER_MEMSNAPSHOTDATABASE_H
#define PROFILER_SERVER_MEMSNAPSHOTDATABASE_H

#include "pch.h"
#include "Database.h"
#include "MemSnapshotDefs.h"
#include "MemSnapshotTableColumn.h"

namespace Dic::Module::FullDb {
using namespace Dic::Protocol;
using namespace Dic::Module::MemSnapshot;

class MemSnapshotDatabase : public Database {
public:
    explicit MemSnapshotDatabase(std::recursive_mutex& sqlMutex) : Database(sqlMutex) {};
    bool CheckAllTableExist();
    bool OpenDbReadOnly(const std::string& dbPath);
    std::string GetRealValueInTableDictionaryMap(const std::string& tableName, const std::string& colName, int intVal);


    // API
    // 查询所有的block，用于内存块生命周期图展示
    bool QueryAllBlocks(std::vector<Block> &blocks);
    // 基于id查询单个block的详细信息
    std::optional<Block> QueryBlockById(int64_t blockId);

    // 查询最大事件id
    [[nodiscard]] int64_t QueryMaxEntryId() const;
    static void Reset();
private:
    static inline const std::string LOG_TAG = "[MemSnapshotDb] ";
    const std::string blockTable = "block";
    const std::string traceEntryTable = "trace_entry";
    const std::string dictionaryTable = "dictionary";
    int64_t maxEntryId = 0;
    std::map<std::string, std::map<int, std::string>> tableDictionaryMap;
    bool InitTableDictionaryMap();
    bool InitContext();
    Block QueryBlockByStep(sqlite3_stmt* stmt);
    static std::string GetTableColumnTag(const std::string& tableName, const std::string& colName);
};
}


#endif //PROFILER_SERVER_MEMSNAPSHOTDATABASE_H
