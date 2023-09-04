import { Browser } from './Browser';
import { IntellijPlatform } from './IntellijPlatform';
import { VsCodePlatform } from './VsCodePlatform';

declare function acquireVsCodeApi(): any;

export let MESSAGE_SENDER: IMessageSender = new Browser();

if (typeof acquireVsCodeApi === 'function') {
    MESSAGE_SENDER = new VsCodePlatform();
    MESSAGE_SENDER.sendMessage = acquireVsCodeApi().postMessage;
}

if (typeof window.cefQuery === 'function') {
    MESSAGE_SENDER = new IntellijPlatform();
}

export interface IMessageSender {
    sendMessage: (ceq: any) => void;
    selectFolder: () => Promise<string>;
    selectFile: () => Promise<string>;
}

export const removeAndAddEventListener = (
    resolve: (value: string | PromiseLike<string>) => void,
): void => {
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
