import { cloneDeep, throttle } from 'lodash';
import { Cache } from '../cache';
import { Logger } from '../../utils/Logger';
import { ValidSession } from '../../entity/session';
import { getRange, dataFunc } from '../utils';

/**
 * this cache won't process any datas,
 * just return the data every threshold ms
 */
// ThrottleCache<T extends DataKey>
export class ThrottleCache<T extends any> implements Cache {
    // data: SampleType[T];
    data: any;
    key: T;
    // params: DataParam<DataKey>;
    fetch: (session: ValidSession, params: any) => void;

    constructor(key: T, threshold: number = 500) {
        this.key = key;
        // this.data = emptyData[key];
        this.data = {};
        // params: DataParam<T>;
        this.fetch = (throttle(async (session: ValidSession, params: any) => {
            // const getRange = defaultTimeStrategy[this.key];
            // const dataFunc = dataFuncMap[this.key];
            try {
                this.data = (await dataFunc(session, getRange, params)) as any[];
            } catch (e) {
                // e: ErrorRes;
                const err = e as (any | undefined);
                err && Logger(`throttleCache/${this.key}`, `got error when fetching data: ${err.errorMessage}`, 'warn');
            }
        }, threshold));
    };

    getData = <T extends any>(session: ValidSession, params: any): Promise<any> => {
        this.fetch(session, params);
        return Promise.resolve({ [this.key as string]: cloneDeep(this.data) });
    };
};
