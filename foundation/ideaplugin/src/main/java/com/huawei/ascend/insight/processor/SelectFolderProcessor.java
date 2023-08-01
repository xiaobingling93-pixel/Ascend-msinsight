/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 */

package com.huawei.ascend.insight.processor;

import com.alibaba.fastjson.JSONObject;
import com.huawei.ascend.insight.common.ProjectContext;
import com.huawei.ascend.insight.common.Response;
import com.intellij.openapi.application.ApplicationManager;
import com.intellij.openapi.fileChooser.FileChooser;
import com.intellij.openapi.fileChooser.FileChooserDescriptor;
import com.intellij.openapi.project.Project;
import com.intellij.openapi.vfs.VirtualFile;

import java.util.concurrent.CompletableFuture;

public class SelectFolderProcessor {
    /**
     * select Folder
     *
     * @param params JSONObject parameters
     * @return Response<?>
     */
    public static Response<?> selectFolder(JSONObject params) {
        CompletableFuture<String> future = openFolderChooserAndGetPath(ProjectContext.getProject());
        String folderPath = future.join();
        return Response.success(folderPath);
    }

    public static CompletableFuture<String> openFolderChooserAndGetPath(Project project) {
        CompletableFuture<String> future = new CompletableFuture<>();
        ApplicationManager.getApplication().invokeLater(() -> {
            FileChooserDescriptor descriptor = new FileChooserDescriptor(false, true, false, false, false, false);
            VirtualFile[] virtualFiles = FileChooser.chooseFiles(descriptor, project, null);
            String path = virtualFiles.length > 0 ? virtualFiles[0].getPath() : "";
            future.complete(path);
        });
        return future;
    }
}
