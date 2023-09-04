export const CONTENT_LENGTH_PREFIX = 'Content-Length';
export const PORT = 9000;

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
    remote?: string;
    moduleName: ModuleName;
    event: string;
    body: T;
};

export type NotificationRegistration = { moduleName: ModuleName; callBack: Function };

export type ModuleName = string;

export type Remote = {
    remote: string;
    dataPath: string[];
};

export type ResponseHandler = (res: Response) => void;

export const isResponse = (msg: Response | Notification): msg is Response => {
    return (msg as Response).requestId !== undefined;
};
