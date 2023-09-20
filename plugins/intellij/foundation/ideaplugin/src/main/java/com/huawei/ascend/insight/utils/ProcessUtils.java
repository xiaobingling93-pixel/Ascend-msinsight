/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.ascend.insight.utils;

import com.huawei.ascend.insight.common.constant.CmdConstants;
import com.huawei.ascend.insight.model.dto.ExecuteResult;

import com.intellij.execution.configurations.GeneralCommandLine;
import com.intellij.execution.configurations.PtyCommandLine;
import com.intellij.openapi.util.SystemInfo;

import org.apache.commons.io.FileUtils;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

/**
 * ProcessUtils
 *
 * @since 2022-10-20
 */
public class ProcessUtils {
    private static final LogPrinter LOGGER = LogPrinter.createLogger(ProcessUtils.class);

    /**
     * <h2>execute process</h2>
     * <p>
     * use PtyCommandLine class, IntelliJ platform class
     * example,
     * java -jar xxx.jar
     * </p>
     *
     * @param cmdList cmdList
     * @param executePath executePath
     * @return is the execution successful
     */
    public static Optional<Process> execute(List<String> cmdList, String executePath) {
        // 不打印cmdList 执行启动dic_server存在敏感信息(sid)
        try {
            PtyCommandLine cmd = new PtyCommandLine(cmdList);
            cmd.setWorkDirectory(FileUtils.getFile(executePath));
            cmd.withParentEnvironmentType(GeneralCommandLine.ParentEnvironmentType.CONSOLE);
            return Optional.of(cmd.createProcess());
        } catch (com.intellij.execution.ExecutionException e) {
            LOGGER.info(e.getMessage());
            return Optional.empty();
        }
    }

    /**
     * <h2>FindProcess</h2>
     * <p>
     * example,
     * TASKLIST /FI IMAGENAME EQ cmd.exe
     * return true when find the process name equal to processsName.
     * </p>
     *
     * @param processName process full name
     *
     * @return Boolean
     */
    public static Boolean findProcess(String processName) {
        BufferedReader bufferedReader = null;
        try {
            List<String> processArgs = new ArrayList<>();
            if (SystemInfo.isWindows) {
                processArgs.add(CmdConstants.WINDOWS_TASKLIST);
                processArgs.add(CmdConstants.WINDOWS_TASKLIST_FILTER);
                processArgs.add(CmdConstants.LINE_DOUBLE_QUOTATION_MASK);
                processArgs.add(CmdConstants.WINDOWS_TASKLIST_IMAGE_NAME);
                processArgs.add(CmdConstants.WINDOWS_TASKLIST_EQUEL);
                processArgs.add(processName);
                processArgs.add(CmdConstants.LINE_DOUBLE_QUOTATION_MASK);
            } else {
                processArgs.add(CmdConstants.LINUX_MAC_BASH_COMMAND);
                processArgs.add(CmdConstants.LINUX_MAC_BASH_TERMINAL);
                processArgs.add(CmdConstants.LINUX_MAC_SHOW_AND_GREP_PROCESS + processName);
            }
            Process proc = Runtime.getRuntime().exec(processArgs.toArray(new String[0]));
            bufferedReader = new BufferedReader(new InputStreamReader(proc.getInputStream(), StandardCharsets.UTF_8));
            String line;
            while ((line = bufferedReader.readLine()) != null) {
                if (line.contains(processName) && !line.contains("grep")) {
                    LOGGER.info("find dic server process");
                    return Boolean.TRUE;
                }
            }
            return Boolean.FALSE;
        } catch (IOException ex) {
            LOGGER.warn("get process inputStream failure");
            return Boolean.FALSE;
        } finally {
            if (bufferedReader != null) {
                try {
                    bufferedReader.close();
                } catch (IOException e) {
                    LOGGER.warn("find process, IOException occurred");
                }
            }
        }
    }

    /**
     * <h2>KillWindowsProcess</h2>
     * <p>
     * Specifies that processes be forcefully ended.
     * example,
     * TASKKILL /F /IM cmd.exe /T
     * It kills all processes with the same name.
     * </p>
     *
     * @param processName process full name
     */
    public static void killProcess(String processName) {
        List<String> processArgs = new ArrayList<>();
        if (SystemInfo.isWindows) {
            processArgs.add(CmdConstants.WINDOWS_TASKKILL);
            processArgs.add(CmdConstants.WINDOWS_TASKKILL_FORCEFULLY);
            processArgs.add(CmdConstants.WINDOWS_TASKKILL_IMAGE_NAME);
            processArgs.add(processName);
            processArgs.add(CmdConstants.WINDOWS_TASKKILL_CHILD_PROCESSES);
        } else {
            processArgs.add(CmdConstants.LINUX_MAC_KILLALL);
            processArgs.add(CmdConstants.LINUX_MAC_KILL_SIGNAL_NINE);
            processArgs.add(processName);
        }
        Optional<ExecuteResult> executeResult = ShellCommand.executeCommand(processArgs);
        if (executeResult.isPresent() && executeResult.get().getExitCode() == 0) {
            LOGGER.info("kill dic server process success");
        }
    }
}
