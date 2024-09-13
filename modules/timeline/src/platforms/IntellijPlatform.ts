/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import type { IMessageSender } from '../connection/messageSender';
import { safeJSONParse } from 'ascend-utils';

export class IntellijPlatform implements IMessageSender {
    selectFolder(): Promise<string> {
        return new Promise(resolve => {
            this.sendMessage({
                request: JSON.stringify({
                    key: 'ascend.selectFolder',
                    data: {
                        method: 'ascend.selectFolder',
                        params: {},
                    },
                }),
                onSuccess: function (response: string) {
                    const result = safeJSONParse(response, { body: '' });
                    resolve(result.body);
                },
            });
        });
    }

    selectFile(): Promise<string> {
        return new Promise(resolve => {
            this.sendMessage({
                request: JSON.stringify({
                    key: 'ascend.selectFolder',
                    data: {
                        method: 'ascend.selectFile',
                        params: {},
                    },
                }),
                onSuccess: function (response: string) {
                    const result = safeJSONParse(response, { body: '' });
                    resolve(result.body);
                },
            });
        });
    }

    sendMessage = (ceq: any): void => {
        window.cefQuery(ceq);
    };
}
