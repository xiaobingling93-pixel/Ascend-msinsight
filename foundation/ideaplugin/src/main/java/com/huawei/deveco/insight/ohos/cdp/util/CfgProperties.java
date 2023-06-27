/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.deveco.insight.ohos.cdp.util;

import com.intellij.CommonBundle;
import com.intellij.reference.SoftReference;

import org.jetbrains.annotations.NonNls;
import org.jetbrains.annotations.Nullable;
import org.jetbrains.annotations.PropertyKey;

import java.lang.ref.Reference;
import java.util.ResourceBundle;

/**
 * cfg properties
 *
 * @since 2022-08-15
 */
public class CfgProperties {
    private static final String TRUE = "true";

    private static final String FALSE = "false";

    @NonNls
    private static final String BUNDLE = "Cfg";

    private static Reference<ResourceBundle> ourBundle;

    /**
     * message
     *
     * @param key bundle key
     * @param params params
     * @return value
     */
    @Nullable
    private static String message(@PropertyKey(resourceBundle = BUNDLE) String key, Object... params) {
        return CommonBundle.messageOrNull(getBundle(), key, params);
    }

    private static ResourceBundle getBundle() {
        ResourceBundle dereference = SoftReference.dereference(ourBundle);
        if (dereference == null) {
            dereference = ResourceBundle.getBundle(BUNDLE);
            ourBundle = new SoftReference<>(dereference);
        }
        return dereference;
    }

    /**
     * save_js_trace_file enabled
     *
     * @param key save_js_trace_file
     * @return String
     */
    @Nullable
    public static String getArkVMCollectConfig(String key) {
        return message(key, FALSE);
    }
}
