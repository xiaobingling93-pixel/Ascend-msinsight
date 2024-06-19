/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.ascend.insight.utils;

import com.intellij.openapi.util.SystemInfo;

/**
 * ExecutableUtils
 *
 * @since 2022-10-20
 */
public class ExecutableUtils {
    private static final LogPrinter LOGGER = LogPrinter.createLogger(ExecutableUtils.class);

    /**
     * DIC_SERVER_PATH_MAC
     */
    private static final String DIC_SERVER_PATH_MAC = "profiler/server/profiler_server";

    /**
     * DIC_SERVER_PATH_WINDOWS
     */
    private static final String DIC_SERVER_PATH_WINDOWS = "profiler/server/profiler_server.exe";

    private static final String INSIGHT_PATH = "/mindstudio-insight";

    /**
     * get server install path
     *
     * @return path
     */
    public static String getServerInstallPath() {
        if (SystemInfo.isWindows) {
            return DIC_SERVER_PATH_WINDOWS;
        } else if (SystemInfo.isMac) {
            return DIC_SERVER_PATH_MAC;
        } else {
            return "";
        }
    }

    /**
     * ext 获取执行文件后缀名
     *
     * @param windowsExtension String
     * @param macExtension String
     * @return String
     */
    public static String ext(String windowsExtension, String macExtension) {
        if (SystemInfo.isWindows) {
            return windowsExtension;
        } else if (SystemInfo.isLinux) {
            return macExtension;
        } else {
            return "";
        }
    }
}
