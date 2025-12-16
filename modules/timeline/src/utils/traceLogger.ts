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
import { platform } from '../platforms';

// 打点工具，用于全局保存打点数据，以对象形式返回
interface TraceData {
    startTime: number;
    endTime: number;
    infos: TraceInfo;
}
interface TraceMessages {
    [x: string]: TraceData | undefined;
}

const traceMessages: TraceMessages = {};

// 不需要返回打点数据的事件
const noneResEvent = [
    'selectJsLane',
    'selectProcessTimeLane',
];

interface TraceInfo {
    [x: string]: unknown;
    action: string;
    responseTime?: number;
    units?: string[];
    selectRange?: number;
}

// 打点入口 traceStart
export function traceStart(key: string, infos: { [x: string]: unknown; action: string }): void {
    traceMessages[key] = {
        startTime: new Date().getTime(),
        endTime: 0,
        infos,
    };
}

// 打点出口 traceEnd
export function traceEnd(key: string): void {
    if (traceMessages[key] === undefined) {
        return;
    }
    const traceData = traceMessages[key] as TraceData;
    // 获取打点结束时间
    traceData.endTime = new Date().getTime();
    const responseTime = traceData.endTime - traceData.startTime;

    // 构建返回对象
    const traceInfo: TraceInfo = traceData.infos;
    traceInfo.responseTime = responseTime;
    const { action, ...others } = traceInfo;

    platform.trace(action, others);

    traceMessages[key] = undefined;
}

// 自封闭打点 traceSingle
export function traceSingle(action: string, unitNames: string[]): void {
    if (noneResEvent.includes(action)) {
        // 不需要返回打点数据的事件直接返回空对象
        platform.trace(action, {});
    } else {
        // 自封闭打点直接返回数据
        platform.trace(action, { units: unitNames });
    }
}
