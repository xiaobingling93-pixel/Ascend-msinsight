/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 */

package com.huawei.ascend.insight.data;

import com.intellij.openapi.components.PersistentStateComponent;
import com.intellij.openapi.application.Application;
import com.intellij.openapi.application.ApplicationManager;
import com.intellij.openapi.components.State;
import com.intellij.openapi.components.Storage;
import com.intellij.util.xmlb.XmlSerializerUtil;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * InsightCfgPreferences
 *
 * @since 2011-11-01
 */
@State(name = "InsightCfgPreferences", storages = {@Storage("insightConfig.xml")})
public class InsightCfgPreferences implements PersistentStateComponent<InsightCfgPreferences> {
    private static volatile InsightCfgPreferences insightCfgPreferences = null;

    private String exportPath = "";
    private String importPath = "";

    private InsightCfgPreferences() {};

    /**
     * get instance
     *
     * @return InsightCfgPreferences instance
     */
    public static InsightCfgPreferences getInstance() {
        if (insightCfgPreferences == null) {
            synchronized (InsightCfgPreferences.class) {
                if (insightCfgPreferences == null) {
                    Application application = ApplicationManager.getApplication();
                    insightCfgPreferences = application.getService(InsightCfgPreferences.class);
                }
            }
        }
        return insightCfgPreferences;
    }

    /**
     * 获取用户导出session的目录
     *
     * @return 用户导出session的目录
     */
    public String getExportPath() {
        return exportPath;
    }

    /**
     * 设置用户导出session的目录
     *
     * @param exportPath 用户导出session的目录
     */
    public void setExportPath(String exportPath) {
        this.exportPath = exportPath;
    }

    /**
     * 获取用户导入session的目录
     *
     * @return 用户导入session的目录
     */
    public String getImportPath() {
        return importPath;
    }

    /**
     * 设置用户导入session的目录
     *
     * @param importPath 用户导入session的目录
     */
    public void setImportPath(String importPath) {
        this.importPath = importPath;
    }

    @Override
    @Nullable
    public InsightCfgPreferences getState() {
        return this;
    }

    @Override
    public void loadState(@NotNull InsightCfgPreferences state) {
        XmlSerializerUtil.copyBean(state, this);
    }
}
