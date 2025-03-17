/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "TraceDatabaseHelper.h"
#include "SystemViewOverallTextRepo.h"
using namespace Dic::Protocol;
namespace Dic::Module::Timeline {
std::vector<OverallTmpInfo> SystemViewOverallTextRepo::QueryOverlapAnalysisDataForOverallMetric(
    const Protocol::SystemViewOverallReqParam &requestParams, const std::shared_ptr<VirtualTraceDatabase> &database)
{
    std::string sql =
        "with overlap as (select end_time, timestamp, thread_name, s.duration from thread t join process p "
        "on p.pid = t.pid join slice s on t.track_id = s.track_id where p.process_name = 'Overlap Analysis' "
        "and t.thread_name != 'Communication') "
        "select thread_name as category, round(sum(duration) / 1000.0, 2) as duration from overlap "
        " group by thread_name order by category;";
    auto stmt = database->CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to prepare sql while querying overlap analysis data for overall metrics.");
        return {};
    }
    std::vector<OverallTmpInfo> overlapInfos;
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to execute query while querying overlap analysis data for overall metrics.");
        return {};
    }
    while (resultSet->Next()) {
        OverallTmpInfo tmpInfo;
        tmpInfo.categoryList.push_back(resultSet->GetString("category"));
        tmpInfo.duration = resultSet->GetDouble("duration");
        overlapInfos.push_back(tmpInfo);
    }
    return overlapInfos;
}

bool SystemViewOverallTextRepo::QueryDataForComputingOverallMetric(
    const Protocol::SystemViewOverallReqParam &requestParams, SystemViewOverallHelper &computeHelper,
    const std::shared_ptr<VirtualTraceDatabase> &database)
{
    // 检查是否存在表kernel details, 列aiv_vec_time或列mac_time，若都不存在，则日志报警并跳过Computing查询
    if (!CheckDataForSystemViewOverall(database)) {
        return true;
    }
    std::map<uint64_t, uint64_t> flowDict = QueryFlowDict(database); // <key: flow end time, value: flow start time>
    computeHelper.cpuCubeOps = QueryCpuCubeOp(database);
    computeHelper.kernelEvents = QueryKernelEventsForSystemViewOverall(flowDict, database);

    // 查询backward track id
    QueryBwdTrackIdForComputingOverall(computeHelper.bwdTrackId, database);
    return true;
}

bool SystemViewOverallTextRepo::CheckDataForSystemViewOverall(const std::shared_ptr<VirtualTraceDatabase> &database)
{
    if (!database->CheckTableExist(TABLE_KERNEL)) {
        ServerLog::Warn("Missing key table while querying computing data in system view overall. Can't find ",
                        TABLE_KERNEL);
        return false;
    }
    if (database->CheckColumnExist(TABLE_KERNEL, "aiv_vec_time_us_")) {
        return true;
    }
    if (database->CheckColumnExist(TABLE_KERNEL, "mac_time_us_")) {
        database->hasMacTime = true;
        return true;
    }
    ServerLog::Warn("Missing key columns while querying computing data in system view overall. Please ensure "
                    "that the profiling data is set to level 1 or higher and aic_metrics is set to PipeUtilization.");
    return false;
}

std::map<uint64_t, uint64_t> SystemViewOverallTextRepo::QueryFlowDict(
    const std::shared_ptr<VirtualTraceDatabase> &database)
{
    std::string sql =
        "select max(case when type = 's' then timestamp end) as start, max(case when type = 'f' then timestamp end) "
        " as end from " + FLOW_TABLE + " where cat = 'async_npu' group by flow_id having "
        " count(case when type = 's' then 1 end) = 1 and count (case when type = 'f' then 1 end) = 1 order by end;";
    auto stmt = database->CreatPreparedStatement(sql);
    std::map<uint64_t, uint64_t> flowDict;
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to query flow dictionary for system view overall.");
        return {};
    }
    while (resultSet->Next()) {
        flowDict[resultSet->GetUint64("end")] = resultSet->GetUint64("start");
    }
    return flowDict;
}

