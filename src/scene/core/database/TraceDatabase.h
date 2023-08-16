/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_TRACEDATABASE_H
#define PROFILER_SERVER_TRACEDATABASE_H

#include "Database.h"
#include "GlobalDefs.h"

namespace Dic {
namespace Scene {
namespace Core {
class TraceDatabase : public Database{
public:
    TraceDatabase() = default;
    ~TraceDatabase() override;

    bool SetConfig();
    bool CreateTable();
    bool CreateIndex();
    bool InsertSlice(const json_t &json);
    bool UpdateProcessName(const json_t &json);
    bool UpdateProcessLabel(const json_t &json);
    bool UpdateProcessSortIndex(const json_t &json);
    bool UpdateThreadName(const json_t &json);
    bool UpdateThreadSortIndex(const json_t &json);
    bool InsertFlow(const json_t &json);
    bool InitStmt();
    void ReleaseStmt();

private:
    const std::string sliceTable = "slice";
    const std::string threadTable = "thread";
    const std::string processTable = "process";
    const std::string flowTable = "flow";
    const std::string idIndex = "id_index";
    const std::string trackIdTimeIndex = "track_id_time_index";

    bool initStmt = false;
    sqlite3_stmt *insertSliceStmt = nullptr;
    sqlite3_stmt *updateProcessNameStmt = nullptr;
    sqlite3_stmt *updateProcessLabelStmt = nullptr;
    sqlite3_stmt *updateProcessSortIndexStmt = nullptr;
    sqlite3_stmt *updateThreadNameStmt = nullptr;
    sqlite3_stmt *updateThreadSortIndexStmt = nullptr;
    sqlite3_stmt *insertFlowStmt = nullptr;

};
} // end of namespace Core
} // end of namespace Scene
} // end of namespace Dic




#endif //PROFILER_SERVER_TRACEDATABASE_H
