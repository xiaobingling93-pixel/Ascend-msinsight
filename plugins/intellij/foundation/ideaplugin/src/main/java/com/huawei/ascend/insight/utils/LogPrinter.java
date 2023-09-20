/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2021. All rights reserved.
 */

package com.huawei.ascend.insight.utils;

import com.intellij.openapi.diagnostic.Logger;

import org.jetbrains.annotations.NotNull;

import java.util.ArrayList;
import java.util.List;

/**
 * log printer
 *
 * @since 2021-05-26
 */
public final class LogPrinter {
    private static final String BIG_PARENTHESES = "{}";

    private final Logger intellijLogger;

    private LogPrinter(@NotNull String name) {
        intellijLogger = Logger.getInstance(name);
    }

    /**
     * create logger printer
     *
     * @param name logger name
     * @return log printer
     */
    public static LogPrinter createLogger(String name) {
        return new LogPrinter(name);
    }

    /**
     * create logger printer
     *
     * @param clazz logger class
     * @return log printer
     */
    public static LogPrinter createLogger(Class<?> clazz) {
        return new LogPrinter(clazz.getSimpleName());
    }

    /**
     * debug
     *
     * @param message message
     */
    public void debug(String message) {
        if (LogProperties.getLogLevel().getLevel() >= LogProperties.LogLevel.DEBUG.getLevel()) {
            intellijLogger.debug(message);
        }
    }

    /**
     * debug
     *
     * @param message message
     * @param throwable stack frame
     */
    public void debug(String message, Throwable throwable) {
        if (LogProperties.getLogLevel().getLevel() >= LogProperties.LogLevel.DEBUG.getLevel()) {
            intellijLogger.debug(message, throwable);
        }
    }

    /**
     * debug
     *
     * @param message message formatter
     * @param args arguments
     */
    public void debug(String message, Object... args) {
        if (LogProperties.getLogLevel().getLevel() >= LogProperties.LogLevel.DEBUG.getLevel()) {
            intellijLogger.debug(formatMessage(message, args));
        }
    }

    /**
     * info
     *
     * @param obj object to string
     */
    public void info(Object obj) {
        if (LogProperties.getLogLevel().getLevel() >= LogProperties.LogLevel.INFO.getLevel()) {
            intellijLogger.info(obj.toString());
        }
    }

    /**
     * info
     *
     * @param message message
     */
    public void info(String message) {
        if (LogProperties.getLogLevel().getLevel() >= LogProperties.LogLevel.INFO.getLevel()) {
            intellijLogger.info(message);
        }
    }

    /**
     * info
     *
     * @param message message
     * @param throwable stack frame
     */
    public void info(String message, Throwable throwable) {
        if (LogProperties.getLogLevel().getLevel() >= LogProperties.LogLevel.INFO.getLevel()) {
            intellijLogger.info(message, throwable);
        }
    }

    /**
     * info
     *
     * @param message message formatter
     * @param args arguments
     */
    public void info(String message, Object... args) {
        if (LogProperties.getLogLevel().getLevel() >= LogProperties.LogLevel.INFO.getLevel()) {
            intellijLogger.info(formatMessage(message, args));
        }
    }

    /**
     * warn
     *
     * @param message message
     */
    public void warn(String message) {
        if (LogProperties.getLogLevel().getLevel() >= LogProperties.LogLevel.WARN.getLevel()) {
            intellijLogger.warn(message);
        }
    }

    /**
     * warn
     *
     * @param message message
     * @param throwable stack frame
     */
    public void warn(String message, Throwable throwable) {
        if (LogProperties.getLogLevel().getLevel() >= LogProperties.LogLevel.WARN.getLevel()) {
            intellijLogger.warn(message, throwable);
        }
    }

    /**
     * warn
     *
     * @param message message formatter
     * @param args arguments
     */
    public void warn(String message, Object... args) {
        if (LogProperties.getLogLevel().getLevel() >= LogProperties.LogLevel.WARN.getLevel()) {
            intellijLogger.warn(formatMessage(message, args));
        }
    }

    /**
     * error
     *
     * @param throwable throwable
     */
    public void error(Throwable throwable) {
        if (LogProperties.getLogLevel().getLevel() >= LogProperties.LogLevel.ERROR.getLevel()) {
            intellijLogger.error(throwable);
        }
    }

    /**
     * error
     *
     * @param message message
     */
    public void error(String message) {
        if (LogProperties.getLogLevel().getLevel() >= LogProperties.LogLevel.ERROR.getLevel()) {
            intellijLogger.error(message);
        }
    }

    /**
     * error
     *
     * @param message message
     * @param throwable stack frame
     */
    public void error(String message, Throwable throwable) {
        if (LogProperties.getLogLevel().getLevel() >= LogProperties.LogLevel.ERROR.getLevel()) {
            intellijLogger.error(message, throwable);
        }
    }

    /**
     * error
     *
     * @param message message formatter
     * @param args arguments
     */
    public void error(String message, Object... args) {
        if (LogProperties.getLogLevel().getLevel() >= LogProperties.LogLevel.ERROR.getLevel()) {
            intellijLogger.error(formatMessage(message, args));
        }
    }

    private static String formatMessage(String message, Object... args) {
        if (message == null) {
            return "";
        }
        List<Integer> keyIndexList = new ArrayList<>();
        int keyIndex = -1;
        while ((keyIndex = message.indexOf(BIG_PARENTHESES, keyIndex + 1)) != -1) {
            keyIndexList.add(keyIndex);
        }
        StringBuilder result = new StringBuilder();
        if (keyIndexList.isEmpty()) {
            result.append(message);
            return result.toString();
        }

        result.append(message, 0, keyIndexList.get(0));
        for (int i = 0; i < keyIndexList.size(); i++) {
            result.append(args.length > i ? args[i] : "{null}");
            if (i == keyIndexList.size() - 1) {
                result.append(message, keyIndexList.get(i) + BIG_PARENTHESES.length(), message.length());
            } else {
                result.append(message, keyIndexList.get(i) + BIG_PARENTHESES.length(), keyIndexList.get(i + 1));
            }
        }

        return result.toString();
    }
}
