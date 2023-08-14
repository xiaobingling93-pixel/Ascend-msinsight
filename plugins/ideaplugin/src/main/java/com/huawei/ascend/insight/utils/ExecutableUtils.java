/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.ascend.insight.utils;

import com.intellij.openapi.application.PathManager;
import com.intellij.openapi.util.SystemInfo;

import org.apache.commons.io.FileUtils;
import org.apache.commons.lang3.StringUtils;

import java.io.File;
import java.io.IOException;

/**
 * ExecutableUtils
 *
 * @since 2022-10-20
 */
public class ExecutableUtils {
    private static final LogPrinter LOGGER = LogPrinter.createLogger(ExecutableUtils.class);

    /**
     * TRACE_STREAMER_PATH_MAC
     */
    private static final String TRACE_STREAMER_PATH_MAC = "tools/profiler/dic_server/trace_streamer";

    /**
     * TRACE_STREAMER_PATH_WINDOWS
     */
    private static final String TRACE_STREAMER_PATH_WINDOWS = "tools/profiler/dic_server/trace_streamer.exe";

    /**
     * DIC_SERVER_PATH_MAC
     */
    private static final String DIC_SERVER_PATH_MAC = "frontend/static/profiler-server";

    /**
     * DIC_SERVER_PATH_WINDOWS
     */
    private static final String DIC_SERVER_PATH_WINDOWS = "frontend/static/profiler-server.exe";

    private static final String INSIGHT_PATH = "/ascend-insight";

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
     * get trace streamer path
     *
     * @return path
     */
    public static String getTraceStreamerPath() {
        if (SystemInfo.isWindows) {
            return getFilePath(TRACE_STREAMER_PATH_WINDOWS);
        } else if (SystemInfo.isMac) {
            return getFilePath(TRACE_STREAMER_PATH_MAC);
        } else {
            LOGGER.warn("get trace streamer failed, system not supported");
            return "";
        }
    }

    /**
     * get file path
     *
     * @param pathSuffix path
     * @return path
     */
    private static String getFilePath(String pathSuffix) {
        // 判断插件目录下是否安装insight,有则用插件目录下，没有则用ide目录下bundle插件的dic和trace_streamer
        String installedInsightPath = PathManager.getPluginsPath() + INSIGHT_PATH;
        File installedInsightFile = new File(installedInsightPath);
        String toolParentPath = installedInsightFile.exists() ? installedInsightPath : PathManager.getHomePath();
        if (StringUtils.isEmpty(toolParentPath)) {
            LOGGER.warn("mindstudio path is not found");
            return "";
        }
        File target = FileUtils.getFile(toolParentPath, pathSuffix);
        try {
            return target.getCanonicalPath();
        } catch (IOException e) {
            LOGGER.warn("get canonical path error");
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
