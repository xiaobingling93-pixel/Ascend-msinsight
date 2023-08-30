
export const CONTENT_LENGTH_PREFIX = 'Content-Length';
export const PORT = 9000;

export type Request = {
    id: number;
    method: string;
    params: Record<string, unknown>;
};

export type Response<T = Record<string, unknown>> = {
    id: number;
    result?: T;
    error?: {
        code: number;
        message: string;
    };
};

export type Notification<T = Record<string, unknown>> = {
    method: string;
    params: T;
};

export type ResponseHandler = (res: Response) => void;

export type NotificationHandler = (notification: any) => void;

export const isResopnse = (msg: Response | Notification): msg is Response => {
    return (msg as Response).id !== undefined;
};
