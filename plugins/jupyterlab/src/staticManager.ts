/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { ServerConnection } from '@jupyterlab/services';
/**
 * The namespace for MindStudioManager statics.
 */

// The options used to initialize a mindstudio manager.
export interface IOptions {
    /**
     * The server settings used by the manager.
     */
    serverSettings?: ServerConnection.ISettings;
}
