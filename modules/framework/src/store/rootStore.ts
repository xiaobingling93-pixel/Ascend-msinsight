/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
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
