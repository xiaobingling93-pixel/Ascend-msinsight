/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.deveco.insight.ohos.ui;

import com.intellij.openapi.ui.DialogWrapper;

import org.jetbrains.annotations.Nullable;

import java.awt.Dimension;
import java.awt.FlowLayout;

import javax.swing.Action;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JPanel;

/**
 * MsgDialogWrapper
 *
 * @since 2022-09-01
 */
public class MsgDialogWrapper extends DialogWrapper {
    private final String msg;
    private final boolean isNeedCancel;

    /**
     * MsgDialogWrapper
     *
     * @param title title
     * @param msg msg
     * @param canNeedCancel canNeedCancel
     */
    public MsgDialogWrapper(String title, String msg, boolean canNeedCancel) {
        super(false);
        setTitle(title);
        this.msg = msg;
        this.isNeedCancel = canNeedCancel;
        init();
    }

    @Nullable
    @Override
    protected JComponent createCenterPanel() {
        JPanel dialogPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        dialogPanel.setPreferredSize(new Dimension(400, 80));
        JLabel label = new JLabel();
        label.setText(msg);
        JPanel contentPanel = new JPanel();
        contentPanel.add(label);
        dialogPanel.add(contentPanel);

        return dialogPanel;
    }
    @Override
    protected Action [] createActions() {
        if (!this.isNeedCancel) {
            return new Action[]{getOKAction()};
        }
        return super.createActions();
    }

    /**
     * createActionByTag
     */
    public void createActionByTag() {
        createActions();
    }
}
