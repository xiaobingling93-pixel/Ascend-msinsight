/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_TRACEDATABASE_H
#define PROFILER_SERVER_TRACEDATABASE_H

#include "ProtocolRequest.h"
#include "ProtocolResponse.h"
#include "ProtocolEntity.h"
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
    bool InitStmt();
    void ReleaseStmt();
    bool InsertSlice(const json_t &json);
    bool UpdateProcessName(const json_t &json);
    bool UpdateProcessLabel(const json_t &json);
    bool UpdateProcessSortIndex(const json_t &json);
    bool UpdateThreadName(const json_t &json);
    bool UpdateThreadSortIndex(const json_t &json);
    bool InsertFlow(const json_t &json);
    void UpdateDepth();

    // search
    std::vector<int64_t> GetTrackIdList();

    // query thread traces data
    bool QueryThreadTraces(Protocol::UnitThreadTracesParams &requestParams, Protocol::UnitThreadTracesBody &responseBody,
        int64_t minTimestamp, int64_t traceId);
    // query threads data
    bool QueryThreads(Protocol::UnitThreadsParams &requestParams, Protocol::UnitThreadsBody &responseBody,
        int64_t minTimestamp, int64_t traceId);
    // query thread Detail
    bool QueryThreadDetail(Protocol::ThreadDetailParams &requestParams, Protocol::UnitThreadDetailBody &responseBody,
       int64_t minTimestamp, int64_t trackId);
    // query flow detail
    bool QueryFlowDetail(Protocol::UnitFlowParams &requestParams, Protocol::UnitFlowBody &responseBody, int64_t minTimestamp);
    bool QueryUnitsMetadata(const std::string &fileId, std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData);
    bool QueryExtremumTimestamp(uint64_t &min, uint64_t &max);
    // query flow name
    bool QueryFlowName(const Protocol::UnitFlowNameParams &requestParams, Protocol::UnitFlowNameBody &responseBody, int64_t minTimestamp, int64_t trackId);

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

    struct SliceTimeData {
        int64_t id;
        int64_t time;
        int64_t dur;
    };

    void UpdateOneTrackDepth(int64_t trackId);
    bool SearchSliceTimeData(int64_t trackId, std::vector<SliceTimeData> &sliceTimeList);
    // depth, idList
    void CalcDepth(const std::vector<SliceTimeData> &sliceData, std::map<int, std::vector<int64_t>> &depthMap);
    void UpdateDepthByID(const std::vector<int64_t> &idList, int depth);

    bool QueryExtremumTimeOfFirstDepth(int64_t trackId, int64_t startTime, int64_t endTime,  Protocol::ExtremumTimestamp &extremumTimestamp);
    void CalculateSelfTime(std::vector<Protocol::SimpleSlice> &simpleSliceVec, std::map<std::string, int64_t> &selfTimeKeyValue, int64_t startTime, int64_t endTime);
    void AddData(std::map<std::string, int64_t> &selfTimeKeyValue, std::string name, int64_t tmpSelfTime);
    std::vector<Protocol::SimpleSlice> ThreadsInfoFilter(std::vector<Protocol::SimpleSlice> &simpleSliceVec, int64_t startTime, int64_t endTime);
    void ReduceThread(std::vector<Protocol::SimpleSlice> &rows, std::map<std::string, int64_t> &selfTimeKeyValue, Protocol::UnitThreadsBody &responseBody);
    bool QueryDurationFromSliceByTimeRange(Protocol::ThreadDetailParams &requestParams, const std::vector<Protocol::SliceDto> &rows,
            std::vector<int64_t> &nextDepthResult, int64_t trackId);
    bool QuerySliceFlowList(const std::string flowId, const std::string type, std::vector<Protocol::SliceFlowDetail> &sliceFlowDetailVec);
};
} // end of namespace Core
} // end of namespace Scene
} // end of namespace Dic

#endif // PROFILER_SERVER_TRACEDATABASE_H
