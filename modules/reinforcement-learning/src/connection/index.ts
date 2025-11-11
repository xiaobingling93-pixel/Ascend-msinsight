/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import { ClientConnector } from '@insight/lib/connection';

export const connector = new ClientConnector({
    getTargetWindow: (): Window[] => [window.parent],
    module: 'RL',
});
