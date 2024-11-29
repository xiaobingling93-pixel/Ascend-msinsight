/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import connector from '@/connection';
import { store } from '@/store/rootStore';
import { request } from '@/centralServer/server';

type ListenerCallback = (res: MessageEvent) => void;

const listenerMap: Record<string, ListenerCallback> = {};
function registerEventListeners(): void {
    const session = store.sessionStore.activeSession;
    if (!session) {
        return;
    }

    connector.registerAwaitFetch(async (e) => {
        const { remote, args, module, voidResponse } = e.data;
        const result = await request(remote, module, args, voidResponse);
        return { dataSource: remote, body: result };
    });
    Object.entries(listenerMap).forEach(([event, callback]) => {
        connector.addListener(event, callback);
    });
}

export { registerEventListeners };
