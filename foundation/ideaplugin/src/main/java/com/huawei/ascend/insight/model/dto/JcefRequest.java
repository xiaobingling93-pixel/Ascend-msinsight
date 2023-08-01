/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 */

package com.huawei.ascend.insight.model.dto;

import com.alibaba.fastjson.JSONObject;

import lombok.AllArgsConstructor;
import lombok.Data;
import org.jetbrains.annotations.NotNull;

/**
 * 前端发送的请求的数据格式定义
 *
 * @since 2023/5/29
 */
@Data
@AllArgsConstructor
public class JcefRequest {
    @NotNull
    private String key;

    @NotNull
    private JcefRequest.Data data;

    /**
     * data数据格式
     */
    @lombok.Data
    @AllArgsConstructor
    public static class Data {
        @NotNull
        private JSONObject params;

        @NotNull
        private String method;
    }
}
