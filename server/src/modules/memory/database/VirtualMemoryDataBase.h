/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_VIRTUALMEMORYDATABASE_H
#define PROFILER_SERVER_VIRTUALMEMORYDATABASE_H

#include "Database.h"
#include "MemoryProtocolUtil.h"
#include "MemoryProtocolRequest.h"
#include "MemoryProtocolRespose.h"

namespace Dic {
namespace Module {
namespace Memory {
using componentDtoVector = std::vector<Protocol::ComponentDto>;
class VirtualMemoryDataBase : public Database {
public:
    explicit VirtualMemoryDataBase(std::mutex &sqlMutex) : mutex(sqlMutex) {};;
    ~VirtualMemoryDataBase() override = default;
    virtual bool QueryOperatorDetail(Protocol::MemoryOperatorParams &requestParams,
                                     std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                                     std::vector<Protocol::MemoryOperator> &opDetails) = 0;
    virtual bool
    QueryMemoryView(Protocol::MemoryComponentParams &requestParams, Protocol::MemoryViewData &operatorBody) = 0;

    virtual bool QueryOperatorsTotalNum(Protocol::MemoryOperatorParams &requestParams, int64_t &totalNum) = 0;
    virtual bool QueryOperatorSize(double &min, double &max, std::string rankId) = 0;
protected:
    std::mutex &mutex;
    const std::string operatorTable = "operator";
    const std::string recordTable = "record";
    const int exLength = 4;
    bool isInference = false;

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
    const std::string COMPONENT_APP = "APP";
    const std::string COMPONENT_GE = "GE";
    const std::string COMPONENT_PTA = "PTA";
    const std::string COMPONENT_PTA_AND_GE = "PTA+GE";

    std::vector<std::string> GetStreamLists(std::string rankId);

    bool ExecuteOperatorSize(double &min, double &max, std::string sql);
    bool ExecuteOperatorsTotalNum(Protocol::MemoryOperatorParams &requestParams, int64_t &totalNum, std::string sql);
    bool ExecuteQueryMemoryView(Protocol::MemoryComponentParams &requestParams, Protocol::MemoryViewData &operatorBody,
                                std::string sql);
    bool ExecuteOperatorDetail(Protocol::MemoryOperatorParams &requestParams,
        std::vector<Protocol::MemoryTableColumnAttr> &columnAttr, std::vector<Protocol::MemoryOperator> &opDetails,
        std::string sql);
    void AddOperatorSql(Protocol::MemoryOperatorParams requestParams, std::string &sql);

private:
    void GetLines(const componentDtoVector componentDtoVec, std::vector<std::vector<std::string>> &lines,
                  std::vector<std::string> &legends, Protocol::MemoryPeak &peak,
                  const std::vector<std::string> &streams);
    std::string GetPeakMemory(const Protocol::MemoryPeak &peak, const std::vector<std::string> &streams);
    void GetStreamLines(const componentDtoVector componentDtoVec, std::vector<std::vector<std::string>> &lines,
                        std::vector<std::string> &legends, Protocol::MemoryPeak &peak,
                        const std::vector<std::string> &streams);
};

}; // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_MEMORY_DATABASE_H
