/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import {
    getLeaksGraphData, getMemoryDetailData, getFuncData, getBlockDetails, getEventDetails,
    type GraphParam, FuncParam, BlockParam, EventParam,
} from '../utils/RequestUtils';
import { message } from 'antd';
import { runInAction } from 'mobx';

export const getFuncNewData = async (session: any, startTimestamp?: number, endTimestamp?: number): Promise<void> => {
    try {
        const funcParam: FuncParam = { deviceId: session.deviceId, relativeTime: true, threadId: session.threadId };
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
            session.maxTime = endTimestamp;
            session.minTime = startTimestamp;
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
