/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.ascend.insight.utils;

import com.huawei.ascend.insight.model.dto.ExecuteResult;

import com.intellij.openapi.util.SystemInfo;

import org.apache.commons.lang3.StringUtils;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * ShellCommand
 *
 * @since 2022-10-20
 */
public class ShellCommand {
    private static final LogPrinter LOGGER = LogPrinter.createLogger(ShellCommand.class);

    private static final Pattern DEFAULT_CHECK_PORT_CMD_REG = Pattern.compile("\\d+");

    /**
     * execute command
     *
     * @param command command
     * @return execute result
     */
    public static Optional<ExecuteResult> executeCommand(List<String> command) {
        if (command == null || command.isEmpty()) {
            return Optional.empty();
        }
        ProcessBuilder processBuilder = new ProcessBuilder();
        processBuilder.command(command);
        processBuilder.directory(new File(System.getProperty("user.home")));
        processBuilder.redirectErrorStream(true);
        Process process;
        ExecutorService threadPool = new ThreadPoolExecutor(0, 64, 60L, TimeUnit.SECONDS, new SynchronousQueue<>());
        try {
            process = processBuilder.start();
            Future<String> outputFuture = threadPool.submit(new StreamCallable(process.getInputStream()));
            String output = outputFuture.get();
            return Optional.of(new ExecuteResult(process.waitFor(), output));
        } catch (IOException | InterruptedException | ExecutionException e) {
            return Optional.empty();
        } finally {
            threadPool.shutdown();
        }
    }

    private static final class StreamCallable implements Callable<String> {
        private final InputStream stream;

        public StreamCallable(InputStream stream) {
            this.stream = stream;
        }

        @Override
        public String call() throws Exception {
            String retString = "";
            BufferedReader brInputStream =
                    new BufferedReader(new InputStreamReader(stream, StandardCharsets.UTF_8));
            try {
                String line;
                while ((line = brInputStream.readLine()) != null) {
                    retString += line;
                    retString += System.lineSeparator();
                }
                return retString;
            } catch (IOException e) {
                LOGGER.warn("Error reading next line!!");
                return null;
            } finally {
                brInputStream.close();
            }
        }
    }

    /**
     * execute command output is or not valid
     *
     * @param output execute command output
     * @param port port
     * @return boolean
     */
    public static boolean isValid(String output, String port) {
        Pattern pattern = DEFAULT_CHECK_PORT_CMD_REG;
        if (SystemInfo.isWindows) {
            pattern = Pattern.compile("(\\d+\\.\\d+\\.\\d+\\.\\d+):" + port + "\\s+");
        }
        Matcher matcher = pattern.matcher(output);
        return StringUtils.isNotEmpty(output) && matcher.find();
    }
}
