export abstract class BasePlatform {
    abstract sendMessage(ceq: any): void;
    abstract selectFolder(): Promise<string>;
    async convertPath(paths: string[]): Promise<string[]> { return paths; };
    abstract selectFile(): Promise<string>;
};

export const removeAndAddEventListener = (
    resolve: Function,
): void => {
    function onMessage(event: MessageEvent): void {
        const message = event.data;
        if (message.command === 'ascend.folderSelected') {
            resolve(message.path);
        } else if (message.command === 'ascend.folderSelectionCanceled') {
            resolve('');
        } else if (message.command === 'ascend.uriTransed') {
            resolve(message.path);
        }
    }
    window.removeEventListener('message', onMessage);
    window.addEventListener('message', onMessage);
};
