/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import { ClientConnector } from '@insight/lib/connection';

export default new ClientConnector({
    getTargetWindow: (): any[] => [window.parent],
    module: 'leaks',
});
