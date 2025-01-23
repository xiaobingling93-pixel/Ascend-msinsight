/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { ServerConnector } from 'ascend-connection';
import { INTERCEPTOR_HANDLERS } from './interceptor';
import { listenerMap } from './notification';
import { store } from '@/store';
import { request } from '@/centralServer/server';
import { GLOBAL_HOST } from '@/centralServer/websocket/defs';

type TargetWindow = Window;

const pluginEvents = ['remote/import', 'remote/reset', 'remote/remove', 'setTheme', 'wakeupPlugin'];

const connector = new ServerConnector({
    getTargetWindow: (): TargetWindow[] => {
        const res: TargetWindow[] = [];
        document?.querySelectorAll('iframe')?.forEach((item) => item.contentWindow && res.push(item.contentWindow));
        return res;
    },
    getInterceptorHandlers: (command: string): (...args: any[]) => void => INTERCEPTOR_HANDLERS[command] as any,
    sendBefore: (originBody: any): any => {
        const body = originBody;
        if (pluginEvents.includes(body?.event as string)) {
            body.target = 'plugin';
        }
        return body;
    },
});

export default connector;

export function registerEventListeners(): void {
    // 接收模块消息
    Object.entries(listenerMap).forEach(([event, callback]) => {
        connector.addListener(event, callback);
    });

    // 转发ws消息到模块
    connector.registerAwaitFetch(async (e) => {
        const currentRemote = store.sessionStore.activeSession?.activeDataSource ?? GLOBAL_HOST;
        const { remote = currentRemote, args, module, voidResponse } = e.data;
        const result = await request(remote, module, args, voidResponse);
        return { dataSource: remote, body: result };
    });
}
