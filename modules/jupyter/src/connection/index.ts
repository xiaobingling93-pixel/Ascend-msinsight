/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { ClientConnector } from 'lib/Connector';

export default new ClientConnector({
    getTargetWindow: (): any[] => [window.parent],
    module: 'jupyter',
});
