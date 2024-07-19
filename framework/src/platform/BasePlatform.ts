/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
export abstract class BasePlatform {
    async convertPath(paths: string[]): Promise<string[]> { return paths; };
    abstract sendMessage(ceq: any): void;
    abstract selectFolder(): Promise<string>;
    abstract selectFile(): Promise<string>;
};

export const removeAndAddEventListener = (
    resolve: (params: any) => void,
): void => {
    function onMessage(event: MessageEvent): void {
        const message = event.data;
        switch (message.command) {
            case 'ascend.folderSelected':
                resolve(message.path);
                break;
            case 'ascend.folderSelectionCanceled':
                resolve('');
                break;
            case 'ascend.uriTransed':
                resolve(message.path);
                break;
            default:
        }
    }
    window.removeEventListener('message', onMessage);
    window.addEventListener('message', onMessage);
};
