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
import type { ThemeItem } from '@insight/lib/theme';

export type NotifyLevel = 'info' | 'warn' | 'error';

export type NavigationParam = { uri: string } & ({
    type: 'js';
    line: number;
    col: number;
} | {
    type: 'native';
    offset: number;
});

export type InsightState = 'running' | 'stop';

export interface Platform {
    trace: (action: string, traceInfo: object) => void;
    initTheme: () => Promise<ThemeItem>;
    notify: (message: string, level?: NotifyLevel) => void;
    dialog: (message: string, needCancel?: boolean, onSuccess?: () => void, onFail?: () => void) => void;
    jumpToSource: (param: NavigationParam) => void;
    setDeviceCpuAbi: (abi: string) => void;
    setDeviceTime: (sessionId: string, deviceKey: string, category: string) => Promise<string>;
    setInsightState: (state: InsightState) => void;
    getUUID: () => Promise<string>;
    initSession: (sessionId: string) => Promise<string>;
    openFrameworkUrl: () => void;
    isUltimateEdition: Promise<boolean>;
}
