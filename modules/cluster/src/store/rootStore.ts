/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
// Cluster Module
import { SessionStore } from './session';

export class RootStore {
    sessionStore: SessionStore;

    constructor() {
        this.sessionStore = new SessionStore();
    }

    resetStore = (): void => {
        this.sessionStore = new SessionStore();
    };
}

export const store = new RootStore();
