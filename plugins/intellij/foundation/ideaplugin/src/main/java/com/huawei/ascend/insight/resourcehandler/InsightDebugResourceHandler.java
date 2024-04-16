/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.ascend.insight.resourcehandler;

import com.huawei.ascend.insight.common.constant.UrlConstants;
import com.huawei.ascend.insight.utils.LogPrinter;

import org.cef.callback.CefCallback;
import org.cef.network.CefRequest;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.Set;

/**
 * Load debug resources when developing
 *
 * @since 2022-06-23
 */
public class InsightDebugResourceHandler extends InsightResourceHandler {
    private static final LogPrinter LOGGER = LogPrinter.createLogger(InsightDebugResourceHandler.class);

    private static final Set<String> DEBUG_DIC_FILES = Set.of(
        UrlConstants.DEBUG_PROFILER_ORIGIN + "static/js/dic_client_wasm.worker.js",
        UrlConstants.DEBUG_PROFILER_ORIGIN + "static/js/dic_client_wasm.wasm",
        UrlConstants.DEBUG_PROFILER_ORIGIN + "static/js/dic_client_wasm.mjs"
    );

    public InsightDebugResourceHandler(String requestUrl) {
        super(requestUrl);
    }

    @Override
    public boolean processRequest(CefRequest request, CefCallback callback) {
        if (!requestUrl.equals(request.getURL())) {
            throw new IllegalStateException();
        }
        try {
            URL resourceUrl;
            if (DEBUG_DIC_FILES.contains(requestUrl)) {
                if (requestUrl.length() < UrlConstants.PROFILER_URL_PREFIX.length()) {
                    return false;
                }
                String transformedUrl = requestUrl.substring(UrlConstants.PROFILER_URL_PREFIX.length())
                    .replaceFirst(UrlConstants.DEBUG_DOMAIN_NAME, UrlConstants.RESOURCE_DIR);
                resourceUrl = getClass().getClassLoader().getResource(transformedUrl);
            } else {
                resourceUrl = new URL(requestUrl);
            }
            if (resourceUrl == null) {
                LOGGER.warn("Failed to load resource {}", requestUrl);
                return false;
            }
            connection = resourceUrl.openConnection();
            inputStream = connection.getInputStream();
        } catch (MalformedURLException e) {
            LOGGER.error("MalformedURL: " + requestUrl);
        } catch (IOException e) {
            LOGGER.error("Open URL error: " + requestUrl);
        }
        callback.Continue();
        return true;
    }
}
