/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.ascend.insight.utils;

import org.cef.browser.CefBrowser;
import org.cef.browser.CefFrame;
import org.cef.callback.CefQueryCallback;
import org.cef.handler.CefMessageRouterHandler;
import org.cef.handler.CefMessageRouterHandlerAdapter;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * CefMessageRouterProxy
 *
 * @since 2023/8/1
 */
public class CefMessageRouterProxy extends CefMessageRouterHandlerAdapter {
    private static volatile CefMessageRouterProxy routerProxy = null;

    private final Map<CefBrowser, CefMessageRouterHandler> routerHandlers = new ConcurrentHashMap<>();

    private CefMessageRouterProxy() {
    }

    /**
     * getInstance
     *
     * @return CefMessageRouterProxy
     */
    public static CefMessageRouterProxy getInstance() {
        if (routerProxy == null) {
            synchronized (CefMessageRouterProxy.class) {
                if (routerProxy == null) {
                    routerProxy = new CefMessageRouterProxy();
                }
            }
        }
        return routerProxy;
    }

    /**
     * onQuery
     *
     * @param browser browser
     * @param frame frame
     * @param queryId queryId
     * @param request request
     * @param isPersistent persistent
     * @param callback callback
     * @return is success
     */
    public boolean onQuery(CefBrowser browser, CefFrame frame, long queryId, String request, boolean isPersistent,
        CefQueryCallback callback) {
        CefMessageRouterHandler messageRouterHandler = this.routerHandlers.get(browser);
        return messageRouterHandler == null
            || messageRouterHandler.onQuery(browser, frame, queryId, request, isPersistent, callback);
    }

    /**
     * onQueryCanceled
     *
     * @param browser browser
     * @param frame frame
     * @param queryId queryId
     */
    public void onQueryCanceled(CefBrowser browser, CefFrame frame, long queryId) {
        CefMessageRouterHandler messageRouterHandler = this.routerHandlers.get(browser);
        if (messageRouterHandler != null) {
            messageRouterHandler.onQueryCanceled(browser, frame, queryId);
        }
    }

    /**
     * putCefRouter
     *
     * @param browser browser
     * @param cefMessageRouterHandler cefMessageRouterHandler
     * @return CefMessageRouterHandler CefMessageRouterHandler
     */
    public CefMessageRouterHandler putCefRouter(CefBrowser browser, CefMessageRouterHandler cefMessageRouterHandler) {
        return this.routerHandlers.putIfAbsent(browser, cefMessageRouterHandler);
    }

    /**
     * removeRouter
     *
     * @param browser browser
     * @return CefMessageRouterHandler CefMessageRouterHandler
     */
    public CefMessageRouterHandler removeRouter(CefBrowser browser) {
        return this.routerHandlers.remove(browser);
    }
}
