/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.ascend.insight.service;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.TimeUnit;

import com.huawei.ascend.insight.common.constant.CmdConstants;
import com.huawei.ascend.insight.utils.*;
import com.intellij.openapi.util.SystemInfo;

/**
 * DicHelper
 *
 * @since 2022-10-20
 */
public class ServerHelper {
    private static final LogPrinter LOGGER = LogPrinter.createLogger(ServerHelper.class);

    private static String port = "9000";

    private static String sid = null;

    /**
     * start dic server
     *
     * @return boolean
     */
    public static boolean startServer() {
        if (ProcessUtils.findProcess(CmdConstants.DIC_SERVER)) {
            return true;
        }
        String serverPath = ExecutableUtils.getServerInstallPath();
        int lastSepIdx = serverPath.lastIndexOf(File.separator);
        if (lastSepIdx < 0 || lastSepIdx == serverPath.length() - 1) {
            LOGGER.warn("wrong server path:{}", serverPath);
            return false;
        }
        List<String> processArgs = new ArrayList<>();
        if (SystemInfo.isWindows) {
            processArgs.add(CmdConstants.WINDOWS_CMD);
            processArgs.add(CmdConstants.WINDOWS_CMD_TERMINAL);
            processArgs.add(CmdConstants.DIC_SERVER);
        } else if (SystemInfo.isMac) {
            processArgs.add("./" + CmdConstants.DIC_SERVER);
        } else {
            LOGGER.info("start dicServer error, system not supported");
        }
        String execDir = serverPath.substring(0, lastSepIdx);
        LOGGER.info("start dic server, execDir is {}", execDir);
        Optional<Process> process = ProcessUtils.execute(processArgs, execDir);
        try {
            if (process.isEmpty()) {
                return false;
            }
            process.get().waitFor(2, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            LOGGER.warn("start dic server's process waitFor occur error");
        }
        return ProcessUtils.findProcess(CmdConstants.DIC_SERVER);
    }
}
