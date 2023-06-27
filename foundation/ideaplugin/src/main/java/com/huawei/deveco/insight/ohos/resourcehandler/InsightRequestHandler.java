/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.deveco.insight.ohos.resourcehandler;

import org.cef.browser.CefBrowser;
import org.cef.browser.CefFrame;
import org.cef.handler.CefRequestHandlerAdapter;
import org.cef.handler.CefResourceRequestHandler;
import org.cef.misc.BoolRef;
import org.cef.network.CefRequest;

/**
 * InsightRequestHandler
 *
 * @since 2022-07-18
 */
public class InsightRequestHandler extends CefRequestHandlerAdapter {
    private static final InsightResourceRequestHandler HANDLER = new InsightResourceRequestHandler();

    @Override
    public CefResourceRequestHandler getResourceRequestHandler(CefBrowser browser, CefFrame frame, CefRequest request,
        boolean isNavigation, boolean isDownload, String requestInitiator, BoolRef disableDefaultHandling) {
        return HANDLER;
    }
}
