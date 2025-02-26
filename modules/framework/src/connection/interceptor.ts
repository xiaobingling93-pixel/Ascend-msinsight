/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import type { NotificationInterceptor, ResponseInterceptor } from './defs';
import {
    importActionHandler,
    type ImportActionResponse,
    parseMemorySuccessHandler,
} from './interceptorHandler';

const IMPORT = 'import/action';
const MEMORY_COMPLETED = 'parse/memoryCompleted';

export type ResponseType = ImportActionResponse;
export const INTERCEPTOR_HANDLERS: Record<string, ResponseInterceptor<ResponseType>> = {
    [IMPORT]: importActionHandler,
};

export const NOTIFICATION_INTERCEPTOR_HANDLERS: Record<string, NotificationInterceptor<any>> = {
    [MEMORY_COMPLETED]: parseMemorySuccessHandler,
};
