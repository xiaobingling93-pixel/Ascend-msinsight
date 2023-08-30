import { ValidSession } from '../../entity/session';
import { Cache, CacheFactory } from '../cache';

export class SwitchCacheInPhase implements Cache {
    phase: ValidSession['phase'];
    cache: Cache;
    factory: CacheFactory;
    switched: boolean = false;

    constructor(phase: ValidSession['phase'], factory: CacheFactory) {
        this.phase = phase;
        this.factory = factory;
        this.cache = factory.create();
    }

    getData<T extends any>(session: ValidSession, params: any): Promise<any> {
        if (!this.switched && session.phase === this.phase) {
            this.cache = this.factory.create();
            this.switched = true;
        }
        return this.cache.getData(session, params);
    }
};
