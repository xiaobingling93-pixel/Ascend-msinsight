import type { DataRequest, ModuleName, NotificationRegistration, Remote } from './websocket/defs';
import { Connection } from '@/centralServer/websocket/connection';

export const CONNECTION_MAP: Map<string, Connection> = new Map();

export const NOTIFICATION_METHOD_MAP: Map<ModuleName, Function> = new Map();

export const registerNotification = function (notificationRegistration: NotificationRegistration) {
    NOTIFICATION_METHOD_MAP.set(
        notificationRegistration.moduleName,
        notificationRegistration.callBack,
    );
};

export const connectRemote = async function (remote: Remote): Promise<boolean> {
    const connection = new Connection(remote.remote);
    try {
        await connection.connect();
    } catch (e) {
        return false;
    }
    CONNECTION_MAP.set(remote.remote, connection);
    const iframe = document.querySelector('iframe') as HTMLIFrameElement;
    iframe.contentWindow?.postMessage(
        {
            event: 'remote/import',
            remote,
            body: '',
        },
        '*',
    );
    return true;
};

export const request = function (
    remote: Remote,
    moduleName: ModuleName,
    args: DataRequest,
): Promise<unknown> {
    const connection: Connection | undefined = CONNECTION_MAP.get(remote.remote);
    return new Promise((resolve, reject) => connection?.fetch(moduleName, args)?.then(resolve, reject));
};
