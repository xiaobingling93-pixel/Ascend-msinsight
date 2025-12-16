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
import type { InsightState, NavigationParam, NotifyLevel, Platform } from './platform';
import type { IMessageSender } from '../connection/messageSender';

export class Browser implements Platform, IMessageSender {
    isUltimateEdition: Promise<boolean> = Promise.resolve(false);

    trace = (action: string, traceInfo: object): void => {
        // do nothing
    };

    initTheme = async (): Promise<ThemeItem> => {
        // do nothing
        return 'dark';
    };

    notify = (message: string, level?: NotifyLevel): void => {
        // do nothing
    };

    dialog = (message: string, needCancel = false, onSuccess?: () => void, onFail?: () => void): void => {

    };

    jumpToSource = (param: NavigationParam): void => {
        // do nothing
    };

    setDeviceCpuAbi = (abi: string): void => {
        // do nothing
    };

    setDeviceTime = (sessionId: string, deviceKey: string, category: string): Promise<string> => {
        // do nothing
        return Promise.resolve('');
    };

    setInsightState = (state: InsightState): void => {
        // do nothing
    };

    getUUID = (): Promise<string> => {
        // do nothing
        return Promise.resolve('');
    };

    initSession = (sessionId: string): Promise<string> => Promise.resolve('');

    openFrameworkUrl = (): void => {
    };

    selectFolder = (): Promise<string> => new Promise(resolve => {
        resolve('browser');
    });

    selectFile = (): Promise<string> => this.selectFolder();

    sendMessage = (): void => {
    };
}
