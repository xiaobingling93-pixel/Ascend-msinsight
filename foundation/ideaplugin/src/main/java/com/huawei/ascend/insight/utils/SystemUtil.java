/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 */

package com.huawei.ascend.insight.utils;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.Locale;

/**
 * system utility
 *
 * @since 2022-1-1
 */
public final class SystemUtil {
    private static final String OS_NAME = System.getProperty("os.name").toLowerCase(Locale.ROOT);

    private static final LogPrinter LOGGER = LogPrinter.createLogger(SystemUtil.class);

    /**
     * is windows
     *
     * @return true/false
     */
    public static boolean isWindows() {
        return OS_NAME.contains("win");
    }

    /**
     * is linux
     *
     * @return true/false
     */
    public static boolean isLinux() {
        return OS_NAME.contains("linux");
    }

    /**
     * is mac
     *
     * @return true/false
     */
    public static boolean isMac() {
        return OS_NAME.contains("mac");
    }

    /**
     * is unix
     *
     * @return true/false
     */
    public static boolean isUnix() {
        return (OS_NAME.contains("nix") || OS_NAME.contains("nux") || OS_NAME.contains("aix"));
    }

    /**
     * exec Command
     *
     * @param processArgs exec args list
     * @param isRecordLog whether redirect to log file
     * @param logName the file path of redirect
     * @return true/false
     */
    public static Boolean execCommand(List<String> processArgs, boolean isRecordLog, String logName) {
        ProcessBuilder processBuilder = new ProcessBuilder(processArgs);
        if (isRecordLog) {
            processBuilder.redirectOutput(new File(logName));
        }
        Process process = null;
        try {
            process = processBuilder.start();
            try (InputStream inputStream = process.getInputStream();
                 InputStream errorStream = process.getErrorStream();
                 BufferedReader brInputStream =
                        new BufferedReader(new InputStreamReader(inputStream, StandardCharsets.UTF_8));
                 BufferedReader brErrorStream =
                        new BufferedReader(new InputStreamReader(errorStream, StandardCharsets.UTF_8))) {
                while (brInputStream.readLine() != null || brErrorStream.readLine() != null) {}
            } catch (IOException ioException) {
                LOGGER.warn("exec Command, IOException occurred");
                return false;
            }
        } catch (IOException ioException) {
            LOGGER.warn("exec Command, IOException occurred");
            return false;
        } finally {
            if (process != null) {
                process.destroy();
            }
        }
        return true;
    }
}