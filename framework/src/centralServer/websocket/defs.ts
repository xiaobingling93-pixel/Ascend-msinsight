/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
export const CONTENT_LENGTH_PREFIX = 'Content-Length';
const getParamMap = (): Map<string, string> => {
    const paramMap: Map<string, string> = new Map();
    if (window.location.search !== '') {
        const params = window.location.search.slice(1).split('&');
        params.forEach(param => {
            const entry = param.split('=');
            if (entry.length === 2) {
                paramMap.set(entry[0], entry[1]);
            }
        });
    }
    if (!paramMap.has('port')) {
        paramMap.set('port', '9000');
    }
    return paramMap;
};
const PARAM_MAP = getParamMap();
let port:number = Number.parseInt(<string>PARAM_MAP.get('port'));
export function setPort(newPort: number): void {
    port = newPort;
}
export const LOCAL_HOST = '127.0.0.1';
export const PORT:number = port;

export interface DataRequest {
    command: string;
    projectName?: string;
    params?: Request['params'];
};

export interface Request {
    id: number;
    type: string;
    moduleName: string;
    params?: Record<string, unknown>;
    command: string;
    projectName?: string;
    token?: string;
};

export interface Response<T = Record<string, unknown>> {
    type: string;
    requestId: number;
    id?: number;
    result?: boolean;
    body?: T;
    command?: string;
    error?: {
        code: number;
        message: string;
    };
};

export interface Notification<T = Record<string, unknown>> {
    dataSource?: DataSource;
    moduleName: ModuleName;
    event: string;
    body: T;
};

export type ModuleName = string;

export interface DataSource {
    remote: string;
    port: number;
    projectName: string;
    dataPath: string[];
    isBaseLine?: boolean;
    baseLineCardId?: string;
};

export interface ProjectDirectory {
    projectName: string;
    fileName: string[];
};

export enum ProjectActionEnum {
    TRANSFER_PROJECT = 0,
    ADD_FILE = 1,
};

export type ResponseHandler = (res: Response) => void;

export const isResponse = (msg: Response | Notification): msg is Response => {
    return (msg as Response).requestId !== undefined;
};
