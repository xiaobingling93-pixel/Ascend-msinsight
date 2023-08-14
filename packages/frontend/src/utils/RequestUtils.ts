/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { isNull } from '../components/communicationAnalysis/Common';
import { communicationAnalysisData, computationCommunicationData, OperatorDetailsData } from './__test__/mockData';

/**
 * 查询所有迭代ID
 * 无参
 * @return {[]} 返回迭代数组[0,1,2,3]
 */
export const queryIterations = async(): Promise<any> => {
    if (isNull(window.request)) {
        return [ 0, 1, 2, 3 ];
    }
    return window.request('communication/duration/iterations', {});
};

/**
 * 查询所有Rank ID
 * 无参
 * @return {[]} 返回Rank数组[0,1,2,3]
 */
export const queryAllRanks = async(): Promise<any> => {
    if (isNull(window.request)) {
        return [ 0, 1, 2, 3 ];
    }
    return window.request('communication/duration/ranks', {});
};

/**
 * 查询迭代ID下某些RankID上运行的算子的名字
 * @param {string} iterationId 迭代ID
 * @param {number[]} rankIds rankId数组
 * @return {[]} 返回算子名数组[0,1,2,3]
 */
export const queryOperators = async(param: {iterationId: string | number;rankIds: number[]}): Promise<any> => {
    if (isNull(window.request)) {
        return [ 'op1', 1, 2, 3 ];
    }
    return window.request('communication/duration/operatorNames', param);
};

/**
 * 查询通信时间详情
 *
 * @param {string} iterationId 迭代ID
 * @param {number[]} rankIds
 * @param {string} operatorName 算子名
 * @return {[]} 返回数组
 */
export const queryCommunication = async(param: {
    iterationId: string | number; rankIds: number[]; operatorName: string;
}): Promise<any> => {
    if (isNull(window.request)) {
        return communicationAnalysisData;
    }
    return window.request('communication/duration/list', param);
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
    rankId: string; pageSize: number;currentPage: number;}): Promise<any> => {
    if (isNull(window.request)) {
        return OperatorDetailsData;
    }
    return window.request('communication/duration/operatorDetails', param);
};

/**
 * 查询计算分析总览通信/计算时间统计值
 *
 * @param {rankId} rankId RankID
 * @param {string} timeFlag 类型：SCMA、
 * @return {[]} 返回数组
 */
export const querySummaryStatistics = async (param: {rankId: string; timeFlag: string}): Promise<any> => {
    if (isNull(window.request)) {
        return computationCommunicationData[param.timeFlag];
    }
    return window.request('summary/statistic', param);
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
export const querySummaryDetail = async (param: {
    rankId: string; timeFlag: string; pageSize: number;currentPage: number;}): Promise<any> => {
    if (isNull(window.request)) {
        return [ ];
    }
    return window.request('summary/statistic/detail', param);
};

export const queryTopSummary = async (param:
{
    step: string | number;
    rankIds: string[];
    order: string | number;
    top: number;
},
): Promise<any> => {
    if (isNull(window.request)) {
        return {
            summaryList: computationCommunicationData.top,
            rankCount: 9,
            stepNum: 10,
        };
    }
    return window.request('summary/queryTopData', param);
};
