/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 */

package com.huawei.ascend.insight.utils;

import static com.huawei.ascend.insight.common.constant.Constant.INSIGHT_NOTIFY;

import java.util.Map;

import com.huawei.ascend.insight.InsightWindowFactory;
import com.intellij.notification.NotificationGroupManager;
import com.intellij.notification.NotificationType;
import org.apache.commons.lang.StringUtils;

/**
 * LogNotification
 *
 * @since 2022-3-21
 */
public class BalloonNotification {
    private static final Map<String, NotificationType> NOTIFICATION_TYPE_MAP = Map.of(
        "error", NotificationType.ERROR,
        "warn", NotificationType.WARNING,
        "info", NotificationType.INFORMATION,
        "ideUpdate", NotificationType.IDE_UPDATE
    );

    /**
     * show notification, title is about this plugin
     *
     * @param message message
     * @param type type
     */
    public static void show(String message, NotificationType type) {
        if (StringUtils.isNotBlank(message)) {
            NotificationGroupManager.getInstance()
                .getNotificationGroup(INSIGHT_NOTIFY)
                .createNotification(message, type)
                .notify(InsightWindowFactory.getProject());
        }
    }

    /**
     * show notification, title is about this plugin
     *
     * @param message message
     * @param level level
     */
    public static void show(String message, String level) {
        NotificationType notificationType = NOTIFICATION_TYPE_MAP.get(level);
        show(message, notificationType);
    }
}