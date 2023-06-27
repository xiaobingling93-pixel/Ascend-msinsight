import { SHA256 } from 'crypto-js';

export const CONTENT_LENGTH_PREFIX = 'Content-Length';
export const PORT = 9000;

export type Request = {
    id: number;
    method: string;
    params: Record<string, unknown>;
};

export type Response<T = Record<string, unknown>> = { id: number } & ({
    result: T;
} | {
    error: {
        code: number;
        message: string;
    };
});

export function getTrackId(tid: number, pid: string): number {
    const str = pid + tid.toString();
    const hashDigest = SHA256(str);
    return hashDigest.words[0];
}
