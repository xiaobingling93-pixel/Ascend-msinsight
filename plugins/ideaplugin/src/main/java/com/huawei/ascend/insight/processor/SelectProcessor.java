/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 */

package com.huawei.ascend.insight.processor;

import com.huawei.ascend.insight.common.ProjectContext;
import com.huawei.ascend.insight.common.Response;

import com.alibaba.fastjson.JSONObject;

import com.intellij.openapi.application.ApplicationManager;
import com.intellij.openapi.fileChooser.FileChooser;
import com.intellij.openapi.fileChooser.FileChooserDescriptor;
import com.intellij.openapi.project.Project;
import com.intellij.openapi.vfs.VirtualFile;

import java.util.concurrent.CompletableFuture;

/**
 * SelectProcessor
 *
 * @since 2023-08-14
 */
public class SelectProcessor {
    /**
     * select Folder
     *
     * @param params JSONObject parameters
     * @return Response<?>
     */
    public static Response<?> selectFolder(JSONObject params) {
        CompletableFuture<String> future = openFolderChooserAndGetPath(ProjectContext.getProject(), false, true);
        String folderPath = future.join();
        return Response.success(folderPath);
    }

    /**
     * select Folder
     *
     * @param params JSONObject parameters
     * @return Response<?>
     */
    public static Response<?> selectFile(JSONObject params) {
        CompletableFuture<String> future = openFolderChooserAndGetPath(ProjectContext.getProject(), true, false);
        String folderPath = future.join();
        return Response.success(folderPath);
    }

    /**
     * open Folder Chooser And Get Path
     *
     * @param project project
     * @param isChooseFiles isChooseFiles
     * @param isChooseFolders isChooseFiles
     * @return CompletableFuture<String>
     */
    public static CompletableFuture<String> openFolderChooserAndGetPath(Project project,
        Boolean isChooseFiles, Boolean isChooseFolders) {
        CompletableFuture<String> future = new CompletableFuture<>();
        ApplicationManager.getApplication().invokeLater(() -> {
            FileChooserDescriptor descriptor = new FileChooserDescriptor(isChooseFiles,
            isChooseFolders, false, false, false, false) {
                @Override
                public boolean isFileVisible(VirtualFile file, boolean isShowHiddenFiles) {
                    return file.isDirectory() || "json".equals(file.getExtension());
                }
            };
            VirtualFile[] virtualFiles = FileChooser.chooseFiles(descriptor, project, null);
            String path = virtualFiles.length > 0 ? virtualFiles[0].getPath() : "";
            future.complete(path);
        });
        return future;
    }
}
