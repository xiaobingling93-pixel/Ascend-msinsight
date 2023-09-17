export const CONTENT_LENGTH_PREFIX = 'Content-Length';
export const PORT = 9000;
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

export type NotificationRegistration = { moduleName: ModuleName; callBack: Function };

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
