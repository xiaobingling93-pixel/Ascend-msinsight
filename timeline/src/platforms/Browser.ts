import { ThemeItem } from '../theme/theme';
import { InsightState, NavigationParam, NotifyLevel, Platform } from './platform';
import { IMessageSender } from '../connection/messageSender';

export class Browser implements Platform, IMessageSender {
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

    isUltimateEdition: Promise<boolean> = new Promise<boolean>(() => false);

    selectFolder = (): Promise<string> => new Promise(resolve => {
        resolve('browser');
    });

    selectFile = (): Promise<string> => this.selectFolder();

    sendMessage = (): void => {
    };
}
