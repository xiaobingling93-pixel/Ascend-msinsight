/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

import { makeAutoObservable } from 'mobx';
import { getRootUnit, isStreamUnit } from '../utils';
import { preOrderFlatten } from './common';
import { ProcessMetaData, ThreadMetaData } from './data';
import { InsightUnit } from './insight';
import { Session } from './session';

export type ThreadGroup = {
    cardId: string;
    processId: string;
    threadIds: string[];
};

/**
 * 存储合并的线程组数据
 * 它的生命周期应与 Project(DataSource) 的生命周期一致；即 Project 切换时，它要触发 clear
 * 在 Project 切换时，它要么清空 session.units 数据，要么完全刷新 session.units 数据
 * 因此当 session.units 数据被清空或完全刷新时，本对象也要清空
 */
export class MergedThreadData {
    private _mergedThreadGroupList: ThreadGroup[];

    constructor() {
        makeAutoObservable(this);
        this._mergedThreadGroupList = [];
    }

    get mergedThreadGroupList(): ThreadGroup[] { return this._mergedThreadGroupList; }

    set mergedThreadGroupList(value: ThreadGroup[]) {
        this._mergedThreadGroupList = value;
    }

    concatThreadGroupList(other: ThreadGroup[]): void {
        this._mergedThreadGroupList = this._mergedThreadGroupList.concat(other);
    }

    addMergedGroup(addedGroup: ThreadGroup): void {
        // 找到脏数据并移除
        const dirtyIndexes = this._mergedThreadGroupList.reduce<number[]>((result, group, idx) => {
            const dirty = group.cardId === addedGroup.cardId &&
                group.processId === addedGroup.processId &&
                group.threadIds.some(threadId => addedGroup.threadIds.includes(threadId));
            if (dirty) {
                result.push(idx);
            }
            return result;
        }, []);
        dirtyIndexes.forEach(idx => this._mergedThreadGroupList.splice(idx, 1));
        // 添加新数据
        this._mergedThreadGroupList.push(addedGroup);
    }

    getNeedMergeThreadLists(session: Session): InsightUnit[][] {
        const mergedThreadGroupList: ThreadGroup[] = this._mergedThreadGroupList;
        if (mergedThreadGroupList.length === 0) { return []; }
        const flattenAscendHardwareUnits = preOrderFlatten(getRootUnit(session.units), 0, {
            exclude: (node) => node.isMerged ||
                ((node.metadata as ProcessMetaData).processName !== undefined && !(node.metadata as ProcessMetaData).processName.startsWith('Ascend Hardware')),
        });
        const streamUnits = flattenAscendHardwareUnits.filter(isStreamUnit);
        const getUnifyKey = ({ cardId, processId, threadId }: Pick<ThreadMetaData, 'cardId' | 'processId' | 'threadId'>): string => {
            return `${cardId}:${processId}:${threadId}`;
        };
        const map = new Map(streamUnits.map(item => [getUnifyKey(item.metadata as ThreadMetaData), item]));
        const invalidGroupIndexes: number[] = [];
        const needMergeThreadLists = mergedThreadGroupList.map((threadGroup, idx): InsightUnit[] => {
            const threads = threadGroup.threadIds.map((id) => {
                const key = getUnifyKey({
                    cardId: threadGroup.cardId,
                    processId: threadGroup.processId,
                    threadId: id,
                });
                console.assert(map.has(key), `${key} is not found when merge units`);
                return map.get(key);
            }).filter(item => item !== undefined);
            if (threads.length === 0) {
                invalidGroupIndexes.push(idx);
            }
            return threads;
        });
        // 清除无效数据
        invalidGroupIndexes.forEach(idx => this._mergedThreadGroupList.splice(idx, 1));
        return needMergeThreadLists;
    }

    clear(): void {
        this._mergedThreadGroupList = [];
    }
}
