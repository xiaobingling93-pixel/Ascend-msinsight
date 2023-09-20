/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 */

package com.huawei.ascend.insight.common;

import lombok.Data;

/**
 * Insight Error, use this class as Enum
 * e.g. InsightError.UNKNOWN_ERROR
 *
 * @since 2023-03-20
 */
@Data
public class InsightError {
    /**
     * Server - Import file error
     */
    public static final InsightError IMPORT_ERROR = new InsightError(5010, "Failed to import .insight data file");

    private final Integer code;

    private final String message;

    /**
     * InsightError is originally an enum type
     * Caller does not need to call construct to build a new error type
     * All available error type has been defined public
     *
     * @param code error code
     * @param message error message
     */
    private InsightError(Integer code, String message) {
        this.code = code;
        this.message = message;
    }
}