std::vector<CpuCubeOpInfo> SystemViewOverallTextRepo::QueryCpuCubeOp(
    const std::shared_ptr<VirtualTraceDatabase> &database)
{
    std::string sql = "select timestamp as start, end_time as end, name, track_id "
                      " from slice where cat = 'cpu_op' order by start;";
    auto stmt = database->CreatPreparedStatement(sql);
    std::vector<CpuCubeOpInfo> cpuCubeOps;
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to query cpu cube operators for system view overall.");
        return {};
    }
    while (resultSet->Next()) {
        CpuCubeOpInfo cubeOp;
        cubeOp.pythonApi = resultSet->GetString("name");
        cubeOp.CheckCubeOp();
        if (cubeOp.isCubeOp) {
            cubeOp.start = resultSet->GetUint64("start");
            cubeOp.end = resultSet->GetUint64("end");
            cubeOp.trackId = resultSet->GetUint64("track_id");
            cpuCubeOps.push_back(cubeOp);
        }
    }
    return cpuCubeOps;
}

std::vector<OverallTmpInfo> SystemViewOverallTextRepo::QueryKernelEventsForSystemViewOverall(
    const std::map<uint64_t, uint64_t> &flowDict, const std::shared_ptr<VirtualTraceDatabase> &database)
{
    std::string sql =
        "select name as opName, op_type as opType, start_time as startTime, duration, ";
    if (!database->hasMacTime) {
        sql += " aicore_time_us_ from kernel_detail where aiv_vec_time_us_ != 'N/A' and aiv_vec_time_us_ != '' "
               " order by start_time;";
    } else {
        sql += " aicore_time_us_, mac_time_us_ from kernel_detail where mac_time_us_ != 'N/A' and mac_time_us_ != '' "
               " order by start_time;";
    }
    auto stmt = database->CreatPreparedStatement(sql);
    std::vector<OverallTmpInfo> kernelEvents;
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to query kernel events for system view overall.");
        return {};
    }
    while (resultSet->Next()) {
        OverallTmpInfo kernelEvent;
        kernelEvent.opName = resultSet->GetString("opName");
        kernelEvent.opType = resultSet->GetString("opType");
        kernelEvent.startTime = resultSet->GetUint64("startTime");
        auto it = flowDict.find(kernelEvent.startTime);
        if (it != flowDict.end()) {
            kernelEvent.flowStartTime = it->second;
        }
        kernelEvent.duration = resultSet->GetDouble("duration");
        kernelEvent.aicoreTime = stod(resultSet->GetString("aicore_time_us_"));
        if (database->hasMacTime) {
            kernelEvent.macTime = stod(resultSet->GetString("mac_time_us_"));
        }
        kernelEvents.push_back(kernelEvent);
    }

    // 按flow start time升序排序
    sort(kernelEvents.begin(), kernelEvents.end());
    return kernelEvents;
}

void SystemViewOverallTextRepo::QueryBwdTrackIdForComputingOverall(uint64_t& bwdTrackId,
    const std::shared_ptr<VirtualTraceDatabase> &database)
{
    // 查询backward track id
    std::string sql = "select track_id from flow where cat = 'fwdbwd' and type = 'f' limit 1;";
    auto stmt = database->CreatPreparedStatement(sql);
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to query backward track id for system view overall.");
        return;
    }
    while (resultSet->Next()) {
        bwdTrackId = resultSet->GetUint64("track_id");
    }
}

