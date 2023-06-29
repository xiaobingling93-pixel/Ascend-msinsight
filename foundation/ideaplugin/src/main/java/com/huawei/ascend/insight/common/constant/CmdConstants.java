/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.ascend.insight.common.constant;

import com.huawei.ascend.insight.utils.ExecutableUtils;

import com.intellij.openapi.application.PathManager;

import org.apache.commons.io.FileUtils;

import java.io.File;

/**
 * CmdConstants
 *
 * @since 2022-10-20
 */
public class CmdConstants {
    /**
     * WINDOWS_CMD
     */
    public static final String WINDOWS_CMD = "CMD";

    /**
     * WINDOWS_CMD_TERMINAL
     */
    public static final String WINDOWS_CMD_TERMINAL = "/C";

    /**
     * PORT_PREFIX
     */
    public static final String PORT_PREFIX = "--wsPort=";

    /**
     * LOG_PATH
     */
    public static final String LOG_PATH = "--logPath=";

    /**
     * SID_PREFIX
     */
    public static final String SID_PREFIX = "--sid=";

    /**
     * PORT_EQUAL
     */
    public static final String PORT_EQUAL = "ws_port=";

    /**
     * HDC_EQUAL
     */
    public static final String HDC_EQUAL = "hdc_path=";

    /**
     * DIC_SERVER
     */
    public static final String DIC_SERVER = "profiler-server" + ExecutableUtils.ext(".exe", "");

    /**
     * TRACE_STREAMER
     */
    public static final String TRACE_STREAMER = "trace_streamer" + ExecutableUtils.ext(".exe", "");

    /**
     * SYSTEM_PATH
     */
    public static final String SYSTEM_PATH = PathManager.getSystemPath();

    /**
     * INSIGHT_LOG_PATH
     */
    public static final File INSIGHT_LOG_PATH = FileUtils.getFile(SYSTEM_PATH, ".insight" + File.separator + "log");

    /**
     * INSIGHT_LOG_PATH_PERMISSION
     */
    public static final String INSIGHT_LOG_PERMISSION = "rwxr-x---";

    /**
     * INSIGHT_TRACE_PATH
     */
    public static final File INSIGHT_TRACE_PATH = FileUtils.getFile(SYSTEM_PATH, ".insight" + File.separator + "trace");

    /**
     * INSIGHT_TRACE_PATH_PERMISSION
     */
    public static final String INSIGHT_TRACE_PERMISSION = "rwxr-x---";

    /**
     * INSIGHT_DB_PATH
     */
    public static final File INSIGHT_DB_PATH = FileUtils.getFile(SYSTEM_PATH, ".insight" + File.separator + "db");

    /**
     * INSIGHT_DB_PERMISSION
     */
    public static final String INSIGHT_DB_PERMISSION = "rwxr-x---";

    /**
     * INSIGHT_TMP_PATH
     */
    public static final File INSIGHT_TMP_PATH = FileUtils.getFile(SYSTEM_PATH, ".insight" + File.separator + "tmp");

    /**
     * INSIGHT_TMP_PERMISSION
     */
    public static final String INSIGHT_TMP_PERMISSION = "rwxr-x---";

    /**
     * INSIGHT_DEFAULT_PERMISSION
     */
    public static final String INSIGHT_DEFAULT_PERMISSION = "rwxr-x---";

    /**
     * WS_BASE_PORT
     */
    public static final String WS_BASE_PORT = "6001";

    /**
     * LINE_AND
     */
    public static final String LINE_AND = "&";

    /**
     * LINE_QUESTION_MASK
     */
    public static final String LINE_QUESTION_MASK = "?";

    /**
     * LINE_DOUBLE_QUOTATION_MASK
     */
    public static final String LINE_DOUBLE_QUOTATION_MASK = "\"";

    /**
     * WINDOWS_TASKLIST
     */
    public static final String WINDOWS_TASKLIST = "TASKLIST";

