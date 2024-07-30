/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { type ResponseInterceptor } from './defs';
import { useLoading } from '@/hooks/useLoading';

export const importActionHandler: ResponseInterceptor = (data): void => {
    // 关闭弹框
    useLoading().close();
};
