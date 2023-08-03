/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.ascend.insight.service;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

import com.huawei.ascend.insight.common.constant.CmdConstants;
import com.huawei.ascend.insight.utils.*;
import com.intellij.notification.NotificationType;
import com.intellij.openapi.application.PathManager;
import com.intellij.openapi.util.SystemInfo;
import com.intellij.util.concurrency.AppExecutorUtil;

/**
 * DicHelper
 *
 * @since 2022-10-20
 */
public class ServerHelper {
    private static final LogPrinter LOGGER = LogPrinter.createLogger(ServerHelper.class);

    private static ScheduledFuture<?> startServerHook = null;

    private static boolean hasBeenDead = false;

    private static boolean isFirstStart = true;

    private static Process serverProcess = null;

    private static int tryRestartTime = 0;

    public static void startServer() {
        ThreadUtil.runInUIThread(() -> {
            executeStartServerCommand();
            startServerHook = AppExecutorUtil.getAppScheduledExecutorService()
                .scheduleWithFixedDelay(ServerHelper::serverCheckAndRestart, 10, 3, TimeUnit.SECONDS);
        });
    }

    public static void cancelServerHook() {
        if (startServerHook != null) {
            startServerHook.cancel(true);
        }
        startServerHook = null;
    }

    private static void serverCheckAndRestart() {
        // 未找到server进程
        if (!ProcessUtils.findProcess(CmdConstants.DIC_SERVER)) {
            hasBeenDead = true;
            if (++tryRestartTime <= 5) {
                LOGGER.info("try to start server again!");
                BalloonNotification.show(
                    "[Ascend Insight]: server is dead,try to restart now, tryTime: " + tryRestartTime,
                    NotificationType.WARNING);
                executeStartServerCommand();
                return;
            }
            BalloonNotification.show("[Ascend Insight]: server restart failed", NotificationType.ERROR);
            return;
        }
        // 找到了server进程
        if (isFirstStart) {
            LOGGER.info("server start success");
            isFirstStart = false;
        }
        if (hasBeenDead) {
            hasBeenDead = false;
            tryRestartTime = 0;
            BalloonNotification.show("[Ascend Insight]: server has been started, please clear and try again",
                NotificationType.INFORMATION);
        }

    }

    private static void executeStartServerCommand() {
        destroy();
        String lineSeparator = StringUtil.lineSeparator;
        String pluginsPath = PathManager.getPluginsPath() + lineSeparator + "ascend-insight" + lineSeparator + "tools";
        List<String> processArgs = new ArrayList<>();
        if (SystemInfo.isWindows) {
            processArgs.add(CmdConstants.WINDOWS_CMD);
            processArgs.add(CmdConstants.WINDOWS_CMD_TERMINAL);
            processArgs.add(CmdConstants.DIC_SERVER);
            Optional<Process> execute = ProcessUtils.execute(processArgs, pluginsPath);
            serverProcess = execute.orElse(null);
            return;
        }
        try {
            Runtime.getRuntime().exec("chmod +x " + pluginsPath + lineSeparator + CmdConstants.DIC_SERVER);
            serverProcess = Runtime.getRuntime().exec(pluginsPath + lineSeparator + CmdConstants.DIC_SERVER);
        } catch (IOException e) {
            LOGGER.info(e.getMessage());
        }
    }

    public static void destroy() {
        if(serverProcess != null) {
            serverProcess.destroy();
            serverProcess = null;
        }
    }
}
