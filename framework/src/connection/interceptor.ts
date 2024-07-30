/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { type ResponseInterceptor } from './defs';
import { importActionHandler } from './interceptorHandler';

const IMPORT = 'import/action';

export const INTERCEPTOR_HANDLERS: Record<string, ResponseInterceptor> = {
    [IMPORT]: importActionHandler,
};
