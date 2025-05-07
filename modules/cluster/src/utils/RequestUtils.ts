/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import {
    GetParallelismPerformanceData,
    GetParallelismPerformanceRes,
    GetParallelStrategyRes, ImportExpertDataParams,
    ParallelismArrangementParams,
    ParallelismArrangementResult,
    QueryModelInfoResult,
    QueryExpertHotspotParams,
    QueryExpertHotspotResult,
    SetParallelStrategyParams,
} from './interface';
import { createCancelableApi } from 'ascend-utils';

/**
 * 查询所有迭代ID
 * 无参
 * @return {[]} 返回迭代数组[0,1,2,3]
 */
export const queryIterations = async(param: {isCompare: boolean}): Promise<any> => {
    return window.requestData('communication/duration/iterations', param);
};

/**
 * 查询一次迭代下所有通信域
 * 无参
 * @return {[]} 返回迭代数组['(0,1,2)']
 */
export const queryStages = async(param: {iterationId: string;baselineIterationId: string;isCompare: boolean }): Promise<any> => {
    return window.requestData('communication/matrix/group', param);
};

/**
 * 查询一次迭代下所有Rank ID
 * @param {string} iterationId 迭代ID
 * @return {[]} 返回Rank数组[0,1,2,3]
 */
export const queryRanks = async(param: {iterationId: string }): Promise<any> => {
    return window.requestData('communication/duration/ranks', param);
};

/**
 * 查询迭代ID下某些RankID上运行的算子的名字
 * @param {string} iterationId 迭代ID
 * @param {number[]} rankList rankId数组
 * @return {[]} 返回算子名数组[0,1,2,3]
 */
export const queryOperators = async(param: {iterationId: string ;stage: string; pgName: string}): Promise<any> => {
    return window.requestData('communication/duration/operatorNames', param);
};

/**
 * 查询通信矩阵中所有算子的名称
 * @param {string} iterationId 迭代ID
 * @param {string} stage 通信域
 * @return {[]} 返回算子名数组[0,1,2,3]
 */
export const queryMatrixOperators = async(param: {iterationId: string ;stage: string; pgName: string}): Promise<any> => {
    return window.requestData('communication/matrix/sortOpNames', param);
};

/**
 * 查询通信时间详情
 *
 * @param {string} iterationId 迭代ID
 * @param {number[]} rankIds
 * @param {string} operatorName 算子名
 * @return {[]} 返回数组
 */
export const queryCommunication = async(param: { iterationId: string ; operatorName: string ; pgName: string}): Promise<any> => {
    return window.requestData('communication/duration/list', param);
};

/**
 * 查询本张卡的通信耗时分析中的专家建议
 * 无参
 * @return {[]} 返回数组
 */
export const queryCommunicationExpertAdvisor = async(): Promise<any> => {
    return window.requestData('communication/advisor', {});
};

/**
 * 查询通信算子时间列表
 *
 * @param {string} iterationId 迭代ID
 * @param {number[]} rankIds
 * @param {string} operatorName 算子名
 */
export const queryCommunicationOperatorLists = async(param: { iterationId: string ; operatorName: string ; pgName: string}): Promise<any> => {
    return window.requestData('communication/operatorLists', param);
};

/**
 * 分页查询某个算子的通信时间详情
 *
 * @param {rankId} rankId RankID
 * @param {number} pageSize
 * @param {number} currentPage
 * @return {[]} 返回数组
 */
export const queryOperatorDetails = async(param: {
    iterationId: number; rankId: number; pageSize: number;currentPage: number;orderBy: string;order: string;
    stage: string;queryType?: string;pgName: string;
}): Promise<any> => {
    return window.requestData('communication/operatorDetails', param);
};

/**
 * 查询计算分析总览通信/计算时间统计值
 *
 * @param {rankId} rankId RankID
 * @param {string} timeFlag 类型：SCMA、
 * @return {[]} 返回数组
 */
export const querySummaryStatistics = async (param: {rankId: string; timeFlag: string;stepId: string}): Promise<any> => {
    return window.requestData('summary/statistic', param);
};

