/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
// Timeline Module
import type { CardInfo } from '../components/ImportSelect';

export const CONTENT_LENGTH_PREFIX = 'Content-Length';
export const PORT = 9000;

export interface ImportResult {
    reset: boolean;
    isPending: boolean;
    isSimulation: boolean;
    isIE: boolean;
    isIpynb: boolean;
    isCluster: boolean;
    result: CardInfo[];
}

export interface Request {
    id: number;
    method: string;
    params: Record<string, unknown>;
};

export interface Response<T = Record<string, unknown>> {
    id: number;
    result?: T;
    error?: {
        code: number;
        message: string;
    };
};

export interface Notification<T = Record<string, unknown>> {
    method: string;
    params: T;
};

export type ResponseHandler = (res: Response) => void;

export type NotificationHandler = (notification: Record<string, unknown>) => void;

export const isResopnse = (msg: Response | Notification): msg is Response => {
    return (msg as Response).id !== undefined;
};
