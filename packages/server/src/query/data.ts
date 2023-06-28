export type ThreadDetailResponse = {
    emptyFlag: boolean;
    data: ThreadDetail[];
};

type ThreadDetail = {
    title: string;
    selfTime: number;
    duration: number;
    args: string;
};

export type ThreadsResponse = {
    emptyFlag: boolean;
    data: Array<{
        title: string;
        selfTime: number;
        wallDuration: number;
        avgWallDuration: number;
        occurrences: number;
    }>;
};

export type FlowResponse = {
    flowDetail: FlowDetail[];
};

type FlowDetail = {
    title: string;
    track_id: number;
    timestamp: number;
};

export type FlowDetailResponse = {
    title: string;
    cat: string;
    id: string;
    from: SliceDao;
    to: SliceDao;
};

export type ThreadDetailRequest = {
    pid: string;
    tid: number;
    startTime: number;
    depth: number;
};

export type ThreadsRequest = {
    pid: string;
    tid: number;
    startTime: number;
    endTime: number;
    depth: number;
};

export type EventRequest = {
    pid: string;
    tid: number;
    startTime: number;
};

export type FlowDetailRequest = {
    pid: string;
    tid: number;
    startTime: number;
    title: string;
};

export type SliceDao = {
    id: number;
    timestamp: number;
    duration: number;
    name: string;
    depth: number;
    track_id: number;
    args: string;
};

export type FlowDao = {
    id: number;
    name: string;
    flow_id: string;
    track_id: number;
    timestamp: number;
    cat: string;
    type: string;
};

export type MetaData = {
    card: CardMetaData;
    process: ProcessMetaData;
    thread: ThreadMetaData;
};

export type metadataDto = {
    pid: string;
    processName: string;
    tid: number;
    threadName: string;
    maxDepth: number;
};

export type InsightMetaData <T extends keyof MetaData> = {
    type: T;
    metadata: MetaData[T];
    children?: Array<InsightMetaData <keyof MetaData>>;
};

export type ThreadMetaData = {
    cardId?: number;
    processId?: string;
    threadId: number;
    threadName: string;
    maxDepth?: number;
};

export type ProcessMetaData = {
    cardId?: number;
    processId: string;
    processName: string;
};

export type CardMetaData = {
    cardId: number;
    cardName?: string;
};
