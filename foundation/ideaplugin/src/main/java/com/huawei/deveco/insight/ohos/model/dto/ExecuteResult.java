/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.deveco.insight.ohos.model.dto;

/**
 * RExecuteResult
 *
 * @since 2023/6/28
 */
public class ExecuteResult {
    private final int exitCode;

    private final String executeOut;

    public ExecuteResult(int exitCode, String executeOut) {
        this.exitCode = exitCode;
        this.executeOut = executeOut;
    }

    public int getExitCode() {
        return exitCode;
    }

    public String getExecuteOut() {
        return executeOut;
    }
}
