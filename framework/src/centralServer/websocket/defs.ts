export const CONTENT_LENGTH_PREFIX = 'Content-Length';
const getParamMap = () => {
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
}
const PARAM_MAP = getParamMap();
export let PORT = Number.parseInt(<string>PARAM_MAP.get('port'));
export function setPort(port: number) {
    PORT = port;
}
export const LOCAL_HOST = '127.0.0.1';

export type DataRequest = {
    command: string;
    params: Request['params'];
};

export type Request = {
    id: number;
    type: string;
    moduleName: string;
    params: Record<string, unknown>;
    command: string;
    token?: string;
};

export type Response<T = Record<string, unknown>> = {
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

export type Notification<T = Record<string, unknown>> = {
    dataSource?: DataSource;
    moduleName: ModuleName;
    event: string;
    body: T;
};

export type ModuleName = string;

export type DataSource = {
    remote: string;
    port: number;
    dataPath: string[];
};

export type ResponseHandler = (res: Response) => void;

export const isResponse = (msg: Response | Notification): msg is Response => {
    return (msg as Response).requestId !== undefined;
};
