/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.ascend.insight.ui;

import java.awt.Color;
import java.awt.Cursor;
import java.awt.Font;
import java.awt.font.TextAttribute;
import java.util.Map;
import java.util.HashMap;

import javax.swing.JLabel;
import javax.swing.JPanel;

/**
 * 默认提示
 *
 * @since 2023/02/06
 */
public class MessagePanel {
    private JPanel rootPanel;
    private JLabel messageLabel;

    /**
     * 规则标题界面
     *
     */
    public MessagePanel() {
        Font font = messageLabel.getFont();
        Map<TextAttribute, ?> temp = font.getAttributes();
        Map<TextAttribute, Object> attributes = new HashMap<>();
        for (Map.Entry<TextAttribute, ?> entry : temp.entrySet()) {
            attributes.put(entry.getKey(), entry.getValue());
        }
        attributes.put(TextAttribute.UNDERLINE, TextAttribute.UNDERLINE_ON);
        attributes.put(TextAttribute.FOREGROUND, new Color(82, 145, 255));
        attributes.put(TextAttribute.SIZE, 14);
        messageLabel.setFont(new Font(attributes));
        messageLabel.setCursor(Cursor.getPredefinedCursor(Cursor.HAND_CURSOR));
    }

    public JPanel getRootPanel() {
        return rootPanel;
    }

    public void setRootPanel(JPanel rootPanel) {
        this.rootPanel = rootPanel;
    }

    public JLabel getMessageLabel() {
        return messageLabel;
    }

    public void setMessageLabel(JLabel messageLabel) {
        this.messageLabel = messageLabel;
    }
}

