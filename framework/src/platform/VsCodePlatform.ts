import { type IMessageSender, removeAndAddEventListener } from './messageSender';

export class VsCodePlatform implements IMessageSender {
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

    sendMessage = (ceq: any): void => {};
}
