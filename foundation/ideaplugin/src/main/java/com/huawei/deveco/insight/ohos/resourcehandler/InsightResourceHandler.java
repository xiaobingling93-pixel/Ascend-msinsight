/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.deveco.insight.ohos.resourcehandler;

import com.huawei.deveco.insight.ohos.common.constant.URLConstants;
import com.huawei.deveco.insight.ohos.utils.LogPrinter;

import org.cef.callback.CefCallback;
import org.cef.handler.CefResourceHandlerAdapter;
import org.cef.misc.IntRef;
import org.cef.misc.StringRef;
import org.cef.network.CefRequest;
import org.cef.network.CefResponse;

import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;
import java.util.HashMap;
import java.util.Map;

/**
 * InsightResourceHandler
 *
 * @since 2022/04/02
 */
public class InsightResourceHandler extends CefResourceHandlerAdapter {
    private static final LogPrinter LOGGER = LogPrinter.createLogger(InsightResourceHandler.class);

    private static final Map<String, String> EXT_2_TYPE = Map.of("css",   // css
            "text/css",   // css
            "js", "text/javascript",  // js
            "html", "text/html",  // html
            "png", "image/png",  // png
            "woff2", "application/octet-stream",  // woff2 font
            "wasm", "application/wasm",  // wasm
            "mjs", "text/javascript",   // wasm
            "map", "application/json" // map
    );

    /**
     * requestUrl with params
     */
    protected final String requestUrl;

    /**
     * requestUrl without params
     */
    protected final String requestPath;

    /**
     * input stream from source
     */
    protected InputStream inputStream;

    /**
     * current connection
     */
    protected URLConnection connection;

    /**
     * handle requestUrl
     *
     * @param requestUrl requestUrl
     */
    public InsightResourceHandler(String requestUrl) {
        this.requestUrl = requestUrl;
        var paramIndex = requestUrl.indexOf("?");
        if (paramIndex == -1) {
            this.requestPath = this.requestUrl;
        } else {
            this.requestPath = requestUrl.substring(0, paramIndex);
        }
    }

    @Override
    public boolean processRequest(CefRequest request, CefCallback callback) {
        if (!requestUrl.equals(request.getURL())) {
            throw new IllegalStateException();
        }
        if (requestPath == null || requestPath.length() < URLConstants.PROFILER_URL_PREFIX.length()) {
            return false;
        }
        String transformedUrl = requestPath.substring(URLConstants.PROFILER_URL_PREFIX.length())
                .replaceFirst(URLConstants.DOMAIN_NAME, URLConstants.RESOURCE_DIR);
        URL resourceUrl = getClass().getClassLoader().getResource(transformedUrl);
        if (resourceUrl == null) {
            LOGGER.warn("Failed to load resource {}", requestUrl);
            return false;
        }
        try {
            connection = resourceUrl.openConnection();
            inputStream = connection.getInputStream();
        } catch (IOException e) {
            LOGGER.warn("Failed to load resource {}", requestUrl);
            return false;
        }
        callback.Continue();
        return true;
    }

    @Override
    public void getResponseHeaders(CefResponse response, IntRef responseLength, StringRef redirectUrl) {
        if (inputStream == null) {
            response.setStatus(404);
            return;
        }
        for (var entry : EXT_2_TYPE.entrySet()) {
            if (requestPath.endsWith(entry.getKey())) {
                response.setMimeType(entry.getValue());
                break;
            }
        }

        if (connection != null) {
            responseLength.set(connection.getContentLength());
        } else {
            responseLength.set(0);
        }

        Map<String, String> headerMap = new HashMap<>();
        response.getHeaderMap(headerMap);
        headerMap.put("Cross-Origin-Opener-Policy", "same-origin");
        headerMap.put("Cross-Origin-Embedder-Policy", "require-corp");
        headerMap.remove("Access-Control-Allow-Origin");
        headerMap.remove("Access-Control-Allow-Methods");
        response.setHeaderMap(headerMap);

        response.setStatus(200);
    }

    @Override
    public boolean readResponse(byte[] dataOut, int bytesToRead, IntRef bytesRead, CefCallback callback) {
        try {
            int availableSize = inputStream.available();
            if (availableSize > 0) {
                int maxBytesToRead = Math.min(availableSize, bytesToRead);
                int actualBytesRead = inputStream.read(dataOut, 0, maxBytesToRead);
                bytesRead.set(actualBytesRead);
                return true;
            } else {
                inputStream.close();
                return false;
            }
        } catch (IOException e) {
            LOGGER.warn("Failed to read response");
        }
        bytesRead.set(0);
        try {
            inputStream.close();
        } catch (IOException e) {
            LOGGER.error("Failed to read response, inputStream.close() IOException occurred");
        }
        return false;
    }

    @Override
    public void cancel() {
        try {
            inputStream.close();
        } catch (IOException e) {
            LOGGER.warn("Failed close connection");
        }
    }
}
