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
import type { ThreadTrace, CounterData, CounterMetaData, CounterRequest, ProcessData, ProcessRequest } from '../entity/data';
import { binarySearchFirstBig, binarySearchLastSmall } from './strategies/utils';
import { customConsole as console } from '@insight/lib/utils';

type Method = 'unit/threadTraces' | 'unit/counter' | 'unit/threadTracesSummary'; // store methodKey
type Handler = (params: Record<string, unknown>, metaData?: unknown) => Promise<ThreadTrace[][] | number[][] | ProcessData[] | undefined>;
const processorMap = new Map<Method, (params: Record<string, unknown>, data: Map<string, unknown>, paramsKey: string) => unknown>();
const handlerMap = new Map<Method, Handler>();
handlerMap.set('unit/counter', requestCounterData);
handlerMap.set('unit/threadTracesSummary', requestProcessData);

processorMap.set('unit/threadTracesSummary', processArr);
processorMap.set('unit/counter', counterArr);

export class SimpleCache {
    data: Map<string, any>;
    timePerPx: number = -1;

    constructor() {
        this.data = new Map();
        this.data.set('unit/counter', new Map<string, Record<string, unknown>>());
        this.data.set('unit/threadTracesSummary', new Map<string, Record<string, unknown>>());
    }

    tryFetchFromCache = async (
        method: Method, requestKey: string, params: Record<string, unknown> & { timePerPx: number }, metaData?: unknown,
    ): Promise<Record<string, unknown> | undefined> => {
        if (this.data.get(method).get(requestKey) === undefined) {
            try {
                const result = await handlerMap.get(method)?.(params, metaData);
                if (result !== undefined && result.length === 0) {
                    this.data.get(method).set(requestKey, undefined);
                }

                if (result !== undefined && result.length >= 1) {
                    this.timePerPx = params.timePerPx;
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
    };

    clear(): void {
        this.data.forEach((value) => {
            value?.clear();
        });
    }
}

async function requestProcessData(requestParam: Record<string, unknown>): Promise<ProcessData[] | undefined> {
    try {
        const param: Record<string, unknown> = Object.assign({}, requestParam);
        const request = await window.request(requestParam.dataSource as DataSource, { command: 'unit/threadTracesSummary', params: param });
        return request.data as ProcessData[];
    } catch (e) {
        console.warn('request threadTrace info failed', e);
        return undefined;
    }
}

async function requestCounterData(requestParam: Record<string, unknown>, metadata: unknown): Promise<number[][]> {
    try {
        const param: Record<string, unknown> = Object.assign({}, requestParam);
        const request = await window.request(requestParam.dataSource as DataSource, { command: 'unit/counter', params: param });
        const acc = (request.data as CounterData[]).reduce((dataList: CounterData[], cur) => {
            const existItem = dataList.find(item => item.timestamp === cur.timestamp);
            if (existItem !== undefined) {
                (metadata as CounterMetaData).dataType.forEach(type => {
                    if (cur.value[type] > existItem.value[type]) {
                        existItem.value[type] = cur.value[type];
                    }
                });
            } else {
                dataList.push(cur);
            }
            return dataList;
        }, []);
        return acc.map((item): number[] => {
            const res = [];
            res.push(item.timestamp);
            (metadata as CounterMetaData).dataType.forEach(type => {
                const val = Number(item.value[type]);
                res.push(isNaN(val) ? 0 : val);
            });
            return res;
        });
    } catch (e) {
        console.warn('request threadTrace info failed', e);
        return [];
    }
}

function processArr(params: Record<string, unknown>, data: Map<string, unknown>, paramsKey: string): unknown {
    const requestParams = params as unknown as ProcessRequest;
    const start = requestParams.startTime;
    const end = requestParams.endTime;

    const result = (data.get('unit/threadTracesSummary') as Map<string, number[][]>).get(paramsKey);
    if (result === undefined) {
        return [[]];
    }
    const startIndex = binarySearchLastSmall(result, (arr: number[]) => arr[0], start);
    const endIndex = binarySearchFirstBig(result, (arr: number[]) => arr[0], end);
    return result.slice(startIndex === 0 ? startIndex : startIndex - 1, endIndex + 2);
}

function counterArr(params: Record<string, unknown>, data: Map<string, unknown>, paramsKey: string): unknown {
    const requestParams = params as unknown as CounterRequest;
    const start = requestParams.startTime;
    const end = requestParams.endTime;

    const result = (data.get('unit/counter') as Map<string, number[][]>).get(paramsKey);
    if (result === undefined) {
        return [[]];
    }
    const startIndex = binarySearchLastSmall(result, (arr: number[]) => arr[0], start);
    const endIndex = binarySearchFirstBig(result, (arr: number[]) => arr[0], end);
    return result.slice(startIndex === 0 ? startIndex : startIndex - 1, endIndex + 2);
}
