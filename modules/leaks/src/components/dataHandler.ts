/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { getLeaksGraphData, getMemoryDetailData, getFuncData, type GraphParam, FuncParam } from '../utils/RequestUtils';
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
