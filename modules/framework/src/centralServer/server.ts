/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
*/
import type { DataRequest, ModuleName, DataSource } from './websocket/defs';
import { Connection } from './websocket/connection';
export const CONNECTION_MAP: Map<string, Connection> = new Map();

const getConnectionMapKey = (dataSource: DataSource): string => {
    return `${dataSource.remote}:${dataSource.port}`;
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

export const request = function (
    dataSource: DataSource,
    moduleName: ModuleName,
    args: DataRequest,
    voidResponse?: boolean,
): Promise<unknown> {
    const connection: Connection | undefined = CONNECTION_MAP.get(getConnectionMapKey(dataSource));
    return new Promise((resolve, reject) => connection?.fetch(moduleName, args, voidResponse)?.then(resolve, reject));
};
