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
const port: number = Number.parseInt(<string>PARAM_MAP.get('port'));
const jupyterlabProxy: boolean = PARAM_MAP.get('jupyterlabProxy') === 'true';

export const LOCAL_HOST = '127.0.0.1';
export const PORT: number = port;
export const JUPYTERLABPROXY: boolean = jupyterlabProxy;
export const GLOBAL_HOST = { remote: LOCAL_HOST, port: PORT };

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
    fileId?: string; // 传递卡数据库路径 dbPath
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
    message?: string;
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

export interface ConnectHost {
    remote: string;
    port: number;
    jupyterlabProxy?: boolean;
}

export interface FileOrDirectory {
    type?: LayerType;
    name: string;
    path: string;
    rankId?: string; // type === 'RANK' 时有值
    children: FileOrDirectory[];
}

export interface Project extends Pick<FileOrDirectory, 'children'> {
    projectName: string;
    projectPath: string[];
    children: FileOrDirectory[];
    selectedFileType?: LayerType;
    selectedFilePath?: string;
    selectedRankId?: string;
}
export interface DataSource extends ConnectHost, Project {
    isBaseLine?: boolean;
    baseLineCardId?: string;
}
export interface ActiveDataSource extends DataSource {
    selectedFileType: LayerType;
    selectedFilePath: string;
}

export type LayerType = 'PROJECT' | 'CLUSTER' | 'HOST' | 'RANK' | 'COMPUTE' | 'IPYNB' | 'UNKNOWN';

export interface ImportResultBody {
    reset: boolean;
    isPending: boolean;
    isSimulation: boolean;
    isIpynb: boolean;
    isCluster: boolean;
    isMultiDevice: boolean; // 判断是否是单Host多Device项目
    children: ImportTreeInfo[];
}

export interface ImportRankInfo {
    rankId: string;
    filePath: string;
}

export interface ImportTreeInfo {
    type: LayerType;
    filePath: string;
    fileDir: string;
    clusterId?: string;
    host?: string;
    cardName?: string;
    rankId?: string;
    children: ImportTreeInfo[];
}

export interface ProjectDirectory {
    projectName: string;
    projectPath: string[];
    children: ImportTreeInfo[];
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
