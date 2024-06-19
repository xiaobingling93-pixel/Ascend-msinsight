/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.ascend.insight.service;

import com.huawei.ascend.insight.common.constant.CmdConstants;
import com.huawei.ascend.insight.utils.BalloonNotification;
import com.huawei.ascend.insight.utils.LogPrinter;
import com.huawei.ascend.insight.utils.ProcessUtils;
import com.huawei.ascend.insight.utils.StringUtil;

import com.intellij.notification.NotificationType;
import com.intellij.openapi.application.ApplicationManager;
import com.intellij.openapi.application.PathManager;
import com.intellij.openapi.util.SystemInfo;
import com.intellij.util.concurrency.AppExecutorUtil;

import org.apache.commons.io.IOUtils;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * DicHelper
 *
 * @since 2022-10-20
 */
public class ServerHelper {
    private static final LogPrinter LOGGER = LogPrinter.createLogger(ServerHelper.class);
    private static final Pattern PORT_PATTERN = Pattern.compile("\\d+");

    private static ScheduledFuture<?> startServerHook = null;

    private static boolean hasBeenDead = false;

    private static boolean isFirstStart = true;

    private static Process serverProcess = null;
    private static int serverPort = 9000;
    private static int tryRestartTime = 0;

    /**
     * startServer
     */
    public static void startServer() {
        ApplicationManager.getApplication().invokeAndWait(() -> {
            executeStartServerCommand();
            startServerHook = AppExecutorUtil.getAppScheduledExecutorService()
                .scheduleWithFixedDelay(ServerHelper::serverCheckAndRestart, 5, 3, TimeUnit.SECONDS);
        });
    }

    public static int getServerPort() {
        return serverPort;
    }

    /**
     * cancelServerHook
     */
    public static void cancelServerHook() {
        if (startServerHook != null) {
            startServerHook.cancel(true);
        }
        startServerHook = null;
    }

    private static void serverCheckAndRestart() {
        // 未找到server进程
        if (serverProcess != null && !serverProcess.isAlive()) {
            hasBeenDead = true;
            if (++tryRestartTime <= 5) {
                LOGGER.info("try to start server again!");
                BalloonNotification.show(
                    "[MindStudio Insight]: server is dead,try to restart now, tryTime: " + tryRestartTime,
                    NotificationType.WARNING);
                executeStartServerCommand();
                return;
            }
            BalloonNotification.show("[MindStudio Insight]: server restart failed", NotificationType.ERROR);
            startServerHook.cancel(true);
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
            BalloonNotification.show("[MindStudio Insight]: server has been started, please clear and try again",
                NotificationType.INFORMATION);
        }
    }

    private static void executeStartServerCommand() {
        destroy();
        String lineSeparator = StringUtil.FILE_SEPARATOR;
        String pluginsPath = PathManager.getPluginsPath() + lineSeparator
            + "mindstudio-insight" + lineSeparator + "tools";
        List<String> processArgs = new ArrayList<>();
        try {
            if (SystemInfo.isWindows) {
                processArgs.add(CmdConstants.WINDOWS_CMD);
                processArgs.add(CmdConstants.WINDOWS_CMD_TERMINAL);
                processArgs.add(CmdConstants.PROFILER_SERVER);
                processArgs.add(CmdConstants.SCAN_PREFIX + CmdConstants.WS_BASE_PORT);
                Process scanProcess = ProcessUtils.execute(processArgs, pluginsPath).get();
                processArgs.remove(processArgs.size() - 1);
                processArgs.add(CmdConstants.PORT_PREFIX + getAvailablePort(scanProcess));
                Optional<Process> execute = ProcessUtils.execute(processArgs, pluginsPath);
                serverProcess = execute.orElse(null);
                return;
            }
            Runtime.getRuntime().exec("chmod +x " + pluginsPath + lineSeparator + CmdConstants.PROFILER_SERVER);
            Process scanProcess = Runtime.getRuntime().exec(pluginsPath + lineSeparator
                    + CmdConstants.PROFILER_SERVER + ' ' + CmdConstants.SCAN_PREFIX + CmdConstants.WS_BASE_PORT);

            serverProcess = Runtime.getRuntime()
                .exec(pluginsPath + lineSeparator + CmdConstants.PROFILER_SERVER + ' ' + CmdConstants.PORT_PREFIX
                    + getAvailablePort(scanProcess));
        } catch (IOException | InterruptedException e) {
            LOGGER.info(e.getMessage());
        }
    }

    private static int getAvailablePort(Process scanProcess) throws InterruptedException, IOException {
        scanProcess.waitFor();
        String scanInfo = IOUtils.toString(scanProcess.getInputStream(), "UTF-8");
        int start = scanInfo.indexOf("Available port: ");
        if (start != -1) {
            Matcher matcher = PORT_PATTERN.matcher(scanInfo.substring(start));
            if (matcher.find()) {
                serverPort = Integer.parseInt(matcher.group());
                return serverPort;
            }
        }
        throw new IllegalStateException("Can\'t find available port");
    }

    /**
     * destroy
     */
    public static void destroy() {
        if (serverProcess != null) {
            serverProcess.destroy();
            serverProcess = null;
        }
    }
}
