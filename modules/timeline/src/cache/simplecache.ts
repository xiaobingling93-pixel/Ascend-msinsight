import { ThreadTraceRequest, ThreadTrace, CounterData, CounterMetaData, CounterRequest } from '../entity/data';
import { binarySearchFirstBig, binarySearchLastSmall } from './strategies/utils';

type Method = 'unit/threadTraces' | 'unit/counter'; // store methodKey
type Handler = (params: Record<string, unknown>, metaData?: unknown) => Promise<ThreadTrace[][] | number[][]>;
const processorMap = new Map<Method, Function>();
const handlerMap = new Map<Method, Handler>();
handlerMap.set('unit/threadTraces', requestThreadTraces);
handlerMap.set('unit/counter', requestCounterData);
processorMap.set('unit/threadTraces', sliceArr);
processorMap.set('unit/counter', counterArr);

export class SimpleCache {
    data: Map<string, any>;

    constructor() {
        this.data = new Map();
        this.data.set('unit/threadTraces', new Map<string, Record<string, unknown>>());
        this.data.set('unit/counter', new Map<string, Record<string, unknown>>());
    }

    async tryFetchFromCache(method: Method, requestKey: string, params: Record<string, unknown>, metaData?: unknown): Promise<Record<string, unknown> | undefined> {
        if (this.data.get(method).get(requestKey) === undefined) {
            try {
                const result = await handlerMap.get(method)?.(params, metaData);
                if (result !== undefined && result.length !== 0) {
                    this.data.get(method).set(requestKey, result);
                }
            } catch (e) {
                console.warn('Failed to try fetch from cache', method, requestKey, e);
                return undefined;
            }
        }
        if (this.data.get(method).get(requestKey) === undefined) {
            console.warn('Failed to try fetch from cache', method, requestKey);
            return undefined;
        }
        return {
            data: processorMap.get(method)?.(params, this.data, requestKey),
        };
    }

    clear(): void {
        this.data.forEach((key) => {
            this.data.get(key).clear();
        });
    }
}

async function requestThreadTraces(requestParam: Record<string, unknown>): Promise<ThreadTrace[][]> {
    try {
        const { store } = require('../store');
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        const param: Record<string, unknown> = Object.assign({}, requestParam);
        param.startTime = 0;
        param.endTime = session?.endTimeAll ?? 0;
        const request = await window.request(requestParam.dataSource as DataSource, { command: 'unit/threadTraces', params: param });
        return request.data as ThreadTrace[][];
    } catch (e) {
        console.warn('request threadTrace info failed', e);
        return [];
    }
}

async function requestCounterData(requestParam: Record<string, unknown>, metadata: unknown): Promise<number[][]> {
    try {
        const { store } = require('../store');
        const { sessionStore } = store;
        const session = sessionStore.activeSession;
        const param: Record<string, unknown> = Object.assign({}, requestParam);
        param.startTime = 0;
        param.endTime = session?.endTimeAll ?? 0;
        const request = await window.request(requestParam.dataSource as DataSource, { command: 'unit/counter', params: param });
        const acc = (request.data as CounterData[]).reduce((acc: CounterData[], cur) => {
            const existItem = acc.find(item => item.timestamp === cur.timestamp);
            if (existItem !== undefined) {
                (metadata as CounterMetaData).dataType.forEach(type => {
                    if (cur.value[type] > existItem.value[type]) {
                        existItem.value[type] = cur.value[type];
                    }
                });
            } else {
                acc.push(cur);
            }
            return acc;
        }, []);
        return acc.map((item): number[] => {
            const res = [];
            res.push(item.timestamp);
            (metadata as CounterMetaData).dataType.forEach(type => {
                res.push(item.value[type]);
            });
            return res;
        });
    } catch (e) {
        console.warn('request threadTrace info failed', e);
        return [];
    }
}

function sliceArr(params: Record<string, unknown>, data: Map<string, any>, paramsKey: string): any {
    const requestParams = params as ThreadTraceRequest;
    const start = requestParams.startTime;
    const end = requestParams.endTime;

    const result = data.get('unit/threadTraces').get(paramsKey);
    return result.map((subArr: unknown[]) => {
        if (subArr.length === 0) {
            return [];
        }
        const startIndex = binarySearchLastSmall(subArr, (data: any) => data.startTime, start);
        const endIndex = binarySearchFirstBig(subArr, (data: any) => data.startTime, end);
        return subArr.slice(startIndex, endIndex + 1);
    });
}

function counterArr(params: Record<string, unknown>, data: Map<string, unknown>, paramsKey: string): unknown {
    const requestParams = params as CounterRequest;
    const start = requestParams.startTime;
    const end = requestParams.endTime;

    const result = (data.get('unit/counter') as Map<string, number[][]>).get(paramsKey);
    if (result === undefined) {
        return [[]];
    }
    const startIndex = binarySearchLastSmall(result, (data: number[]) => data[0], start);
    const endIndex = binarySearchFirstBig(result, (data: number[]) => data[0], end);
    return result.slice(startIndex === 0 ? startIndex : startIndex - 1, endIndex + 2);
}