    /**
     * WINDOWS_TASKLIST_FILTER
     */
    public static final String WINDOWS_TASKLIST_FILTER = "/FI";

    /**
     * WINDOWS_TASKLIST_IMAGE_NAME
     */
    public static final String WINDOWS_TASKLIST_IMAGE_NAME = "IMAGENAME";

    /**
     * WINDOWS_TASKLIST_EQUEL
     */
    public static final String WINDOWS_TASKLIST_EQUEL = "eq";

    /**
     * WINDOWS_TASKKILL
     */
    public static final String WINDOWS_TASKKILL = "TASKKILL";

    /**
     * WINDOWS_TASKKILL_FORCEFULLY
     */
    public static final String WINDOWS_TASKKILL_FORCEFULLY = "/F";

    /**
     * WINDOWS_TASKKILL_IMAGENAME
     */
    public static final String WINDOWS_TASKKILL_IMAGE_NAME = "/IM";

    /**
     * WINDOWS_TASKKILL_CHILD_PROCESSES
     */
    public static final String WINDOWS_TASKKILL_CHILD_PROCESSES = "/T";

    /**
     * WINDOWS_NETSTAT_FINDSTR
     */
    public static final String WINDOWS_NETSTAT_FINDSTR = "netstat -ano | findstr";

    /**
     * linux and mac run command
     */
    public static final String LINUX_MAC_BASH_COMMAND = "/bin/sh";

    /**
     * LINUX_MAC_BASH_TERMINAL
     */
    public static final String LINUX_MAC_BASH_TERMINAL = "-c";

    /**
     * LINUX_MAC_SHOW_AND_GREP_PROCESS
     */
    public static final String LINUX_MAC_SHOW_AND_GREP_PROCESS = "ps -ef | grep ";

    /**
     * LINUX_MAC_LSOF
     */
    public static final String LINUX_MAC_LSOF = "lsof -i:";

    /**
     * LINUX_MAC_KILLALL
     */
    public static final String LINUX_MAC_KILLALL = "killall";

    /**
     * LINUX_MAC_KILL_SIGNAL_NINE
     */
    public static final String LINUX_MAC_KILL_SIGNAL_NINE = "-9";

    /**
     * start get sysEvent command
     */
    public static final String HDC_HI_SYS_EVENT = "hisysevent -r -d";

    /**
     * watch cmd with cat gpu usage
     */
    public static final String HDC_WATCH_GPU_DATA =
        "watch -n 0.1 cat /sys/class/devfreq/gpufreq/gpu_scene_aware/utilisation";

    /**
     * get current ability state command
     */
    public static final String HDC_GET_ABILITY_STATE = "aa dump -a";

    /**
     * start record trace command
     */
    public static final String HDC_HI_TRACE_START = "hitrace --trace_begin ability app";

    /**
     * trace path
     */
    public static final String HDC_HI_TRACE_PATH = "/data/local/tmp/";

    /**
     * stop record trace command
     */
    public static final String HDC_HI_TRACE_STOP = "hitrace --trace_finish -o ";

    /**
     * file recv command
     */
    public static final String HDC_FILE_RECV = "file recv %s %s";

    /**
     * file rm command
     */
    public static final String HDC_FILE_RM = "rm %s";

    /**
     * stop hiProfiler
     */
    public static final String OHOS_STOP_HI_PROFILER_COMMAND = "hiprofiler_cmd -k";

    /**
     * stop hiPerf
     */
    public static final String OHOS_STOP_HI_PERF_RECORD_COMMAND = "killall -2 hiperf";

    /**
     * start hiProfiler
     */
    public static final String OHOS_START_HI_PROFILER_COMMAND = "hiprofiler_cmd -s";

    /**
     * wake up process
     */
    public static final String OHOS_UNFREEZED_APP_COMMAND = "hidumper -s 1910 -a 'Thaw %s'";

