/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

import type { ThemeItem } from 'ascend-theme';
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
