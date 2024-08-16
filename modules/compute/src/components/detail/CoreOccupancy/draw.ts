/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import type { ICore } from './Index';
import { type ShowAs } from './Filter';
import type { Theme } from '@emotion/react';

interface DataConfig {
    data: ICore[];
    showAs: ShowAs;
    svgWidth: number;
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

// 画布、节点、文字等尺寸
export const MIN_CHART_HEIGHT = 300;
export const MAX_CHART_HEIGHT = 600;
export const MIN_CHART_WIDTH = 500;
export const sizeConfig = {
    oneRowCount: 0,
    rowCount: 0,
    svgWidth: 0,
    svgHeight: 0,
    chartHeight: MIN_CHART_HEIGHT,
    core: {
        width: 120,
        height: 250,
        widthSpace: 30,
        heightSpace: 10,
        title: {
            top: 20,
        },
        border: 1,
    },
    subCore: {
        width: 80,
        height: 28,
        heightSpace: 15,
        top: 35,
    },
    legend: {
        width: 70,
        height: 300,
        top: 20,
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
export function getDrawData(config: DataConfig): CoreDrawData[] {
    // 计算尺寸
    setSize(config);
    // 格式化数据
    let data: CoreDrawData[] = [];
    if (sizeConfig.rowCount !== 0 && sizeConfig.oneRowCount !== 0) {
        data = wrapData(config);
    }
    return data;
}

// 设置画布尺寸
function setSize(config: DataConfig): void {
    const { svgWidth, data } = config;
    const { rowCount, oneRowCount, svgHeight, chartHeight } = computeSize(svgWidth, data.length);
    sizeConfig.svgWidth = svgWidth;
    sizeConfig.oneRowCount = oneRowCount;
    sizeConfig.rowCount = rowCount;
    sizeConfig.svgHeight = svgHeight;
    sizeConfig.chartHeight = chartHeight;
}

// 计算绘制尺寸
function computeSize(svgWidth: number, count: number): Record<string, number> {
    // 一行显示几个
    const oneCoreSize = sizeConfig.core.width + sizeConfig.core.widthSpace;
    const oneRowCount = oneCoreSize <= 0
        ? count
        : Math.floor((svgWidth - sizeConfig.legend.width - sizeConfig.core.width) / oneCoreSize) + 1;
    // 总共需要多少行
    const rowCount = oneRowCount <= 0 ? 0 : Math.ceil(count / oneRowCount);
    // 画布高度
    const svgHeight = rowCount <= 0
        ? 0
        : ((rowCount - 1) * (sizeConfig.core.height + sizeConfig.core.heightSpace)) + sizeConfig.core.height;
    // 图高度
    let chartHeight;
    if (svgHeight >= MIN_CHART_HEIGHT && svgHeight <= MAX_CHART_HEIGHT) {
        chartHeight = svgHeight + 15;
    } else if (svgHeight < MIN_CHART_HEIGHT) {
        chartHeight = MIN_CHART_HEIGHT;
    } else {
        chartHeight = MAX_CHART_HEIGHT;
    }
    return { rowCount, oneRowCount, svgHeight, chartHeight };
}

function wrapData({ data: originData, showAs }: DataConfig): CoreDrawData[] {
    const subCoreNameList: string[] = ['Cube0', 'Vector0', 'Vector1'];
    const data = originData.map(item => ({
        name: `Core${item.coreId}`.slice(0, MAX_TEXT_LENGTH),
        children: subCoreNameList.map(subCoreName => {
            const subCore = item.subCoreDetails.find(subCoreItem => subCoreItem.subCoreName === subCoreName)?.[showAs];
            return {
                name: subCoreName,
                value: String(subCore?.value ?? '').slice(0, MAX_TEXT_LENGTH),
                level: String(subCore?.level ?? '').slice(0, MAX_TEXT_LENGTH),
            };
        }),
    }));
    return data;
}

export function isLastInRow(index: number): boolean {
    return sizeConfig.oneRowCount === 0
        ? false
        : (index + 1) % sizeConfig.oneRowCount === 0;
}

export function isInLastRow(index: number): boolean {
    return sizeConfig.oneRowCount === 0
        ? false
        : Math.ceil((index + 1) / sizeConfig.oneRowCount) === sizeConfig.rowCount;
}

// 颜色
export function getSubCoreColor(level: string, theme: Theme): string {
    COLOR[0] = theme.bgColorGrey;
    return COLOR[level] ?? COLOR[0];
}
