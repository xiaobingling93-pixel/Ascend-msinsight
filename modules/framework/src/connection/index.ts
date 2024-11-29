/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { ServerConnector } from 'ascend-connection';
import { INTERCEPTOR_HANDLERS } from './interceptor';
const pluginEvents = ['remote/import', 'remote/reset', 'remote/remove'];
type TargetWindow = Window;
export default new ServerConnector({
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
