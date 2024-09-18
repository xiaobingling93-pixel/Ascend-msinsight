/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { type ResponseInterceptor } from './defs';
import { importActionHandler, type ImportActionResponse } from './interceptorHandler';

const IMPORT = 'import/action';

export type ResponseType = ImportActionResponse;
export const INTERCEPTOR_HANDLERS: Record<string, ResponseInterceptor<ResponseType>> = {
    [IMPORT]: importActionHandler,
};
