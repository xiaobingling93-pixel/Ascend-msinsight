import type { DataRequest, ModuleName, NotificationRegistration, DataSource } from './websocket/defs';
import { Connection } from '@/centralServer/websocket/connection';

export const CONNECTION_MAP: Map<string, Connection> = new Map();

export const NOTIFICATION_METHOD_MAP: Map<ModuleName, Function> = new Map();

export const registerNotification = function (notificationRegistration: NotificationRegistration) {
    NOTIFICATION_METHOD_MAP.set(
        notificationRegistration.moduleName,
        notificationRegistration.callBack,
    );
};

const getConnectionMapKey = (dataSource: DataSource): string => {
    return `${dataSource.remote}:${dataSource.port}`;
}

export const disconnectRemote = function (dataSource: DataSource): boolean {
    const connection: Connection | undefined = CONNECTION_MAP.get(getConnectionMapKey(dataSource));
    try {
        connection?.disconnect();
        CONNECTION_MAP.delete(getConnectionMapKey(dataSource));
    } catch (e) {
        console.warn(e);
        return false;
    }
    return true;
}

export const isExistedRemote = function(dataSource: DataSource): boolean {
    return CONNECTION_MAP.has(getConnectionMapKey(dataSource));
}

export const connectRemote = async function (dataSource: DataSource): Promise<boolean> {
    const connection = new Connection(dataSource);
    try {
        await connection.connect();
    } catch (e) {
        return false;
    }
    CONNECTION_MAP.set(getConnectionMapKey(dataSource), connection);
    return true;
};

export const addDataPath = function(dataSource: DataSource): void {
    const connection = CONNECTION_MAP.get(getConnectionMapKey(dataSource));
    connection?.addDataPath(dataSource.dataPath);
}

export const deleteDataPath = function(dataSource: DataSource): void {
    const connection = CONNECTION_MAP.get(getConnectionMapKey(dataSource));
    connection?.deleteDataPath(dataSource.dataPath);
}

export const request = function (
    dataSource: DataSource,
    moduleName: ModuleName,
    args: DataRequest,
): Promise<unknown> {
    const connection: Connection | undefined = CONNECTION_MAP.get(getConnectionMapKey(dataSource));
    return new Promise((resolve, reject) => connection?.fetch(moduleName, args)?.then(resolve, reject));
};
