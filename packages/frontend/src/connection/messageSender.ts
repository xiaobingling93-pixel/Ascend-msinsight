import { BrowserPlatform, IntellijPlatform, VsCodePlatform } from '../platforms/platform';

declare function acquireVsCodeApi(): any;

export let messageSender: IMessageSender = new BrowserPlatform();

if (typeof acquireVsCodeApi === 'function') {
    messageSender = new VsCodePlatform();
    messageSender.sendMessage = acquireVsCodeApi().postMessage;
}

if (typeof window.cefQuery === 'function') {
    messageSender = new IntellijPlatform();
}

export interface IMessageSender {
    sendMessage: (ceq: any) => void;
    selectFolder: () => Promise<string>;
}

export const removeAndAddEventListener = (resolve: (value: (string | PromiseLike<string>)) => void): void => {
    function onMessage(event: MessageEvent): void {
        const message = event.data;
        if (message.command === 'ascend.folderSelected') {
            resolve(message.path);
        } else if (message.command === 'ascend.folderSelectionCanceled') {
            resolve('');
        }
    }
    window.removeEventListener('message', onMessage);
    window.addEventListener('message', onMessage);
};
