/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
export type ResponseInterceptor<T> = (event: MessageEvent, responseInterceptor: T) => void;
