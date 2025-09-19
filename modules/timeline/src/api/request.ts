/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import {
    GetAICoreParams,
    GetAICoreParamsResult,
    GetOverallMetricsMoreListParams,
    GetOverallMetricsMoreListResult,
    GetOverallMetricsParams,
    GetOverallMetricsResult,
    GetOverallMetricsResultItem,
    ParseCardsParam,
    SetCardAliasParams,
    QueryOperatorDispatchParams,
    OperatorDispatchResult,
    QueryAllSameOperatorsDurationResult,
    QueryAllSameOperatorsDurationParams,
    QueryCommunicationKernelDetailParams,
    QueryCommunicationKernelDetailResult,
    CreateCurveParams, CreateCurveResult,
} from './interface';
import connector from '../connection';

// 根据cardId集合解析timeline
export const parseCards = async(param: ParseCardsParam): Promise<any> => {
    return window.requestData('parse/cards', param, 'timeline');
};

// 获取system view综合指标列表
export const getOverallMetrics = async (params: GetOverallMetricsParams): Promise<GetOverallMetricsResult> => {
    const res = await window.requestData('systemView/overall', params, 'timeline');
    // 递归设置 categoryList 字段
    const maxDepth = 10;
    const addCatField = (data: GetOverallMetricsResultItem[], parentCat: string[] = [], depth = 1): void => {
        if (depth > maxDepth) {
            return;
        }
        data?.forEach(item => {
            item.categoryList = [...parentCat, item.name];

            if (item.children && item.children.length > 0) {
                addCatField(item.children, item.categoryList, depth + 1);
            }
        });
    };

    if (res.data.length === 0) {
        return new Promise((resolve) => {
            connector.addListener('OverallMetrics', async () => {
                const result = await window.requestData('systemView/overall', params, 'timeline');
                addCatField(result.data);
                resolve(result);
            });
        });
    }
    addCatField(res.data);
    return res;
};

// 获取system view综合指标详细算子列表
export const getOverallMetricsMoreList = async (params: GetOverallMetricsMoreListParams): Promise<GetOverallMetricsMoreListResult> => {
    return window.requestData('systemView/overall/more/details', params, 'timeline');
};

// 设置卡级别别名
export const setCardAliasReq = async (params: SetCardAliasParams): Promise<any> => {
    return window.requestData('unit/setCardAlias', params, 'timeline');
};

// 获取ExpertSummary中的AI Core降频分析
export const queryExpertAnalysis = async (params: GetAICoreParams): Promise<GetAICoreParamsResult> => {
    return window.requestData('expertAnalysis/AICoreFreq', params, 'timeline');
};

// 获取算子下发路径问题列表
export const queryOperatorDispatch = async (params: QueryOperatorDispatchParams): Promise<OperatorDispatchResult> => {
    return window.requestData('advisor/operatorDispatch', params, 'advisor');
};

export async function queryAllSameOperatorsDuration(params: QueryAllSameOperatorsDurationParams): Promise<QueryAllSameOperatorsDurationResult> {
    return window.requestData('query/all/same/operators/duration', params, 'timeline');
}

export async function queryCommunicationKernelDetail(params: QueryCommunicationKernelDetailParams): Promise<QueryCommunicationKernelDetailResult> {
    return window.requestData('unit/kernelDetail', params, 'timeline');
}

export async function createCurve(params: CreateCurveParams): Promise<CreateCurveResult> {
    return window.requestData('create/curve', params, 'timeline');
}
