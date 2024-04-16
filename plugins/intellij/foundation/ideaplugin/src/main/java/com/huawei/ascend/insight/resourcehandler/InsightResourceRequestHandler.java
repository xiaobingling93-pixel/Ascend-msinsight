/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.ascend.insight.resourcehandler;

import com.huawei.ascend.insight.common.constant.UrlConstants;
import com.huawei.ascend.insight.utils.LogPrinter;

import org.cef.browser.CefBrowser;
import org.cef.browser.CefFrame;
import org.cef.handler.CefResourceHandler;
import org.cef.handler.CefResourceRequestHandlerAdapter;
import org.cef.network.CefRequest;

/**
 * InsightResourceRequestHandler
 *
 * @since 2022-07-18
 */
public class InsightResourceRequestHandler extends CefResourceRequestHandlerAdapter {
    private static final LogPrinter LOGGER = LogPrinter.createLogger(InsightResourceRequestHandler.class);
    @Override
    public CefResourceHandler getResourceHandler(CefBrowser browser, CefFrame frame, CefRequest request) {
        String url = request.getURL();
        if (url == null) {
            return null;
        }
        if (url.startsWith(UrlConstants.PROFILER_ORIGIN)) {
            return new InsightResourceHandler(url);
        } else if (url.startsWith(UrlConstants.DEBUG_PROFILER_ORIGIN)) {
            return new InsightDebugResourceHandler(url);
        } else {
            LOGGER.warn("please check UrlConstants!!");
        }
        return null;
    }
}
