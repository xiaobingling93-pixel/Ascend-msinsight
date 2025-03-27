/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import {
    GetOverallMetricsMoreListParams, GetOverallMetricsMoreListResult,
    GetOverallMetricsParams,
    GetOverallMetricsResult, GetOverallMetricsResultItem,
    ParseCardsParam,
    SetCardAliasParams,
} from './interface';

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
