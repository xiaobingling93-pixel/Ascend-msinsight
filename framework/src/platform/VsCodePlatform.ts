/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
*/
import { BasePlatform, removeAndAddEventListener } from './BasePlatform';

export class VsCodePlatform extends BasePlatform {
    selectFolder(): Promise<string> {
        return new Promise((resolve) => {
            this.sendMessage({ command: 'ascend.selectFolder' });
            removeAndAddEventListener(resolve);
        });
    }

    selectFile(): Promise<string> {
        return new Promise((resolve) => {
            this.sendMessage({ command: 'ascend.selectFile' });
            removeAndAddEventListener(resolve);
        });
    }

    convertPath(paths: string[]): Promise<string[]> {
        return new Promise((resolve) => {
            this.sendMessage({ command: 'ascend.transUri', body: paths.map(this.getAbsolutePath) });
            removeAndAddEventListener(resolve);
        });
    }

    sendMessage = (ceq: any): void => {};

    private getAbsolutePath(path: string): string {
        return import.meta.url.replace('/index.html', path.replace('.', ''));
    }
}
