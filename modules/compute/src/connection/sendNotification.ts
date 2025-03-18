/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/
import connector from './index';

// 跳转到Source页签
export const swtich2Source = (data: Record<string, any>): void => {
    connector.send({
        event: 'switchModule',
        body: {
            switchTo: 'source',
            toModuleEvent: 'showCacheInstructions',
            params: data,
            broadcast: true,
        },
    });
};
