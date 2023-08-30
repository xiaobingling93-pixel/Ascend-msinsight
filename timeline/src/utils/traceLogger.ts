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
    action: string;
    [x: string]: unknown;
    responseTime?: number;
    units?: string[];
    selectRange?: number;
}

// 打点入口 traceStart
export function traceStart(key: string, infos: { action: string; [x: string]: unknown }): void {
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
