/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.ascend.insight.common.constant;

import com.intellij.openapi.util.text.Strings;

/**
 * URLConstants
 *
 * @since 2022/04/02
 */
public final class URLConstants {
    /**
     * PROFILER_SCHEME_NAME
     */
    public static final String PROFILER_SCHEME_NAME = "http";

    /**
     * PROFILER_URL_PREFIX
     */
    public static final String PROFILER_URL_PREFIX = PROFILER_SCHEME_NAME + "://";

    /**
     * DOMAIN_NAME
     */
    public static final String DOMAIN_NAME = "localhost";

    /**
     * PROFILER_ORIGIN
     */
    public static final String PROFILER_ORIGIN = Strings.join(PROFILER_URL_PREFIX, DOMAIN_NAME, "/");

    /**
     * RESOURCE_DIR
     */
    public static final String RESOURCE_DIR = "frontend";

    /**
     * HOME_PAGE
     */
    public static final String HOME_PAGE = PROFILER_ORIGIN + "index.html";

    /**
     * DEBUG_PORT
     */
    public static final String DEBUG_PORT = "3000";

    /**
     * DEBUG_DOMAIN_NAME
     */
    public static final String DEBUG_DOMAIN_NAME = Strings.join(DOMAIN_NAME, ":", DEBUG_PORT);

    /**
     * DEBUG_PROFILER_ORIGIN
     */
    public static final String DEBUG_PROFILER_ORIGIN = Strings.join(PROFILER_URL_PREFIX,
        DEBUG_DOMAIN_NAME, "/");

    /**
     * DEBUG_HOME_PAGE
     */
    public static final String DEBUG_HOME_PAGE = DEBUG_PROFILER_ORIGIN + "index.html";

    // forbid instantiation
    private URLConstants() {
    }
}
