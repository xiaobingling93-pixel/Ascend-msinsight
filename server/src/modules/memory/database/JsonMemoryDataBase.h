/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_MEMORY_DATABASE_H
#define PROFILER_SERVER_MEMORY_DATABASE_H
#include "Protocol.h"
#include "VirtualMemoryDataBase.h"
#include "MemoryProtocolRespose.h"
#include "MemoryProtocolRequest.h"
#include "MemoryDef.h"

namespace Dic {
namespace Module {
namespace Memory {
using memoryLines = std::vector<std::vector<std::string>>;
using componentDtoVector = std::vector<Protocol::ComponentDto>;
class JsonMemoryDataBase : public VirtualMemoryDataBase {
public:
    explicit JsonMemoryDataBase(std::mutex &sqlMutex);
    ~JsonMemoryDataBase() override;

    bool SetConfig();
    bool CreateTable();
    bool DropTable();
    bool InitStmt();
    void ReleaseStmt();

    void InsertOperatorDetailList(const std::vector<Operator> &eventList);
    void insertOperatorDetail(const Operator &event);
    void InsertRecordDetailList(const std::vector<Record> &eventList);
    void insertRecordDetail(const Record &event);

    bool QueryOperatorDetail(Protocol::MemoryOperatorParams &requestParams,
                             std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                             std::vector<Protocol::MemoryOperator> &opDetails) override;
    bool QueryMemoryView(Protocol::MemoryComponentParams &requestParams,
                         Protocol::MemoryViewData &operatorBody) override;

    void SaveRecordDetail();
    void SaveOperatorDetail();

    bool QueryOperatorsTotalNum(Protocol::MemoryOperatorParams &requestParams, int64_t &totalNum) override;
    bool QueryOperatorSize(double &min, double &max, std::string rankId) override;
    uint64_t QueryMinOperatorAllocationTime();
    uint64_t QueryMinRecordTimestamp();

    void SetInferenceType(bool inference);
    bool IsInferenceType() const;

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

    const std::string COMPONENT_APP = "APP";
    const std::string COMPONENT_GE = "GE";
    const std::string COMPONENT_PTA = "PTA";
    const std::string COMPONENT_PTA_AND_GE = "PTA+GE";
};

} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_MEMORY_DATABASE_H
