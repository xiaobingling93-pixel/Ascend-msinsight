/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 */

package com.huawei.ascend.insight.common;

import com.intellij.openapi.project.Project;

/**
 * Context information of project
 *
 * @since 2022-10-20
 */
public class ProjectContext {
    private static Project sProject;

    public static Project getProject() {
        return sProject;
    }

    public static void setProject(Project project) {
        sProject = project;
    }
}
