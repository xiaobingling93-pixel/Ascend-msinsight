/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
*/
import type { DataRequest, ModuleName, DataSource, ProjectActionEnum } from './websocket/defs';
import { Connection } from '@/centralServer/websocket/connection';
import connector from '@/connection';
import { useLoading } from '@/hooks/useLoading';
import type {DataInfo, TimelineCardInfo} from '@/stores/compareConfig';

export const CONNECTION_MAP: Map<string, Connection> = new Map();

const getConnectionMapKey = (dataSource: DataSource): string => {
    return `${dataSource.remote}:${dataSource.port}`;
};

export const isConnected = (dataSoure: DataSource): boolean => {
    const connection = CONNECTION_MAP.get(getConnectionMapKey(dataSoure));
    return !!connection?.isConnected;
};

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
};

export const isExistedRemote = function(dataSource: DataSource): boolean {
    return CONNECTION_MAP.has(getConnectionMapKey(dataSource));
};

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

export const addDataPath = function(dataSource: DataSource, action: ProjectActionEnum, isConflict: boolean): void {
    const connection = CONNECTION_MAP.get(getConnectionMapKey(dataSource));
    if (connection) {
        useLoading().open({});
        connector.send({
            event: 'remote/import',
            body: { dataSource, isConflict, action },
        });
    }
};

export const sendDaseLineInfo = function(dataSource: DataSource, baseLine: TimelineCardInfo[]): void {
    const connection = CONNECTION_MAP.get(getConnectionMapKey(dataSource));
    if (connection) {
        connector.send({
            event: 'baseline/add',
            body: { dataSource, baseLine },
        });
    }
};

export const removeBaseline = function(dataSource: DataSource, singleDataPath: string): void {
    const connection = CONNECTION_MAP.get(getConnectionMapKey(dataSource));
    if (connection) {
        connector.send({
            event: 'baseline/remove',
            body: { dataSource, singleDataPath },
        });
    }
};

export const deleteDataPath = function(dataSource: DataSource): void {
    const connection = CONNECTION_MAP.get(getConnectionMapKey(dataSource));
    if (connection) {
        connector.send({
            event: 'remote/remove',
            body: { dataSource },
        });
    }
};

export const request = function (
    dataSource: DataSource,
    moduleName: ModuleName,
    args: DataRequest,
    voidResponse?: boolean,
): Promise<unknown> {
    const connection: Connection | undefined = CONNECTION_MAP.get(getConnectionMapKey(dataSource));
    return new Promise((resolve, reject) => connection?.fetch(moduleName, args, voidResponse)?.then(resolve, reject));
};
