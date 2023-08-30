import { InsightStore } from './insight';
import { SessionStore } from './session';

export class RootStore {
    insightStore: InsightStore;
    sessionStore: SessionStore;

    constructor() {
        this.insightStore = new InsightStore();
        this.sessionStore = new SessionStore();
    }

    resetStore = (): void => {
        this.insightStore = new InsightStore();
        this.sessionStore = new SessionStore();
    };
}

export const store = new RootStore();
