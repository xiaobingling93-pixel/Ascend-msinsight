/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import type { DataRequest, ModuleName, DataSource, Host } from './websocket/defs';
import { Connection } from './websocket/connection';
import connector from '@/connection';
import { ProjectAction } from '@/utils/enum';
export const CONNECTION_MAP: Map<string, Connection> = new Map();

const getConnectionMapKey = (dataSource: Host): string => {
    return `${dataSource?.remote}:${dataSource?.port}`;
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

export const isExistedRemote = function(dataSource: DataSource): boolean {
    return CONNECTION_MAP.has(getConnectionMapKey(dataSource));
};

export const addDataPath = function(dataSource: DataSource, action: ProjectAction, isConflict: boolean): void {
    const connection = CONNECTION_MAP.get(getConnectionMapKey(dataSource));
    if (connection) {
        connector.send({
            event: 'remote/import',
            body: { dataSource, isConflict, action },
        });
    }
};

export const request = function (
    dataSource: Host,
    moduleName: ModuleName,
    args: DataRequest,
    voidResponse?: boolean,
): Promise<unknown> {
    const connection: Connection | undefined = CONNECTION_MAP.get(getConnectionMapKey(dataSource));
    return new Promise((resolve, reject) => connection?.fetch(moduleName, args, voidResponse)?.then(resolve, reject));
};
