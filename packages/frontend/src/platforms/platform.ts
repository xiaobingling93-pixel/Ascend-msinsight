import { ThemeItem } from '../theme/theme';
import { IMessageSender, removeAndAddEventListener } from '../connection/messageSender';

export type NotifyLevel = 'info' | 'warn' | 'error';

export type NavigationParam = { uri: string } & ({
    type: 'js';
    line: number;
    col: number;
} | {
    type: 'native';
    offset: number;
});

export type InsightState = 'running' | 'stop';

export interface Platform {
    trace: (action: string, traceInfo: object) => void;
    initTheme: () => Promise<ThemeItem>;
    notify: (message: string, level?: NotifyLevel) => void;
    dialog: (message: string, needCancel?: boolean, onSuccess?: () => void, onFail?: () => void) => void;
    jumpToSource: (param: NavigationParam) => void;
    setDeviceCpuAbi: (abi: string) => void;
    setDeviceTime: (sessionId: string, deviceKey: string, category: string) => Promise<string>;
    setInsightState: (state: InsightState) => void;
    getUUID: () => Promise<string>;
    initSession: (sessionId: string) => Promise<string>;
    openFrameworkUrl: () => void;
    isUltimateEdition: Promise<boolean>;
}

export class IntellijPlatform implements IMessageSender {
    selectFolder(): Promise<string> {
        return new Promise(resolve => {
            this.sendMessage({
                request: JSON.stringify({
                    key: 'ascend.selectFolder',
                    data: {
                        method: 'ascend.selectFolder',
                        params: {},
                    },
                }),
                onSuccess: function (response: string) {
                    resolve(JSON.parse(response).body);
                },
            });
        });
    }

    sendMessage(ceq: any): void {
        window.cefQuery(ceq);
    }
}

export class VsCodePlatform implements IMessageSender {
    selectFolder(): Promise<string> {
        return new Promise(resolve => {
            this.sendMessage({ command: 'ascend.selectFolder' });
            removeAndAddEventListener(resolve);
        });
    }

    sendMessage(ceq: any): void {
    }
}

export class BrowserPlatform implements IMessageSender {
    selectFolder(): Promise<string> {
        return new Promise(resolve => {
            resolve('browser');
        });
    }

    sendMessage(): void {
    }
}
