export type ThreadDetailResponse = {
    emptyFlag: boolean;
    data: {
        title: string;
        selfTime: number;
        duration: number;
        args: string;
    };
};

export type ThreadDetailRequest = {
    pid: string;
    tid: number;
    startTime: number;
    depth: number;
};

export type SliceDao = {
    timestamp: number;
    duration: number;
    name: string;
    args: string;
};
