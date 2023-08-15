/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.ascend.insight;

import com.alibaba.fastjson.JSONObject;

import org.cef.callback.CefQueryCallback;
import org.jetbrains.annotations.NotNull;

/**
 * CefQueryHandler
 *
 * @since 2022-07-18
 */
public interface CefQueryHandler {
    /**
     * 处理前端的请求
     *
     * @param method 请求的方法
     * @param params 请求的参数
     * @param callback 请求的回调函数
     */
    void onQuery(@NotNull String method, @NotNull JSONObject params, @NotNull CefQueryCallback callback);
}
