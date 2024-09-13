/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { BasePlatform } from './BasePlatform';
import { safeJSONParse } from '@/utils';

export class IntellijPlatform extends BasePlatform {
    selectFolder(): Promise<string> {
        return new Promise((resolve) => {
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
        return new Promise((resolve) => {
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
