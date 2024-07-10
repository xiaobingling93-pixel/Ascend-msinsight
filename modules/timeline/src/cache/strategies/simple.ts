import { ElementType, KeysMatching } from '../../entity/common';
import { ValidSession } from '../../entity/session';
import { Logger } from '../../utils/Logger';
import { getRange, dataFunc } from '../utils';
import { Cache, CacheFactory } from '../cache';
import { binarySearchFirstBig, binarySearchLastSmall } from './utils';

export interface KeyedCacheFactory<K extends any> extends CacheFactory {
    readonly key: K;
    create: () => Cache;
}

// used for 1-dimensional time based data
export class SimpleOfflineTimeSeriesCache<K extends KeysMatching<any, unknown[]>> implements Cache {
    private data?: any = undefined;
    private readonly key: K;
    private readonly useRelative: boolean;
    private readonly getTime: (e: ElementType<any>) => number;

    static newFactory<K extends KeysMatching<any, unknown[]>>(
        key: K,
        time: KeysMatching<any, number> | ((e: ElementType<any>) => number),
        useRelative: boolean = true,
    ): KeyedCacheFactory<K> {
        return {
            key,
            create: () => new SimpleOfflineTimeSeriesCache(key, time, useRelative),
        };
    }

    constructor(key: K, time: KeysMatching<any, number> | ((e: ElementType<any>) => number),
        useRelative: boolean = true) {
        this.key = key;
        // wedge data[time]
        this.getTime = typeof time === 'function' ? time : (data): number => data as unknown as number;
        this.useRelative = useRelative;
    }

    async getData<T extends any>(session: ValidSession, params: any): Promise<any> {
        if (this.data === undefined) {
            try {
                this.data = [];
            } catch (e) {
                // wedge e: ErrorRes
                const err = e as (any | undefined);
                err && Logger(`simpleCache/${this.key}`, `got error when fetching data: ${err.errorMessage}`, 'warn');
            }
        }
        if (this.data === undefined) {
            throw new Error('fetch data failed in SimpleOfflineTimeSeriesCache');
        }
        const [start, end] = getRange(session);
        const startIndex = binarySearchLastSmall(this.data as any[], this.getTime, start);
        const endIndex = binarySearchFirstBig(this.data as any[], this.getTime, end);
        return {
            [this.key]: this.data.slice(startIndex, endIndex + 1),
        };
    }
}

// used for grouped 1-dimensional time based data, such as multi-line status data
export class SimpleOfflineGroupedTimeSeriesCache<K extends KeysMatching<any, object[][]>> implements Cache {
    private data?: any = undefined;
    private readonly key: K;
    private readonly useRelative: boolean;
    private readonly getTime: (e: ElementType<ElementType<any>>) => number;

    static newFactory<K extends KeysMatching<any, object[][]>>(
        key: K,
        timeField: KeysMatching<ElementType<ElementType<any>>, number>,
        useRelative: boolean = true,
    ): KeyedCacheFactory<K> {
        return {
            key,
            create: () => new SimpleOfflineGroupedTimeSeriesCache(key, timeField, useRelative),
        };
    }

    constructor(key: K, timeField: KeysMatching<ElementType<ElementType<any>>, number>, useRelative: boolean = true) {
        this.key = key;
        this.useRelative = useRelative;
        this.getTime = (data): number => data[timeField] as unknown as number;
    }

    async getData<T extends any>(session: ValidSession, params: any): Promise<any> {
        if (this.data === undefined) {
            this.data = [];
        }
        if (this.data === undefined) {
            throw new Error('fetch data failed in SimpleOfflineGroupedTimeSeriesCache');
        }
        const [start, end] = getRange(session);
        return {
            [this.key]: this.data.map((it: any[]) => {
                const startIndex = binarySearchLastSmall(it as Array<ElementType<ElementType<any>>>, this.getTime, start);
                const endIndex = binarySearchFirstBig(it as Array<ElementType<ElementType<any>>>, this.getTime, end);
                return it.slice(startIndex, endIndex + 1);
            }),
        };
    }
}
