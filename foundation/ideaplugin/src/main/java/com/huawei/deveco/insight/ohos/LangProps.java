/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 */

package com.huawei.deveco.insight.ohos;

import com.intellij.AbstractBundle;
import com.intellij.DynamicBundle;
import com.intellij.reference.SoftReference;

import java.lang.ref.Reference;
import java.util.ResourceBundle;

/**
 * Language properties
 *
 * @since 2022-11-01
 */
public final class LangProps {
    private static Reference<ResourceBundle> ourBundle;

    /**
     * message
     *
     * @param key key
     * @return String
     */
    public static String message(String key) {
        return AbstractBundle.message(getBundle(), key);
    }

    private static ResourceBundle getBundle() {
        String bundleName = DynamicBundle.getLocale().getLanguage();
        if (bundleName == null || bundleName.isEmpty()) {
            bundleName = "message.en";
        }
        ResourceBundle bundle = SoftReference.dereference(ourBundle);
        if (bundle == null) {
            bundle = ResourceBundle.getBundle("message." + bundleName);
            ourBundle = new SoftReference<>(bundle);
        }
        return bundle;
    }
}