/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { notZero, COLOR } from '../Common';
import { Session } from '../../entity/session';
import { ArrangementItem, ConnectionsItem, ParallelismType } from '../../utils/interface';
import { hexToRgb } from 'ascend-utils';

export interface communicatorContainerData {
    partitionModes: partitionMode[];
    defaultPPSize: number;
}

export interface partitionMode {
    mode: string;
    communicators: Communicator[];
}

export interface Communicator {
    value: string;
}

export interface RankDyeingData {
    [key: string]: { min: number; max: number };
}

interface DyeingColorParams {
    session: Session;
    index: number;
    dyeingMode: string;
    range: Array<number | null>;
}

/**
 * 计算每张卡的性能颜色
 * @param session
 * @param index 卡号
 * @param dyeingMode 性能指标
 * @param range 性能区间
 */
export const getDyeingColor = ({ session, index, dyeingMode, range }: DyeingColorParams): string => {
    const performanceDataItem = session.performanceDataMap.get(index);
    const [startVal, endVal] = range;
    const isRangeEmpty = startVal === null || endVal === null;

    if (isRangeEmpty || performanceDataItem === undefined) {
        return '';
    }

    let performanceValue = performanceDataItem[dyeingMode];
    if (!performanceValue) {
        performanceValue = performanceDataItem.commCompare[dyeingMode];
    }
    if (!performanceValue || performanceValue < startVal || performanceValue > endVal) {
        return '';
    }

    if (performanceValue === startVal && performanceValue === endVal) {
        return COLOR.BAND_1;
    }

    // 计算归一化比例
    const ratio = (performanceValue - startVal) / notZero(endVal - startVal);

    const COLOR_BANDS = [COLOR.BAND_3, COLOR.BAND_2, COLOR.BAND_1, COLOR.BAND_0];

    // 计算区间索引和局部比例
    const segmentIndex = Math.min(Math.floor(ratio * 3), 2); // 限制最大索引为2
    const startColor = COLOR_BANDS[segmentIndex];
    const endColor = COLOR_BANDS[segmentIndex + 1];
    const localRatio = (ratio - (segmentIndex / 3)) * 3;

    return interpolateColor(startColor, endColor, localRatio);
};

// 计算插值的方法
const interpolateColor = (minColor: string, maxColor: string, ratio: number): string => {
    const minColorRgb = hexToRgb(minColor);
    const maxColorRgb = hexToRgb(maxColor);

    if (minColorRgb === null || maxColorRgb === null) {
        return '';
    }

    const [r1, g1, b1] = minColorRgb;
    const [r2, g2, b2] = maxColorRgb;

    const r = Math.round(r1 + ((r2 - r1) * ratio));
    const g = Math.round(g1 + ((g2 - g1) * ratio));
    const b = Math.round(b1 + ((b2 - b1) * ratio));

    return `rgb(${r}, ${g}, ${b})`;
};

export interface FrameGroupItem {
    type: ParallelismType;
    list: Array<Pick<ArrangementItem, 'index' | 'position' | 'attribute'>>;
}

// 筛选 DP/EP 框的条件：DP/EP 值相同
export const findDPOrEPFrame = (arrangements: ArrangementItem[], type: ParallelismType): FrameGroupItem[] => {
    const groupList: FrameGroupItem[] = [];
    const groupMap = new Map<number, FrameGroupItem['list']>();

    arrangements.forEach((arrangement) => {
        const { index, position, attribute } = arrangement;
        const indexKey = attribute[`${type}Index`];

        if (indexKey === undefined) {
            return;
        }

        if (!groupMap.has(indexKey)) {
            groupMap.set(indexKey, []);
        }
        groupMap.get(indexKey)?.push({ index, position, attribute });
    });

    // DPSize 或 EPSize 配置为 1 时，不显示框
    if (groupMap.size < 2) {
        return [];
    }

    groupMap.forEach((list) => {
        groupList.push({
            type,
            list,
        });
    });
    return groupList;
};

// 筛选CP框的条件：CP值相同 且 DP值不同
export const findCPFrame = (arrangements: ArrangementItem[]): FrameGroupItem[] => {
    const groups: Map<string, ArrangementItem[]> = new Map();

    // 根据 dpIndex-cpIndex 分组
    arrangements.forEach(item => {
        const { cpIndex, dpIndex } = item.attribute;
        const key = `${dpIndex}-${cpIndex}`;

        if (!groups.has(key)) {
            groups.set(key, []);
        }

        groups.get(key)?.push(item);
    });

    // 如果最后一个组的 key 中 cpIndex === 0，则表示 cpIndex 全部为0，即不存在CP组
    const lastGroup = Array.from(groups).pop();
    if (lastGroup?.[0].split('-')[1] === '0') {
        return [];
    }

    const result: FrameGroupItem[] = Array.from(groups.entries()).map(([_, items]) => {
        return { type: 'cp', list: items.map(item => ({ index: item.index, position: item.position, attribute: item.attribute })) };
    }).filter((group): group is FrameGroupItem => group !== null);

    return result;
};

// 筛选 TP、PP 框
export const findTPOrPPFrame = (arrangements: ArrangementItem[], type: ParallelismType, connections: ConnectionsItem[]): FrameGroupItem[] => {
    return connections.filter(connection => connection.type === type).map(item => {
        return {
            type,
            list: item.list.map(it => ({
                index: it,
                position: arrangements[it].position,
                attribute: arrangements[it].attribute,
            })),
        };
    });
};

// 根据 types 获取相关通信域(框)的数据
export const groupFrames = (arrangements: ArrangementItem[], types: ParallelismType[], connections?: ConnectionsItem[]): FrameGroupItem[] => {
    const groupedResults: FrameGroupItem[] = [];

    types.forEach((type) => {
        switch (type) {
            case 'dp':
            case 'ep':
                groupedResults.push(...findDPOrEPFrame(arrangements, type));
                break;
            case 'cp':
                groupedResults.push(...findCPFrame(arrangements));
                break;
            case 'tp':
            case 'pp':
                if (connections) {
                    groupedResults.push(...findTPOrPPFrame(arrangements, type, connections));
                }
                break;
            default:
                break;
        }
    });

    return groupedResults;
};
