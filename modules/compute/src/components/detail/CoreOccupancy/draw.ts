/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import type { ICore } from './Index';
import { type ShowAs } from './Filter';
import type { Theme } from '@emotion/react';

interface DataConfig {
    data: ICore[];
    showAs: ShowAs;
}

export interface CoreDrawData {
    name: string;
    children: SubCoreDrawData[];
}
interface SubCoreDrawData {
    name: string;
    value: string;
    level: string;
}

// 画布、节点、图例等尺寸
export const sizeConfig = {
    core: {
        width: 130,
        height: 280,
        space: 15,
        title: {
            top: 20,
        },
        border: 1,
    },
    subCore: {
        width: 90,
        height: 33,
        heightSpace: 15,
        top: 35,
    },
    legend: {
        width: 70,
        height: 300,
        top: 10,
        left: 10,
        title: {
            height: 10,
        },
        block: {
            width: 30,
            height: 25,
        },
    },
};

export const COLOR: Record<string, string> = {
    1: '#BC2021',
    2: '#D32322',
    3: '#EC2829',
    4: '#ED3D3D',
    5: '#EF5353',
    6: '#A7CE52',
    7: '#94C32B',
    8: '#81BA06',
    9: '#74A604',
    10: '#679405',
};

const MAX_TEXT_LENGTH = 50;

// 图例
export function getLegendData(theme: Theme): Array<{ level: number; color: string}> {
    const legendData = [];
    for (let i = 10; i >= 0; i--) {
        if (i === 0) {
            legendData.push({ level: i, color: theme.bgColorGrey });
            continue;
        }
        legendData.push({ level: i, color: COLOR[i] });
    }
    return legendData;
}

// 画图
export function getDrawData({ data: originData, showAs }: DataConfig): CoreDrawData[] {
    const subCoreNameList: string[] = ['Cube0', 'Vector0', 'Vector1'];
    const data = originData.map(item => ({
        name: `Core${item.coreId}`.slice(0, MAX_TEXT_LENGTH),
        children: subCoreNameList.map(subCoreName => {
            const subCore = item.subCoreDetails.find(subCoreItem => (subCoreItem.subCoreName ?? '').toLowerCase() === subCoreName.toLowerCase())?.[showAs];
            return {
                name: subCoreName,
                value: String(getSubCoreValue(showAs, subCore?.value)).slice(0, MAX_TEXT_LENGTH),
                level: String(subCore?.level ?? '').slice(0, MAX_TEXT_LENGTH),
            };
        }),
    }));
    return data;
}

function getSubCoreValue(showAs: ShowAs, value?: number): string | number {
    switch (showAs) {
        case 'throughput':
            return formatThroughput(value);
        case 'cacheHitRate':
            return value !== undefined ? Number((value).toFixed(2)) : '';
        case 'cycles':
            return String(value);
        default:
            return '';
    }
}
const K = 1024;
const M = 1024 * 1024;
const G = 1024 * 1024 * 1024;
function formatThroughput(value?: number | string): string {
    const num = Number(value);
    if (num < K) {
        return `${num}Byte`;
    } else if (num < M) {
        return `${Number((num / K).toFixed(2))} KB`;
    } else if (num < G) {
        return `${Number((num / M).toFixed(2))} MB`;
    } else {
        return `${Number((num / G).toFixed(2))} GB`;
    }
}
// 颜色
export function getSubCoreColor(level: string, theme: Theme): string {
    COLOR[0] = theme.bgColorGrey;
    return COLOR[level] ?? COLOR[0];
}
