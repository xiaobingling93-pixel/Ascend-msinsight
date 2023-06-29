/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2021. All rights reserved.
 */

package com.huawei.ascend.insight.utils;

import com.intellij.CommonBundle;
import com.intellij.reference.SoftReference;

import org.jetbrains.annotations.NonNls;
import org.jetbrains.annotations.PropertyKey;

import java.lang.ref.Reference;
import java.util.ResourceBundle;

/**
 * log properties
 *
 * @since 2021-05-27
 */
public class LogProperties {
    private static final String TRUE = "true";

    private static final String FALSE = "false";

    private static final String DEBUG = "debug";

    private static final String INFO = "info";

    private static final String WARN = "warn";

    private static final String ERROR = "error";

    @NonNls
    private static final String BUNDLE = "Log";

    private static Reference<ResourceBundle> ourBundle;

    /**
     * log level
     */
    public enum LogLevel {
        DEBUG(4),
        INFO(3),
        WARN(2),
        ERROR(1);

        int level;

        LogLevel(int level) {
            this.level = level;
        }
        public int getLevel() {
            return level;
        }
    }

    /**
     * message
     *
     * @param key bundle key
     * @param params params
     * @return value
     */
    private static String message(@PropertyKey(resourceBundle = BUNDLE) String key, Object... params) {
        return CommonBundle.messageOrNull(getBundle(), key, params);
    }

    private static ResourceBundle getBundle() {
        ResourceBundle resourceBundle = SoftReference.dereference(ourBundle);
        if (resourceBundle == null) {
            resourceBundle = ResourceBundle.getBundle(BUNDLE);
            ourBundle = new SoftReference<>(resourceBundle);
        }
        return resourceBundle;
    }

    /**
     * get log level
     *
     * @return log level
     */
    public static LogLevel getLogLevel() {
        String logLevelStr = message("log_level", WARN);
        switch (logLevelStr) {
            case DEBUG:
                return LogLevel.DEBUG;
            case INFO:
                return LogLevel.INFO;
            case ERROR:
                return LogLevel.ERROR;
            case WARN:
            default:
                return LogLevel.WARN;
        }
    }
}
