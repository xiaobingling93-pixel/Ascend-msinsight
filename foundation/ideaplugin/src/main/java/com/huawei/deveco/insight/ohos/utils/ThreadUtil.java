/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 */

package com.huawei.deveco.insight.ohos.utils;

import com.intellij.openapi.application.Application;
import com.intellij.openapi.application.ApplicationManager;

import org.jetbrains.annotations.NotNull;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;
import java.util.concurrent.FutureTask;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * thread utility
 *
 * @since 2022-1-1
 */
public final class ThreadUtil {
    private static final LogPrinter LOGGER = LogPrinter.createLogger(ThreadUtil.class);

    /**
     * thread sleep
     *
     * @param ms milliseconds
     */
    public static void threadSleep(long ms) {
        try {
            Thread.sleep(ms);
        } catch (InterruptedException e) {
            LOGGER.warn("Thread sleep failed.");
        }
    }

    /**
     * run runnable in application's thread pool
     *
     * @param application Application
     * @param runnable    runnable
     * @return future
     */
    public static Future<?> runInBackground(@NotNull Application application, Runnable runnable) {
        return application.executeOnPooledThread(runnable);
    }

    /**
     * run runnable in UI Thread
     *
     * @param runnable runnable
     */
    public static void runInUIThread(Runnable runnable) {
        ApplicationManager.getApplication().invokeLater(runnable);
    }

    /**
     * FutureTaskThread
     *
     * @param <T> T
     */
    public static class FutureTaskThread<T> extends Thread {
        private FutureTask<T> task = null;

        /**
         * constructor
         *
         * @param task task
         */
        public FutureTaskThread(FutureTask<T> task) {
            super(task);
            this.task = task;
        }

        /**
         * execute task and get result
         *
         * @return T
         */
        public T executeAndGetResult() {
            T ret = null;
            this.start();
            try {
                ret = task.get();
            } catch (InterruptedException e) {
                LOGGER.warn("Occurs an error: execute task is interrupted failed.");
            } catch (ExecutionException e) {
                LOGGER.warn("Occurs an error: execute task is executed failed.");
            }
            return ret;
        }

        /**
         * execute task
         */
        public void execute() {
            this.start();
        }

        /**
         * terminate
         */
        public void terminate() {
            task.cancel(true);
        }
    }

    /**
     * ThreadPool
     *
     * @since 2022-1-28
     */
    public static class ThreadPool {
        private static final LogPrinter LOGGER = LogPrinter.createLogger(ThreadPool.class.getSimpleName());
        private static final int TASK_QUEUE_CAPACITY = 100;
        private static final int MAXIMUM_POOL_SIZE = 64;
        private static final int CUSTOM_THREAD_FACTORY_MAX_THREAD = 64;

        /**
         * create new cached thread pool
         *
         * @param threadPoolName threadPoolName
         * @return executor service
         */
        public static ExecutorService newCachedThreadPool(String threadPoolName) {
            return getThreadPoolExecutor(0, threadPoolName);
        }

        /**
         * <h2>get custom threadPoolExecutor</h2>
         *
         * @param corePoolSize corePoolSize
         * @param threadPoolName threadPoolName
         * @return threadPoolExecutor
         */
        public static ThreadPoolExecutor getThreadPoolExecutor(int corePoolSize, String threadPoolName) {
            BlockingQueue<Runnable> blockingQueue = new LinkedBlockingQueue<>(TASK_QUEUE_CAPACITY);

            return new ThreadPoolExecutor(corePoolSize, MAXIMUM_POOL_SIZE, 60L, TimeUnit.SECONDS,
                    blockingQueue, new CustomThreadFactory(CUSTOM_THREAD_FACTORY_MAX_THREAD, threadPoolName),
                    new ThreadPoolExecutor.DiscardOldestPolicy());
        }

        /**
         * get single thread Executor
         *
         * @param theadPoolName theadPoolName
         * @return threadPoolExecutor
         */
        public static ThreadPoolExecutor getSingleThreadExecutor(String theadPoolName) {
            LOGGER.info("singleThreadExecutor name {}", theadPoolName);
            return new ThreadPoolExecutor(1, 1, 0L, TimeUnit.MILLISECONDS, new LinkedBlockingQueue<>());
        }
    }

    /**
     * CustomThreadFactory
     *
     * @since 2022-1-28
     */
    public static class CustomThreadFactory implements ThreadFactory {
        private static final LogPrinter LOGGER = LogPrinter.createLogger(CustomThreadFactory.class.getSimpleName());

        private final int maxThread;

        private final AtomicInteger count = new AtomicInteger(0);

        private final String threadName;

        /**
         * constructor
         *
         * @param maxThread maxThread
         * @param threadName threadName
         */
        public CustomThreadFactory(int maxThread, String threadName) {
            this.maxThread = maxThread;
            this.threadName = threadName;
        }

        @Override
        public Thread newThread(@NotNull Runnable runnable) {
            int incrementAndGet = count.incrementAndGet();
            if (incrementAndGet > maxThread) {
                count.decrementAndGet();
                LOGGER.info("Thread creation is greater than the set threshold.");
            }
            return new Thread(runnable, threadName);
        }
    }
}