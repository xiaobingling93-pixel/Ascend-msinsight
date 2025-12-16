/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
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
import {
    getLeaksGraphData, getMemoryDetailData, getFuncData, getBlockDetails, getEventDetails,
    type GraphParam, FuncParam, BlockParam, EventParam,
    ThreShold,
} from '../utils/RequestUtils';
import { message } from 'antd';
import { runInAction } from 'mobx';

export const getFuncNewData = async (session: any, startTimestamp?: number, endTimestamp?: number): Promise<void> => {
    try {
        const funcParam: FuncParam = { deviceId: session.deviceId, relativeTime: true, threadId: session.threadId, allowTrim: session.allowTrim };
        if (startTimestamp !== undefined && endTimestamp !== undefined) {
            funcParam.startTimestamp = startTimestamp;
            funcParam.endTimestamp = endTimestamp;
        }
        const funcDatas = await getFuncData(funcParam);
        runInAction(() => {
            session.funcData = funcDatas;
            session.funcOptions = [...new Set(funcDatas.traces.map(trace => trace.func))].map(func => ({ label: func, value: func }));
            const funcSet = new Set(session.searchFunc);
            session.searchFunc = funcSet.size ? session.funcOptions.filter((item: any) => funcSet.has(item.value)).map((i: any) => i.value) : [];
            if (startTimestamp !== undefined && endTimestamp !== undefined) {
                session.maxTime = endTimestamp;
                session.minTime = startTimestamp;
            }
            session.maxDepth = funcDatas.maxDepth;
        });
    } catch (error: any) {
        message.error(error.message);
    }
};
export const getBarNewData = async (session: any, startTimestamp?: number, endTimestamp?: number): Promise<void> => {
    try {
        const blockParam: GraphParam = { deviceId: session.deviceId, graph: 'blocks', relativeTime: true, eventType: session.eventType };
        const allocationParam: GraphParam = { deviceId: session.deviceId, graph: 'allocations', relativeTime: true, eventType: session.eventType };
        if (startTimestamp !== undefined && endTimestamp !== undefined) {
            blockParam.startTimestamp = startTimestamp;
            blockParam.endTimestamp = endTimestamp;
            allocationParam.startTimestamp = startTimestamp;
            allocationParam.endTimestamp = endTimestamp;
        }
        const [blockDatas, allocationDatas] = await Promise.all([
            getLeaksGraphData(blockParam),
            getLeaksGraphData(allocationParam),
        ]);
        runInAction(() => {
            session.blockData = blockDatas;
            session.allocationData = allocationDatas;
            if (startTimestamp === undefined && endTimestamp === undefined) {
                session.maxTime = Math.max(blockDatas.maxTimestamp, allocationDatas.maxTimestamp, session.funcData.maxTimestamp);
                session.minTime = Math.min(blockDatas.minTimestamp, allocationDatas.minTimestamp, session.funcData.minTimestamp);
                session.threadFlag = false;
            }
            if (startTimestamp !== undefined && endTimestamp !== undefined) {
                session.maxTime = endTimestamp;
                session.minTime = startTimestamp;
            }
        });
    } catch (error: any) {
        message.error(error.message);
    }
};
export const getNewDetailData = async (session: any): Promise<void> => {
    try {
        const memoryDatas = await getMemoryDetailData(session.deviceId, session.memoryStamp, session.eventType);
        runInAction(() => {
            session.memoryData = memoryDatas;
        });
    } catch (error: any) {
        message.error(error.message);
    }
};
const handleThreshold = (blockParam: any, session: any): void => {
    const { lazyUsedThreshold, delayedFreeThreshold, longIdleThreshold } = session;
    if (lazyUsedThreshold.valueT === null && lazyUsedThreshold.perT === null && delayedFreeThreshold.valueT === null &&
        delayedFreeThreshold.perT === null && longIdleThreshold.valueT === null && longIdleThreshold.perT === null
    ) return;
    const threshold: { [key: string]: ThreShold } = {};
    if (lazyUsedThreshold.valueT !== null || lazyUsedThreshold.perT !== null) {
        blockParam.lazyUsedThreshold = { perT: null, valueT: null };
        threshold.lazyUsedThreshold = lazyUsedThreshold;
    }
    if (delayedFreeThreshold.valueT !== null || delayedFreeThreshold.perT !== null) {
        blockParam.delayedFreeThreshold = { perT: null, valueT: null };
        threshold.delayedFreeThreshold = delayedFreeThreshold;
    }
    if (longIdleThreshold.valueT !== null || longIdleThreshold.perT !== null) {
        blockParam.longIdleThreshold = { perT: null, valueT: null };
        threshold.longIdleThreshold = longIdleThreshold;
    }
    Object.keys(threshold).forEach((key) => {
        if (threshold[key].valueT !== null) {
            blockParam[key].valueT = threshold[key].valueT;
        } else {
            blockParam[key].valueT = 0;
        }
        if (threshold[key].perT !== null) {
            blockParam[key].perT = threshold[key].perT;
        } else {
            blockParam[key].perT = 0;
        }
    });
};
export const getBlockTableData = async (session: any): Promise<void> => {
    try {
        const blockParam: BlockParam = {
            deviceId: session.deviceId,
            relativeTime: true,
            eventType: session.eventType,
            isTable: true,
            startTimestamp: session.minTime,
            endTimestamp: session.maxTime,
            currentPage: session.blocksCurrentPage,
            pageSize: session.blocksPageSize,
            desc: session.blocksOrder,
        };
        if (session.blocksOrderBy) {
            blockParam.orderBy = session.blocksOrderBy;
        }
        if (Object.keys(session.blocksFilters).length > 0) {
            blockParam.filters = session.blocksFilters;
        }
        if (Object.keys(session.blocksRangeFilters).length > 0) {
            blockParam.rangeFilters = session.blocksRangeFilters;
        }
        if (session.onlyInefficient) {
            blockParam.onlyInefficient = true;
        }
        handleThreshold(blockParam, session);
        const blockTableData = await getBlockDetails(blockParam);
        runInAction(() => {
            session.blocksTableData = blockTableData.blocks;
            session.blocksTableHeader = blockTableData.headers;
            session.blocksTotal = blockTableData.total;
        });
    } catch (error: any) {
        message.error(error.message);
    }
};
export const getEventTableData = async (session: any): Promise<void> => {
    try {
        const eventParam: EventParam = {
            deviceId: session.deviceId,
            relativeTime: true,
            startTimestamp: session.minTime,
            endTimestamp: session.maxTime,
            currentPage: session.eventsCurrentPage,
            pageSize: session.eventsPageSize,
            desc: session.eventsOrder,
        };
        if (session.eventsOrderBy) {
            eventParam.orderBy = session.eventsOrderBy;
        }
        if (Object.keys(session.eventsFilters).length > 0) {
            eventParam.filters = session.eventsFilters;
        }
        if (Object.keys(session.eventsRangeFilters).length > 0) {
            eventParam.rangeFilters = session.eventsRangeFilters;
        }
        const eventTableData = await getEventDetails(eventParam);
        runInAction(() => {
            session.eventsTableData = eventTableData.events;
            session.eventsTableHeader = eventTableData.headers;
            session.eventsTotal = eventTableData.total;
        });
    } catch (error: any) {
        message.error(error.message);
    }
};
