/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 */

package com.huawei.ascend.insight.handlers;

import com.alibaba.fastjson.JSONObject;
import com.google.gson.Gson;
import com.huawei.ascend.insight.CefQueryHandler;
import com.huawei.ascend.insight.common.Response;
import com.huawei.ascend.insight.processor.SelectProcessor;
import com.huawei.ascend.insight.utils.LogPrinter;
import com.intellij.openapi.application.ApplicationManager;
import org.cef.callback.CefQueryCallback;
import org.jetbrains.annotations.NotNull;

import java.util.Map;
import java.util.function.Function;

/**
 * Handler for selectFolder
 *
 * @since 2022-10-15
 */
public class SelectFolderHandler implements CefQueryHandler {
    private static final LogPrinter LOGGER = LogPrinter.createLogger(SelectFolderHandler.class);

    private static final Map<String, Function<JSONObject, Response<?>>> IMPORT_FUNC_MAP = Map.of(
            "ascend.selectFolder", SelectProcessor::selectFolder,
            "ascend.selectFile", SelectProcessor::selectFile
    );
    @Override
    public void onQuery(@NotNull String method, @NotNull JSONObject params, @NotNull CefQueryCallback callback) {
        ApplicationManager.getApplication().executeOnPooledThread(() -> {
            Response<?> response = IMPORT_FUNC_MAP.get(method).apply(params);
            callback.success(new Gson().toJson(response));
        });
    }
}