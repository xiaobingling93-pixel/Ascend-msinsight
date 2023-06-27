/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.deveco.insight.ohos.cdp.util;

import com.huawei.deveco.insight.ohos.utils.LogPrinter;

import java.util.Map;
import java.util.Optional;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * ScheduledTaskManager
 *
 * @since 2022-10-22
 */
public class ScheduledTaskManager {
    private static final LogPrinter LOGGER = LogPrinter.createLogger(ScheduledTaskManager.class);

    private static volatile ScheduledTaskManager instance;

    private final Map<String, Runnable> taskHashMap = new ConcurrentHashMap<>();

    private final Map<String, ScheduledExecutorService> executorHashMap = new ConcurrentHashMap<>();

    /**
     * get single instance
     *
     * @return ScheduledTaskManager
     */
    public static ScheduledTaskManager getInstance() {
        if (instance == null) {
            synchronized (ScheduledTaskManager.class) {
                if (instance == null) {
                    instance = new ScheduledTaskManager();
                }
            }
        }
        return instance;
    }

    /**
     * add task to single thread executor service
     *
     * @param taskKey taskKey
     * @param task task
     */
    public void addSingleExecutorService(String taskKey, Runnable task) {
        LOGGER.info("addExecutorService taskKey is {}", taskKey);
        ScheduledExecutorService scheduled = Executors.newSingleThreadScheduledExecutor();
        executorHashMap.put(taskKey, scheduled);
        taskHashMap.put(taskKey, task);
    }

    /**
     * add task to executor service
     *
     * @param taskKey taskKey
     * @param task task
     */
    public void addExecutorService(String taskKey, Runnable task) {
        LOGGER.info("addExecutorService taskKey is {}", taskKey);
        int processors = Runtime.getRuntime().availableProcessors();
        ScheduledExecutorService scheduled = processors > 2
            ? new ScheduledThreadPoolExecutor(processors)
            : Executors.newSingleThreadScheduledExecutor();
        executorHashMap.put(taskKey, scheduled);
        taskHashMap.put(taskKey, task);
    }

    /**
     * start executor service, execute the task without delay
     *
     * @param taskKey taskKey
     * @param timeUnit timeUnit
     */
    public void startExecutorService(String taskKey, TimeUnit timeUnit) {
        LOGGER.info("startExecutorService taskKey is {}, timeUnit is {}", taskKey, timeUnit);
        ScheduledExecutorService scheduled = executorHashMap.get(taskKey);
        Runnable task = taskHashMap.get(taskKey);
        scheduled.schedule(task, 0, timeUnit);
    }

    /**
     * start executor service
     *
     * @param taskKey taskKey
     * @param delay delay
     * @param period period
     * @param timeUnit timeUnit
     */
    public void startExecutorService(String taskKey, long delay, long period, TimeUnit timeUnit) {
        LOGGER.info("startExecutorService taskKey is {}, period is {}, timeUnit is {}", taskKey, period, timeUnit);
        ScheduledExecutorService scheduled = executorHashMap.get(taskKey);
        Runnable task = taskHashMap.get(taskKey);
        if (delay > 0) {
            scheduled.scheduleWithFixedDelay(task, delay, period, timeUnit);
        } else {
            scheduled.scheduleAtFixedRate(task, 0, period, timeUnit);
        }
    }

    /**
     * delete executor service
     *
     * @param taskKey taskKey
     */
    public void deleteExecutorService(String taskKey) {
        LOGGER.info("deleteExecutorService taskKey is {}", taskKey);
        ScheduledExecutorService scheduledExecutorService = executorHashMap.get(taskKey);
        if (scheduledExecutorService != null) {
            scheduledExecutorService.shutdownNow();
            if (!executorHashMap.isEmpty()) {
                executorHashMap.remove(taskKey);
            }
            if (!taskHashMap.isEmpty()) {
                taskHashMap.remove(taskKey);
            }
        }
    }

    /**
     * check executor service
     *
     * @param taskKey taskKey
     * @return ScheduledExecutorService
     */
    public Optional<ScheduledExecutorService> checkExecutorService(String taskKey) {
        LOGGER.info("checkExecutorService taskKey is {}", taskKey);
        return Optional.ofNullable(executorHashMap.get(taskKey));
    }
}
