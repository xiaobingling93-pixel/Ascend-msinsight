import { ThreadTraceRequest, ThreadTrace } from '../entity/data';
import { binarySearchFirstBig, binarySearchLastSmall } from './strategies/utils';
import { store } from '../store';

const stackStatusKey = 'chart/stackStatus';
type Method = 'unit/threadTraces'; // store methodKey
type Handler = (params: Record<string, unknown>) => Promise<any>;
const handlerMap = new Map<Method, Handler>();
handlerMap.set('unit/threadTraces', requestThreadTraces);

export class SimpleCache {
    data: Map<string, any>;

    constructor() {
        this.data = new Map();
        this.data.set(stackStatusKey, new Map<string, Record<string, unknown>>());
    }

    async tryFetchFromCache(method: Method, requestKey: string, params: Record<string, unknown>): Promise<Record<string, unknown> | undefined> {
        if (this.data.get(stackStatusKey).get(requestKey) === undefined) {
            try {
                const result = await handlerMap.get(method)?.(params);
                if (result.length !== 0) {
                    this.data.get(stackStatusKey).set(requestKey, result);
                }
            } catch (e) {
                console.warn('Failed to try fetch from cache', method, requestKey, e);
                return undefined;
            }
        }
        if (this.data.get(stackStatusKey).get(requestKey) === undefined) {
            console.warn('Failed to try fetch from cache', method, requestKey);
            return undefined;
        }
        return {
            data: sliceArr(params, this.data, requestKey),
        };
    }

    clear(): void {
        this.data.get(stackStatusKey).clear();
    }
}

async function requestThreadTraces(requestParam: Record<string, unknown>): Promise<ThreadTrace[][]> {
    try {
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        const param: Record<string, unknown> = Object.assign({}, requestParam);
        param.startTime = 0;
        param.endTime = session?.endTimeAll ?? 0;
        const request = await window.request({ command: 'unit/threadTraces', params: param });
        return request.data as ThreadTrace[][];
    } catch (e) {
        console.warn('request threadTrace info failed', e);
        return [];
    }
}

function sliceArr(params: Record<string, unknown>, data: Map<string, any>, paramsKey: string): any {
    const requestParams = params as ThreadTraceRequest;
    const start = requestParams.startTime;
    const end = requestParams.endTime;

    const result = data.get(stackStatusKey).get(paramsKey);
    return result.map((subArr: unknown[]) => {
        if (subArr.length === 0) {
            return [];
        }
        const startIndex = binarySearchLastSmall(subArr, (data: any) => data.startTime, start);
        const endIndex = binarySearchFirstBig(subArr, (data: any) => data.startTime, end);
        return subArr.slice(startIndex, endIndex + 1);
    });
}
