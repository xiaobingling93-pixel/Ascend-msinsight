/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { notZero } from '../Common';
import { Session } from '../../entity/session';
import { ArrangementItem, ConnectionsItem, ParallelismType } from '../../utils/interface';

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

export const getDyeingColor = (session: Session, index: number, dyeingMode: string, step: number): string => {
    const performanceValue = session.performanceDataMap.get(index);
    if (performanceValue === undefined || !Object.keys(performanceValue).includes(dyeingMode)) {
        return 'rgba(0,0,0,0)';
    }
    const ratio = (performanceValue[dyeingMode] - session.rankDyeingData[dyeingMode].min) / notZero(session.rankDyeingData[dyeingMode].min);
    const level = Math.ceil(ratio / step);

    return level < 5 ? `rgba(36,171,54,${(5 - level) * 0.2})` : `rgba(227,32,32,${(level - 4) * 0.2})`;
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

        if (!groupMap.has(indexKey)) {
            groupMap.set(indexKey, []);
        }
        groupMap.get(indexKey)?.push({ index, position, attribute });
    });

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
