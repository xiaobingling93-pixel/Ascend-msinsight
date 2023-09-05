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
class MemoryDataBase : public Database {
public:
    MemoryDataBase() = default;
    ~MemoryDataBase() override;

    bool SetConfig();
    bool CreateTable();
    bool InitStmt();
    void ReleaseStmt();

    bool InsertOperatorDetailList(const std::vector<Operator> &eventList);
    bool insertOperatorDetail(const Operator &event);
    bool InsertRecordDetailList(const std::vector<Record> &eventList);
    bool insertRecordDetail(const Record &event);

    bool QueryOperatorDetail(Protocol::MemoryOperatorParams &requestParams,
                             std::vector<Protocol::MemoryOperator> &responseBody);
    bool QueryMemoryView(Protocol::MemoryComponentParams &requestParams, Protocol::OperatorMemory &operatorBody);

    bool SaveRecordDetail();
    bool SaveOperatorDetail();
private:
    const std::string operatorTable = "operator";
    const std::string recordTable = "record";

    bool initStmt = false;
    const int cacheSize = 100;
    std::vector<Operator> operatorCache;
    std::vector<Record> recordCache;

    sqlite3_stmt *insertOperatorStmt = nullptr;
    sqlite3_stmt *insertRecordStmt = nullptr;

    sqlite3_stmt *GetOperatorStmt(uint64_t paramLen);
    sqlite3_stmt *GetRecordStmt(uint64_t paramLen);

    void GetOperatorLine(Protocol::ComponentDto item, Protocol::OperatorMemory &operatorMap);
    void GetAppLine(Protocol::ComponentDto item, Protocol::OperatorMemory &operatorMap);
    void GetComponentMap(Protocol::ComponentDto item, Protocol::ComponentMemory &componentMap);
};

} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_MEMORY_DATABASE_H
