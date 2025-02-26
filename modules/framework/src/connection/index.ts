/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { ServerConnector } from 'ascend-connection';
import { INTERCEPTOR_HANDLERS, NOTIFICATION_INTERCEPTOR_HANDLERS } from './interceptor';
import { listenerMap } from './notification';
import { store } from '@/store';
import { request } from '@/centralServer/server';
import { GLOBAL_HOST } from '@/centralServer/websocket/defs';

type TargetWindow = Window;

const pluginEvents = ['remote/import', 'remote/reset', 'remote/remove', 'setTheme', 'wakeupPlugin', 'language', 'switchLanguage'];

const connector = new ServerConnector({
    getTargetWindow: (): TargetWindow[] => {
        const res: TargetWindow[] = [];
        document?.querySelectorAll('iframe')?.forEach((item) => item.contentWindow && res.push(item.contentWindow));
        return res;
    },
    getInterceptorHandlers: (command: string): (...args: any[]) => void => INTERCEPTOR_HANDLERS[command] as any,
    sendBefore: (origin: any): any => {
        const message = origin;
        if (pluginEvents.includes(message?.event as string)) {
            message.target = 'plugin';
        }
        if (NOTIFICATION_INTERCEPTOR_HANDLERS[message?.event] !== undefined) {
            NOTIFICATION_INTERCEPTOR_HANDLERS[message.event](message.body);
        }
        return message;
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