void SystemViewOverallTextRepo::QueryCommunicationOverlapOverallInfos(
    const Protocol::SystemViewOverallReqParam &requestParams, double e2eTime,
    std::vector<Protocol::SystemViewOverallRes> &responseBody, const std::shared_ptr<VirtualTraceDatabase> &database)
{
    // 查询通信未掩盖数据
    std::vector<Protocol::ThreadTraces> uncovered{};
    uint64_t totalTime = 0;
    if (!database->QueryOverlapAnalysisData(QUERY_OVERLAP_ANALYSIS_BY_TYPE_TEXT_SQL,
        "Communication(Not Overlapped)", 0, uncovered, totalTime)) {
        return; // QueryOverlapAnalysisData has all needed log
    }
    auto it = std::find_if(responseBody.begin(), responseBody.end(), [](const Protocol::SystemViewOverallRes& item) {
        return item.name == COMMUNICATION_NOT_OVERLAP_TIME;
    });
    if (it == responseBody.end()) {
        double ratio = 0;
        double notOverlapTime = NS_TO_US * totalTime;
        if (e2eTime != 0) {
            ratio = NumberUtil::DoubleReservedNDigits(notOverlapTime / e2eTime * PERCENTAGE_RATIO_SCALE, TWO);
        }
        Protocol::SystemViewOverallRes notOverlapped = {
            .totalTime = notOverlapTime, .ratio = ratio, .nums = 0, .avg = 0, .max = 0, .min = 0,
            .name = COMMUNICATION_NOT_OVERLAP_TIME, .children = {}, .level = 1, // level 1
            .id = std::to_string(Protocol::SystemViewOverallRes::idCounter.fetch_add(1))
        };
        responseBody.emplace_back(notOverlapped);
    }
    // 查询泳道与通信Group间的对应关系
    std::map<std::string, std::string> groupMap{};
    if (!database->QueryCommunicationGroupMap(QUERY_COMMUNICATION_GROUP_MAP_TEXT_SQL, groupMap)) {
        return;
    }
    // 查询HCCL层通信算子和通信Task，并与通信未掩盖数据取交集，对于通信Task进一步区分wait和Transmit
    std::string sql4Summary = TraceDatabaseHelper::GeneratorCommunicationSummarySql4Text({}, {});
    // 如果不存在上文就会添加，因此此处一定能找到
    it = std::find_if(responseBody.begin(), responseBody.end(), [](const Protocol::SystemViewOverallRes& item) {
        return item.name == COMMUNICATION_NOT_OVERLAP_TIME;
    });
    database->CalculateCommunicationSummaryData(uncovered, groupMap, sql4Summary, e2eTime, *it);
}

bool SystemViewOverallTextRepo::QueryCommunicationOpsTimeDataByGroupName(const std::string &name, uint64_t offset,
    const std::vector<Protocol::ThreadTraces> &notOverlapData, std::vector<SameOperatorsDetails> &opsDetails,
    const std::shared_ptr<VirtualTraceDatabase> &database)
{
    // 找到指定group name对应的track_id，以便下一步sql查询
    auto stmt = database->CreatPreparedStatement(QUERY_COMMUNICATION_GROUP_ID_TEXT_SQL);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to prepare sql for Query Communication Group Id By Name.");
        return false;
    }
    uint64_t groupId = TraceDatabaseHelper::QueryCommunicationGroupIdByName(stmt, name);
    if (groupId == UINT64_MAX) {
        ServerLog::Error("Failed to Query Communication Ops Time Data due to illegal group name.");
        return false;
    }
    auto stmt2 = database->CreatPreparedStatement(QUERY_COMMUNICATION_OP_BY_GROUP_ID_TEXT_SQL);
    if (stmt2 == nullptr) {
        ServerLog::Error("Failed to prepare sql for query communication ops time data for text scene.");
        return false;
    }
    if (!TraceDatabaseHelper::QueryCommunicationOpTimeDataByGroupId(stmt2,
                                                                    groupId, offset, notOverlapData, opsDetails)) {
        ServerLog::Error("Failed to Query Communication Ops Time Data due to incorrect sql execution.");
        return false;
    }
    return true;
}
}