/**
 * 查询计算分析总览通信/计算时间详情
 *
 * @param {rankId} rankId RankID
 * @param {string} timeFlag 类型：Communication、Computing……
 * @param {number} pageSize
 * @param {number} currentPage
 * @return {[]} 返回数组
 */
export const queryComputeDetail = async (param: {
    rankId: string; timeFlag: string; pageSize: number;currentPage: number;orderBy: string;order: string;
}): Promise<any> => {
    return window.requestData('summary/queryComputeDetail', param);
};

export const queryCommunicationDetail = async (param: {
    rankId: string; pageSize: number;currentPage: number;orderBy: string;order: string;}): Promise<any> => {
    return window.requestData('summary/queryCommunicationDetail', param);
};

/**
 *
 * @param param
 *  step: string | number;
 *  rankIds: string[];
 *  orderBy: string ;
 *  top: number;
 */
export const queryTopSummary = async (param: {isCompare?: boolean}): Promise<any> => {
    return window.requestData('summary/queryTopData', {
        ...param,
        stepIdList: [],
    });
};

/**
 * 查询通信矩阵
 *
 * @param {string} iterationId 迭代ID
 * @param {string} stage
 * @param {string} operatorName 算子名
 * @return {[]} 返回数组
 */
export const queryCommunicationMatrix = async(param: { iterationId: string ; pgName: string ; stage: string ; operatorName: string; isCompare: boolean}):
Promise<any> => {
    return window.requestData('communication/matrix/bandwidthInfo', param);
};

export interface QueryFwpBwdTimelineParams {
    stepId: string;
    stageId: string;
}
export interface TraceItem {
    name: string;
    start: number;
    duration: number;
    pid: string;
    tid: string;
    id: string;
    cname: 'FP' | 'BP' | 'SEND' | 'RECV' | 'BATCH_SEND_RECV';
}

interface RankItem {
    rank: string;
    componentList: Array<{
        component: 'FP/BP' | 'P2P Op';
        traceList: TraceItem[];
    }>;
}
export interface QueryFwpBwdTimelineRes {
    minTime: number;
    maxTime: number;
    rankList: RankItem[];
}

export const queryFwpBwdTimeline = async(params: QueryFwpBwdTimelineParams): Promise<QueryFwpBwdTimelineRes> => {
    return window.requestData('parallelism/pipeline/fwdBwdTimeline', params, 'summary');
};

export const getParallelStrategy = async (): Promise<GetParallelStrategyRes> => {
    return await window.requestData('summary/query/parallelStrategy', {}, 'summary');
};

export const setParallelStrategy = async (params: SetParallelStrategyParams): Promise<void> => {
    return await window.requestData('summary/set/parallelStrategy', params, 'summary');
};

/**
 * 获取并行策略排布数据
 * @param {ParallelismArrangementParams} params
 * @return {ParallelismArrangementResult}
 */
export const queryParallelismArrangementCancelable = createCancelableApi(
    async(params: ParallelismArrangementParams): Promise<ParallelismArrangementResult> => {
        return await window.requestData('parallelism/arrangement/all', params, 'summary');
    },
);

export const queryAllConnections = async(params: ParallelismArrangementParams): Promise<ParallelismArrangementResult> => {
    return await window.requestData('parallelism/arrangement/all', params, 'summary');
};

export const getParallelismPerformanceDataCancelable = createCancelableApi(
    async (params: GetParallelismPerformanceData): Promise<GetParallelismPerformanceRes> => {
        return await window.requestData('parallelism/performance/data', params, 'summary');
    },
);

/**
 * 导入 MOE 专家负载均衡数据
 */
export const importExpertData = async(params: ImportExpertDataParams): Promise<void> => {
    return await window.requestData('summary/importExpertData', params, 'summary');
};

/**
 * 查询 MOE 专家负载均衡数据
 */
export const queryExpertHotspot = async(params: QueryExpertHotspotParams): Promise<QueryExpertHotspotResult> => {
    return await window.requestData('summary/queryExpertHotspot', params, 'summary');
};

/**
 * 查询 MOE 专家负载均衡搜索条件
 */
export const queryModelInfo = async(): Promise<QueryModelInfoResult> => {
    return await window.requestData('summary/queryModelInfo', {}, 'summary');
};
