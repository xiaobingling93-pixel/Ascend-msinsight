const origin = Symbol('origin');

export type WithOrigin<T extends object> = T & { [origin]?: T };

export function getOrigin<T extends object>(data: WithOrigin<T>): T {
    if (data[origin] === undefined) {
        data[origin] = data;
    }
    return data[origin];
};
