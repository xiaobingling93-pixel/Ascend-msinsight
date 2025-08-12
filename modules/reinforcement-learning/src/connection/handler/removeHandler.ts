/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { NotificationHandler } from '@/connection/defs';
import { rootStore } from '@/stores';

// 删除单个项目
export const removeHandler: NotificationHandler = (): void => {
    rootStore.reset();
};

// 删除全部项目
export const resetHandler: NotificationHandler = (): void => {
    rootStore.reset();
};
