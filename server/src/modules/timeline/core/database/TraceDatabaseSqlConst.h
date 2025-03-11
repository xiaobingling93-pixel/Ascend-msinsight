/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_TRACEDATABASESQLCONST_H
#define PROFILER_SERVER_TRACEDATABASESQLCONST_H

#include <string>
namespace Dic::Module::Timeline {
const std::string QUERY_FWDBWD_FLOW_DATA_TEXT_SQL =
    "WITH combined AS ( \n"
    "    SELECT f.flow_id, f.type, f.timestamp AS slice_begin, s.end_time AS slice_end \n"
    "    FROM flow f JOIN slice s ON f.track_id = s.track_id AND f.timestamp = s.timestamp \n"
    "    WHERE f.timestamp >= ? AND f.timestamp <= ? AND f.cat = 'fwdbwd' AND f.type IN ('s', 'f') \n"
    "    ORDER by f.flow_id \n"
    "), " // 过滤flow表里前反向数据，并与slice表join，以获取每个元素的结束时间
    "fwd AS ( \n"
    "    SELECT flow_id, slice_begin, slice_end FROM combined WHERE type = 's' \n"
    "), \n"
    "bwd AS ( \n"
    "    SELECT flow_id, slice_begin, slice_end FROM combined WHERE type = 'f' \n"
    "), \n"
    "flowsAscByFwd AS ( \n"
    "    SELECT fwd.slice_begin AS fwdStart, fwd.slice_end AS fwdEnd, \n"
    "    bwd.slice_begin AS bwdStart, bwd.slice_end AS bwdEnd, \n"
    "    ROW_NUMBER() OVER (ORDER BY fwd.slice_begin) AS rowNum \n"
    "    FROM fwd JOIN bwd ON fwd.flow_id = bwd.flow_id \n"
    "), " // 将前反向联系起来，并按前向起始时间升序排列，以方便后续识别前反向边界
    "increaseEndIndex AS ( \n"
    "    SELECT CASE WHEN d1.rowNum != 1 THEN d1.rowNum END as endIndex \n"
    "    FROM flowsAscByFwd d1 LEFT JOIN flowsAscByFwd d2 ON d2.rowNum = d1.rowNum + 1\n"
    "    WHERE d2.bwdStart > d1.bwdStart OR d2.bwdStart IS NULL OR d1.rowNum = 1\n"
    "), " // 按前向开始时间升序处理，前向递增，反向递减，前反向边界，反向有个增大的突变(vpp场景下，vpp的边界是例外，后续需要特殊处理)
    "flowsAscByBwd AS ( \n"
    "    SELECT fwdStart, bwdStart, rowNum as oldRowNum, \n"
    "    ROW_NUMBER() OVER (ORDER BY bwdStart) AS rowNum \n"
    "    FROM flowsAscByFwd \n"
    "), " // 按反向起始时间升序排列，以方便后续识别前反向边界
    "decreaseEndIndex AS ( \n"
    "    SELECT d1.oldRowNum as endIndex \n"
    "    FROM flowsAscByBwd d1 LEFT JOIN flowsAscByBwd d2 ON d2.rowNum = d1.rowNum - 1 \n"
    "    WHERE d1.fwdStart > d2.fwdStart OR d2.fwdStart IS NULL OR d1.rowNum = 1 \n"
    "), " // 按反向开始时间升序排列，过滤出潜在的突变点，作为正向处理的补充，以解决vpp场景下
    "possibleIndex AS ( \n"
    "    SELECT * FROM increaseEndIndex WHERE endIndex is NOT NULL\n"
    "    UNION \n"
    "    SELECT * FROM decreaseEndIndex WHERE endIndex is NOT NULL\n"
    "    ORDER BY endIndex \n"
    "), " // 将前向开始时间升序识别的边界与后向开始时间升序识别的边界进行合并
    "possibleData AS ( \n"
    "    SELECT \n"
    "        CASE WHEN d1.rowNum != 1 THEN d1.fwdEnd ELSE 0 END as nextFpEnd, \n"
    "        CASE WHEN d1.rowNum != 1 THEN d1.bwdStart ELSE 0 END as nextBpStart, \n"
    "        COALESCE(d2.fwdStart, 0) AS prevFpStart, \n"
    "        COALESCE(d2.bwdEnd, 0) AS prevBpEnd, \n"
    "        ROW_NUMBER() OVER (ORDER BY d1.rowNum) AS rowNum \n"
    "    FROM flowsAscByFwd d1 LEFT JOIN flowsAscByFwd d2 ON d2.rowNum = d1.rowNum + 1 \n"
    "    WHERE d1.rowNum = 1 OR d1.rowNum in possibleIndex \n"
    ") \n"
    "SELECT d1.prevFpStart as fpStart, d1.prevBpEnd as bpEnd, d2.nextFpEnd as fpEnd, d2.nextBpStart as bpStart, \n"
    "d2.nextFpEnd - d1.prevFpStart as fpDuration, d1.prevBpEnd - d2.nextBpStart as bpDuration \n"
    "FROM possibleData d1 JOIN possibleData d2 ON d2.rowNum = d1.rowNum + 1";

const std::string QUERY_FWDBWD_FLOW_DATA_DB_SQL =
    "with type as ( \n"
    "    SELECT id FROM ENUM_API_TYPE WHERE name = 'op' \n"
    "), " // 过滤cpu_op对应的类型值
    "flowCat as ( \n"
    "    SELECT connectionId \n"
    "    FROM connectionCats WHERE cat = 'fwdbwd' ORDER BY connectionId \n"
    "), " // 过滤所有的前反向连线id
    "flowTable as ( \n"
    "    SELECT ids.id as flowId, ids.connectionId as connectionId \n"
    "    FROM flowCat cats JOIN CONNECTION_IDS ids \n"
    "    ON cats.connectionId = ids.connectionId ORDER by ids.id ASC \n"
    "), " // 过滤前反向连线flowId和connectId数据
    "apiTable as ( \n"
    "    SELECT startNs, endNs, connectionId FROM PYTORCH_API \n"
    "    WHERE connectionId IS NOT NULL AND type in type \n"
    "    AND startNs >= ? and endNs <= ? \n"
    "    ORDER BY connectionId \n"
    "), " // 过滤出所有的cpu_op数据
    "combined as ( \n"
    "    SELECT startNs, endNs, flow.connectionId FROM flowTable flow join apiTable api \n"
    "    ON flow.flowId = api.connectionId ORDER BY flow.connectionId ASC \n"
    "), " // 建立起连线与界面元素起始时间的联系
    "flowsAscByFwd as (\n"
    "    SELECT s.startNs as fwdStart, s.endNs as fwdEnd, f.startNs as bwdStart, f.endNs as bwdEnd, \n"
    "    ROW_NUMBER() OVER (ORDER BY s.startNs) AS rowNum \n"
    "    FROM combined s JOIN combined f \n"
    "    ON s.connectionId = f.connectionId AND s.startNs < f.startNs \n"
    "), " // 将前反向联系起来，并按前向起始时间升序排列，以方便后续识别前反向边界
    "increaseEndIndex AS ( \n"
    "    SELECT CASE WHEN d1.rowNum != 1 THEN d1.rowNum END as endIndex \n"
    "    FROM flowsAscByFwd d1 LEFT JOIN flowsAscByFwd d2 ON d2.rowNum = d1.rowNum + 1 \n"
    "    WHERE d2.bwdStart > d1.bwdStart OR d2.bwdStart IS NULL OR d1.rowNum = 1 \n"
    "), " // 按前向开始时间升序处理，前向递增，反向递减，前反向边界，反向有个增大的突变(vpp场景下，vpp的边界是例外，后续需要特殊处理)
    "flowsAscByBwd AS (\n"
    "    SELECT fwdStart, bwdStart, rowNum as oldRowNum, \n"
    "    ROW_NUMBER() OVER (ORDER BY bwdStart) AS rowNum \n"
    "    FROM flowsAscByFwd \n"
    "), " // 按反向起始时间升序排列，以方便后续识别前反向边界
    "decreaseEndIndex AS ( \n"
    "    SELECT d1.oldRowNum as endIndex \n"
    "    FROM flowsAscByBwd d1 LEFT JOIN flowsAscByBwd d2 ON d2.rowNum = d1.rowNum - 1 \n"
    "    WHERE d1.fwdStart > d2.fwdStart OR d2.fwdStart IS NULL OR d1.rowNum = 1 \n"
    "), " // 按反向开始时间升序排列，过滤出潜在的突变点，作为正向处理的补充，以解决vpp场景下
    "possibleIndex AS ( \n"
    "    SELECT endIndex FROM increaseEndIndex WHERE endIndex is NOT NULL \n"
    "    UNION \n"
    "    SELECT endIndex FROM decreaseEndIndex WHERE endIndex is NOT NULL \n"
    "    ORDER BY endIndex \n"
    "), " // 将前向开始时间升序识别的边界与后向开始时间升序识别的边界进行合并
    "possibleData AS ( \n"
    "    SELECT \n"
    "        CASE WHEN d1.rowNum != 1 THEN d1.fwdEnd ELSE 0 END as nextFpEnd, \n"
    "        CASE WHEN d1.rowNum != 1 THEN d1.bwdStart ELSE 0 END as nextBpStart, \n"
    "        COALESCE(d2.fwdStart, 0) AS prevFpStart, \n"
    "        COALESCE(d2.bwdEnd, 0) AS prevBpEnd, \n"
    "        ROW_NUMBER() OVER (ORDER BY d1.rowNum) AS rowNum \n"
    "    FROM flowsAscByFwd d1 LEFT JOIN flowsAscByFwd d2 ON d2.rowNum = d1.rowNum + 1 \n"
    "    WHERE d1.rowNum = 1 OR d1.rowNum in possibleIndex \n"
    ") " // 将flowsAscByFwd的当前行与下一行组装到一起，根据前反向索引，查询数据
    "SELECT d1.prevFpStart as fpStart, d1.prevBpEnd as bpEnd, d2.nextFpEnd as fpEnd, d2.nextBpStart as bpStart, \n"
    "d2.nextFpEnd - d1.prevFpStart as fpDuration, d1.prevBpEnd - d2.nextBpStart as bpDuration \n"
    "FROM possibleData d1 JOIN possibleData d2 ON d2.rowNum = d1.rowNum + 1";
}
#endif // PROFILER_SERVER_TRACEDATABASESQLCONST_H
