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
using componentDtoVector = std::vector<Protocol::ComponentDto>;
class MemoryDataBase : public Database {
public:
    explicit MemoryDataBase(std::mutex &sqlMutex);
    ~MemoryDataBase() override;

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
        std::vector<Protocol::MemoryTableColumnAttr> &columnAttr, std::vector<Protocol::MemoryOperator> &opDetails);
    bool QueryMemoryView(Protocol::MemoryComponentParams &requestParams, Protocol::MemoryViewData &operatorBody);

    void SaveRecordDetail();
    void SaveOperatorDetail();

    bool QueryOperatorsTotalNum(Protocol::MemoryOperatorParams &requestParams, int64_t &totalNum);
    bool QueryOperatorSize(double &min, double &max);
    uint64_t QueryMinOperatorAllocationTime();
    uint64_t QueryMinRecordTimestamp();

    void SetInferenceType(bool inference);
    bool IsInferenceType() const;

private:
    std::mutex &mutex;
    const std::string operatorTable = "operator";
    const std::string recordTable = "record";
    const int exLength = 4;

    const std::vector<std::string> baseLegends = {
        "Time (ms)", "Operators Allocated", "Operators Activated", "Operators Reserved"
    };

    const std::string appLegend = "App Reserved";
    const std::vector<Protocol::MemoryTableColumnAttr> tableColumnAttr = {
        {"Name", "string", "name"},
        {"Size(KB)", "number", "size"},
        {"Allocation Time(ms)", "number", "allocationTime"},
        {"Release Time(ms)", "number", "releaseTime"},
        {"Duration(ms)", "number", "duration"},
        {"Active Release Time(ms)", "number", "activeReleaseTime"},
        {"Active Duration(ms)", "number", "activeDuration"},
        {"Allocation Total Allocated(MB)", "number", "allocationAllocated"},
        {"Allocation Total Reserved(MB)", "number", "allocationReserved"},
        {"Allocation Total Active(MB)", "number", "allocationActive"},
        {"Release Total Allocated(MB)", "number", "releaseAllocated"},
        {"Release Total Reserved(MB)", "number", "releaseReserved"},
        {"Release Total Active(MB)", "number", "releaseActive"},
        {"Stream", "string", "streamId"}
    };

    const std::vector<std::string> activeRelatedColumn = {
        "Active Release Time(ms)",
        "Active Duration(ms)",
        "Allocation Total Active(MB)",
        "Release Total Active(MB)",
        "Stream"
    };

    bool hasInitStmt = false;
    const int cacheSize = 100;
    std::vector<Operator> operatorCache;
    std::vector<Record> recordCache;

    sqlite3_stmt *insertOperatorStmt = nullptr;
    sqlite3_stmt *insertRecordStmt = nullptr;

    sqlite3_stmt *GetOperatorStmt(uint64_t paramLen);
    sqlite3_stmt *GetRecordStmt(uint64_t paramLen);

    std::string GetOperatorSql(Protocol::MemoryOperatorParams &requestParams);

    std::string GetPeakMemory(const Protocol::MemoryPeak &peak, const std::vector<std::string>& streams);
    void GetLines(const componentDtoVector componentDtoVec, std::vector<std::vector<std::string>> &lines,
        std::vector<std::string> &legends, Protocol::MemoryPeak &peak, const std::vector<std::string>& streams);
    void GetStreamLines(const componentDtoVector componentDtoVec, std::vector<std::vector<std::string>> &lines,
        std::vector<std::string> &legends, Protocol::MemoryPeak &peak, const std::vector<std::string>& streams);
    const std::string COMPONENT_APP = "APP";
    const std::string COMPONENT_GE = "GE";
    const std::string COMPONENT_PTA = "PTA";
    const std::string COMPONENT_PTA_AND_GE = "PTA+GE";
    bool isInference = false;

    std::vector<std::string> GetStreamLists();
};

}; // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_MEMORY_DATABASE_H