    /**
     * processName of profiler
     */
    public static final String OHOS_PROFILER_NAME = "hiprofilerd";

    /**
     * processName of profiler plugins
     */
    public static final String OHOS_PROFILER_PLUGIN_NAME = "hiprofiler_plugins";

    /**
     * get process list command
     */
    public static final String OHOS_PROCESS_LIST_COMMAND = "ps -ef";

    /**
     * get thread name prefix
     */
    public static final String OHOS_CAT_PROC_COMMAND = "cat /proc/";

    /**
     * get thread name
     */
    public static final String OHOS_TASK_COMMAND = "/task/";

    /**
     * get thread name
     */
    public static final String OHOS_CMD_LINE_COMMAND = "/cmdline";

    /**
     * perf.data file path in device
     */
    public static final String PERF_DATA_PATH = "/data/local/tmp/perf.data";

    /**
     * hiperf record help
     */
    public static final String OHOS_HIPERF_HELP_COMMAND = "hiperf record --help";

    /**
     * HIPERF_RECORD_PART1 HIPERF_RECORD_EXCLUDE HIPERF_RECORD_PART2 workaround CodeCheck
     */
    public static final String HIPERF_RECORD_PART1 = "hiperf record --control prepare --clockid monotonic -p %s";

    /**
     * HIPERF_RECORD_PART1 HIPERF_RECORD_EXCLUDE HIPERF_RECORD_PART2 workaround CodeCheck
     */
    public static final String HIPERF_RECORD_EXCLUDE = " --exclude-thread SamplingThread";

    /**
     * HIPERF_RECORD_PART1 HIPERF_RECORD_EXCLUDE HIPERF_RECORD_PART2 workaround CodeCheck
     */
    public static final String HIPERF_RECORD_PART2 = " -f 2000 --call-stack dwarf --offcpu";

    /**
     * OpenHarmony prepare hiperf
     */
    public static final String OHOS_PREPARE_HIPERF_RECORD_COMMAND =
        HIPERF_RECORD_PART1 + HIPERF_RECORD_EXCLUDE + " -o " + PERF_DATA_PATH + HIPERF_RECORD_PART2;

    /**
     * query gpu usage
     */
    public static final String OHOS_QUERY_GPU_USAGE_COMMAND =
        "cat /sys/class/devfreq/gpufreq/gpu_scene_aware/utilisation";

    /**
     * OpenHarmony prepare hiperf does not support --exclude-thread
     */
    public static final String OHOS_PREPARE_HIPERF_RECORD_NOT_SUPPORT_EXCLUDE_COMMAND =
        HIPERF_RECORD_PART1 + " -o " + PERF_DATA_PATH + HIPERF_RECORD_PART2;

    /**
     * OpenHarmony start hiperf
     */
    public static final String OHOS_START_HIPERF_RECORD_COMMAND = "hiperf record --control start";

    /**
     * OpenHarmony stop hiperf
     */
    public static final String OHOS_STOP_HIPERF_RECORD_COMMAND = "hiperf record --control stop";

    /**
     * perf file prefix
     */
    public static final String PERF_DATA = "perf_data_";

    /**
     * perf file suffix
     */
    public static final String TRACE_SUFFIX = ".data";

    /**
     * forward port
     */
    public static final String FORWARD_PORT = "fport %s %s";

    /**
     * rm forward port
     */
    public static final String FORWARD_RM_PORT = "fport rm %s %s";

    /**
     * local abstract
     */
    public static final String LOCAL_ABSTRACT = "localabstract:%s";

    /**
     * tcp
     */
    public static final String TCP = "tcp:%s";

    /**
     * forward port result
     */
    public static final String RESULT_OK = "OK";

    /**
     * rm forward port result
     */
    public static final String RESULT_SUCCESS = "success";

    /**
     * PandaDebugger
     */
    public static final String PANDA_DEBUGGER = "PandaDebugger";
}
