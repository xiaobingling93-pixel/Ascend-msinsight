/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */
import { NotificationEvent, NotificationHandler } from '@/connection/defs';
import { connector } from '@/connection';

function isValidPayload(payload: unknown): payload is Record<string, unknown> {
    return typeof payload === 'object' && payload !== null;
}

export function registerMessageListeners(handlers: Record<string, NotificationHandler>): void {
    Object.entries(handlers).forEach(([event, handler]) => {
        connector.addListener(event, (e: MessageEvent<NotificationEvent>) => {
            const { body } = e.data;

            if (!isValidPayload(body)) return;
            handler(body);
        });
    });
}
