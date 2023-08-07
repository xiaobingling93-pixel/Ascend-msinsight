import path from 'path';

export const CONTENT_LENGTH_PREFIX = 'Content-Length';
export const PORT = 9000;
export const INSIGHT_NAME = 'insight_server';
export const USER_HOME = getUserHome();
export const CACHE_PATH = path.join(USER_HOME, '.'.concat(INSIGHT_NAME));

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

function getUserHome(): string {
    if (process.env.HOME !== undefined) {
        return process.env.HOME;
    } else if (process.env.USERPROFILE !== undefined) {
        return process.env.USERPROFILE;
    } else {
        return __dirname;
    }
}
