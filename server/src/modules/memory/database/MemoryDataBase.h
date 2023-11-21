/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_MEMORY_DATABASE_H
#define PROFILER_SERVER_MEMORY_DATABASE_H
#include "Protocol.h"
#include "Database.h"
#include "MemoryProtocolRespose.h"
#include "MemoryProtocolRequest.h"
#include "MemoryDef.h"

namespace Dic {
namespace Module {
namespace Memory {
using memoryLines = std::vector<std::vector<std::string>>;
using componentDtoVector = std::vector<Protocol::ComponentDto>;
class MemoryDataBase : public Database {
public:
    MemoryDataBase() = default;
    ~MemoryDataBase() override;

    bool SetConfig();
    bool CreateTable();
    bool InitStmt();
    void ReleaseStmt();

    void InsertOperatorDetailList(const std::vector<Operator> &eventList);
    void insertOperatorDetail(const Operator &event);
    void InsertRecordDetailList(const std::vector<Record> &eventList);
    void insertRecordDetail(const Record &event);

    bool QueryOperatorDetail(Protocol::MemoryOperatorParams &requestParams,
                             std::vector<Protocol::MemoryOperator> &responseBody);
    bool QueryMemoryView(Protocol::MemoryComponentParams &requestParams, Protocol::OperatorMemory &operatorBody);

    void SaveRecordDetail();
    void SaveOperatorDetail();

    bool QueryOperatorsTotalNum(Protocol::MemoryOperatorParams &requestParams, int64_t &totalNum);
    bool QueryOperatorSize(double &min, double &max);

private:
    const std::string operatorTable = "operator";
    const std::string recordTable = "record";
    const int exLength = 4;

    bool hasInitStmt = false;
    const int cacheSize = 100;
    std::vector<Operator> operatorCache;
    std::vector<Record> recordCache;

    sqlite3_stmt *insertOperatorStmt = nullptr;
    sqlite3_stmt *insertRecordStmt = nullptr;

    sqlite3_stmt *GetOperatorStmt(uint64_t paramLen);
    sqlite3_stmt *GetRecordStmt(uint64_t paramLen);

    std::string GetOperatorSql(Protocol::MemoryOperatorParams &requestParams);

    std::string GetPeakMemory(const Protocol::MemoryPeak &peak);
    void GetLines(const componentDtoVector componentDtoVec,
                  memoryLines &lines, Protocol::MemoryPeak &peak);
};

} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_MEMORY_DATABASE_H
