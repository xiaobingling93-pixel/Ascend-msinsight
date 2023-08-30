import { IMessageSender } from './messageSender';

export class Browser implements IMessageSender {
    selectFolder = (): Promise<string> =>
        new Promise((resolve) => {
            resolve('browser');
        });

    selectFile = (): Promise<string> => this.selectFolder();

    sendMessage = (): void => {};
}
