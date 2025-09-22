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
    explicit VirtualMemoryDataBase(std::recursive_mutex &sqlMutex) : Database(sqlMutex) {};;
    ~VirtualMemoryDataBase() override = default;
    virtual bool QueryMemoryType(std::string &type, std::vector<std::string> &graphId) = 0;
    virtual bool QueryMemoryResourceType(std::string &type) = 0;
    virtual bool QueryOperatorDetail(Protocol::MemoryOperatorParams &requestParams,
                                     std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                                     std::vector<Protocol::MemoryOperator> &opDetails) = 0;
    virtual bool QueryComponentDetail(Protocol::MemoryComponentParams &requestParams,
                                      std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                                      std::vector<Protocol::MemoryComponent> &componentDetails) = 0;
    virtual bool
    QueryMemoryView(Protocol::MemoryViewParams &requestParams, Protocol::MemoryViewData &operatorBody,
        uint64_t offsetTime) = 0;
    virtual bool QueryStaticOperatorList(Protocol::StaticOperatorListParams &requestParams,
                                         std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                                         std::vector<Protocol::StaticOperatorItem> &opDetails) = 0;

    virtual bool QueryStaticOperatorGraph(Protocol::StaticOperatorGraphParams &requestParams,
                                         Protocol::StaticOperatorGraphItem &graphItem) = 0;

    virtual bool QueryOperatorsTotalNum(Protocol::MemoryOperatorParams &requestParams, int64_t &totalNum) = 0;
    virtual bool QueryComponentsTotalNum(Protocol::MemoryComponentParams &requestParams, int64_t &totalNum) = 0;
    virtual bool QueryStaticOperatorsTotalNum(Protocol::StaticOperatorListParams &requestParams, int64_t &totalNum) = 0;

    virtual bool QueryOperatorSize(Protocol::MemoryOperatorSizeParams &requestParams, double &min, double &max) = 0;
    virtual bool QueryStaticOperatorSize(Protocol::StaticOperatorSizeParams &requestParams,
                                         double &min, double &max) = 0;
    virtual bool QueryEntireOperatorTable(Protocol::MemoryOperatorParams &requestParams,
        std::vector<Protocol::MemoryOperator> &opDetails, uint64_t offsetTime) = 0;
    virtual bool QueryEntireComponentTable(Protocol::MemoryComponentParams &requestParams,
        std::vector<Protocol::MemoryComponent> &componentDetails, uint64_t offsetTime) = 0;
    virtual bool QueryEntireStaticOperatorTable(Protocol::StaticOperatorListParams& requestParams,
                                                std::vector<Protocol::StaticOperatorItem>& opDetails) = 0;
