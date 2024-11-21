/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 */

package com.huawei.ascend.insight.utils;

import com.alibaba.fastjson.JSONException;
import com.alibaba.fastjson.JSONObject;

import org.jetbrains.annotations.Nullable;

/**
 * JsonUtil
 *
 * @since 2022-07-25
 */
public class JsonUtil {
    private JsonUtil() {
    }

    /**
     * 解析json为指定对象
     *
     * @param text json字符串
     * @param clazz 目标类型
     * @return 目标对象
     */
    @Nullable
    public static <T> T parseObject(String text, Class<T> clazz) {
        try {
            return JSONObject.parseObject(text, clazz);
        } catch (JSONException e) {
            return null;
        }
    }
}