protected:
    const std::string operatorTable = "operator";
    const std::string recordTable = "record";
    const int defaultPageSize = 10;
    const int64_t maxUnsignedInt = 4294967295;
    const int64_t maxPageSize = 1000;
    const int64_t maxCurrentPage = 10000000000;
    const double kbSizeDouble = 1024.0;
    const double staticDefaultTotalSize = -1.0; // 静态表TOTAL字段默认赋值异常值
    // 组件占用内存大于100M的才展示
    const double componentThresholdMb = 100.0;
    const double componentThresholdByte = 100.0 * 1024.0 * 1024.0;
    bool isInference = false;

    const std::vector<std::string> baseLegends = {
        "Time (ms)", "Operators Allocated", "Operators Activated", "Operators Reserved"
    };
    const std::vector<std::string> workspaceLegends = {
        "Workspace Allocated", "Workspace Reserved"
    };
    const std::vector<std::string> componentTimeLegends = {
        "Time (ms)"
    };
    const std::vector<std::string> componentPtaLegends = {
        "PTA Allocated", "PTA Activated", "PTA Reserved"
    };
    const std::vector<std::string> componentGeLegends = {
        "GE Allocated", "GE Activated", "GE Reserved"
    };

    const std::vector<std::string> staticGraphLegends = {
        "Node Index", "Size", "Total Size"
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

    const std::vector<Protocol::MemoryTableColumnAttr> staticOpTableColumnAttr = {
        {"Device ID", "string", "deviceId"},
        {"Name", "string", "opName"},
        {"Node Index Start", "number", "nodeIndexStart"},
        {"Node Index End", "number", "nodeIndexEnd"},
        {"Size(MB)", "number", "size"}
    };

    const std::vector<std::string> activeRelatedColumn = {
        "Active Release Time(ms)",
        "Active Duration(ms)",
        "Allocation Total Active(MB)",
        "Release Total Active(MB)",
        "Stream"
    };

    const std::vector<Protocol::MemoryTableColumnAttr> componentTableColumnAttr = {
        {"Component", "string", "component"},
        {"Peak Memory Reserved(MB)", "number", "totalReserved"},
        {"Timestamp(ms)", "number", "timestamp"}
    };
    const std::string COMPONENT_APP = "APP";
    const std::string COMPONENT_GE = "GE";
    const std::string MIND_SPORE = "MindSpore";
    const std::string COMPONENT_PTA = "PTA";
    const std::string COMPONENT_PTA_AND_GE = "PTA+GE";
    const std::string COMPONENT_WORKSPACE = "WORKSPACE";
    const std::string MIND_SPORE_GE = "MindSpore+GE";

    std::vector<std::string> GetStreamLists(std::string deviceId, std::string deviceIdColumnName);
    bool ExecuteMemoryType(std::vector<std::string> &graphId, std::string &type);
    bool ExecuteMemoryResourceType(std::string &type, std::string sql);
    bool ExecuteOperatorSize(Protocol::MemoryOperatorSizeParams &requestParams, double &min,
        double &max, std::string sql);
    bool ExecuteStaticOperatorSize(Protocol::StaticOperatorSizeParams &requestParams,
                                   double &min, double &max, const std::string &sql);
    bool ExecuteOperatorsTotalNum(Protocol::MemoryOperatorParams &requestParams, int64_t &totalNum, std::string sql);
    bool ExecuteComponentTotalNum(Protocol::MemoryComponentParams &requestParams, int64_t &totalNum, std::string &sql);
    bool ExecuteStaticOperatorListTotalNum(Protocol::StaticOperatorListParams &requestParams,
                                           int64_t &totalNum, std::string sql);
    bool ExecuteQueryMemoryViewExecuteSql(Protocol::MemoryViewParams &requestParams,
                                   std::vector<Protocol::ComponentDto> &componentDtoVec,
                                   std::vector<std::string> &streams,
                                   std::string &sql, std::string deviceIdColumnName);
    bool ExecuteQueryMemoryViewGetGraph(Protocol::MemoryViewParams &requestParams,
                                         std::vector<Protocol::ComponentDto> &componentDtoVec,
                                         std::vector<std::string> &streams,
                                         Protocol::MemoryViewData &operatorBody);
    bool ExecuteOperatorDetail(Protocol::MemoryOperatorParams &requestParams,
        std::vector<Protocol::MemoryTableColumnAttr> &columnAttr, std::vector<Protocol::MemoryOperator> &opDetails,
        std::string sql, std::string deviceIdColumnName);
    bool ExecuteQueryEntireOperatorTable(Protocol::MemoryOperatorParams &requestParams,
        std::vector<Protocol::MemoryOperator> &opDetails, const std::string &sql);
    bool ExecuteComponentDetail(Protocol::MemoryComponentParams &requestParams,
                                std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                                std::vector<Protocol::MemoryComponent> &componentDetails, std::string &sql);
    bool ExecuteQueryEntireComponentTable(Protocol::MemoryComponentParams &requestParams,
        std::vector<Protocol::MemoryComponent> &componentDetails, std::string &sql);
    bool ExecuteStaticOperatorGraph(Protocol::StaticOperatorGraphParams &requestParams,
                                    Protocol::StaticOperatorGraphItem &graphItem, const std::string& totalSql,
                                    const std::string& graphStartSql, const std::string& graphEndSql);
    bool ExecuteStaticGraphTotalSize(Protocol::StaticOperatorGraphParams &requestParams,
                                      const std::string& graphStartSql, double &maxIndex);
    bool ExecuteStaticGraphStartIndex(Protocol::StaticOperatorGraphParams &requestParams,
                                      const std::string& graphStartSql, std::map<int64_t, double> &graphSizeMap,
                                      int64_t &maxIndex);
    bool ExecuteStaticGraphEndIndex(Protocol::StaticOperatorGraphParams &requestParams,
                                    const std::string& graphEndSql, std::map<int64_t, double> &graphSizeMap,
                                    int64_t &maxIndex);

    bool ExecuteStaticOperatorDetail(Protocol::StaticOperatorListParams &requestParams,
        std::vector<Protocol::MemoryTableColumnAttr> &columnAttr, std::vector<Protocol::StaticOperatorItem> &opDetails,
        const std::string& sql);
    bool ExecuteQueryEntireStaticOperatorTable(Protocol::StaticOperatorListParams &requestParams,
        std::vector<Protocol::StaticOperatorItem> &opDetails, const std::string& sql);
    void AddOperatorSql(Protocol::MemoryOperatorParams requestParams, std::string &sql);
    void AddStableOperatorSql(Protocol::StaticOperatorListParams requestParams, std::string &sql);

private:
    void BuildOverallLinesComponentPoints(const Protocol::ComponentDto &item,
                                            const std::vector<std::string> &streams,
                                            Protocol::MemoryPeak &peak,
                                            std::vector<double> &lines);
    void BuildOverallLinesFrameworkPoints(const Protocol::ComponentDto &item,
                                            const std::vector<std::string> &streams,
                                            Protocol::MemoryPeak &peak,
                                            std::vector<double> &lines);
    void BuildOverallLinesWorkspacePoints(const Protocol::ComponentDto &item,
                                            const std::vector<std::string> &streams,
                                            Protocol::MemoryPeak &peak,
                                            std::vector<double> &lines);
    void GetOverallLines(const componentDtoVector &componentDtoVec, std::vector<double> &lines,
                  std::vector<std::string> &legends, Protocol::MemoryPeak &peak,
                  const std::vector<std::string> &streams);
    void GetOverallLinesLegends(const componentDtoVector &componentDtoVec,
        std::vector<std::string> &legends, Protocol::MemoryPeak &peak,
        const std::vector<std::string> &streams);
    std::string GetPeakMemory(const Protocol::MemoryPeak &peak, const std::vector<std::string> &streams);
    void GetComponentLines(const componentDtoVector &componentDtoVec, std::vector<double> &lines,
        std::vector<std::string> &legends, Protocol::MemoryPeak &peak, const std::vector<std::string> &streams);
    void GetComponentLinesLegends(const componentDtoVector &componentDtoVec,
        std::vector<std::string> &legends, Protocol::MemoryPeak &peak);
    void InsertSize(std::vector<double> &points, const Protocol::ComponentDto &item);
    void InsertStringNull(std::vector<double> &points, const int times);
    void GetStreamLines(const componentDtoVector &componentDtoVec, std::vector<double> &lines,
                        std::vector<std::string> &legends, Protocol::MemoryPeak &peak,
                        const std::vector<std::string> &streams);
    std::vector<Protocol::MemoryOperator> QueryOperatorDetail(sqlite3_stmt *stmt);

    std::string GetCurveSql(const Protocol::MemoryViewParams &requestParams, std::string &sql) const;
};

}; // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_MEMORY_DATABASE_H
