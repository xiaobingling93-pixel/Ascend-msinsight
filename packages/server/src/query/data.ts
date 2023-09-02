export type ThreadDetailResponse = {
    emptyFlag: boolean;
    data: ThreadDetail;
};

type ThreadDetail = {
    title: string;
    selfTime: number;
    duration: number;
    args: string;
    cat: string;
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
    tid: number;
    pid: string;
    timestamp: number;
    depth: number;
    flowId: string;
};

export type LocationData = {
    pid: string;
    tid: number;
    depth: number;
    timestamp: number;
};

export type FlowDetailResponse = {
    title: string;
    cat: string;
    id: string;
    from: LocationData;
    to: LocationData;
};

export type ThreadDetailRequest = {
    rankId: string;
    pid: string;
    tid: number;
    startTime: number;
    depth: number;
};

export type ThreadsRequest = {
    rankId: string;
    pid: string;
    tid: number;
    startTime: number;
    endTime: number;
};

export type EventRequest = {
    rankId: string;
    pid: string;
    tid: number;
    startTime: number;
};

export type FlowDetailRequest = {
    rankId: string;
    flowId: string;
};

export type SliceDto = {
    id: number;
    timestamp: number;
    duration: number;
    name: string;
    depth: number;
    track_id: number;
    args: string;
    cat: string;
};

export type SimpleSlice = {
    timestamp: number;
    duration: number;
    endTime: number;
    name: string;
    depth: number;
};

export type FlowDto = {
    id: number;
    name: string;
    flow_id: string;
    track_id: number;
    timestamp: number;
    cat: string;
    type: string;
};

export type SimpleFlowDto = {
    name: string;
    flowId: string;
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
    label: string;
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
    cardId?: string;
    processId?: string;
    threadId: number;
    threadName: string;
    maxDepth?: number;
};

export type ProcessMetaData = {
    cardId?: string;
    processId: string;
    processName: string;
    label?: string;
};

export type CardMetaData = {
    cardId: string;
    cardName?: string;
};

export type ExtremumTimestamp = {
    minTimestamp: number;
    maxTimestamp: number;
};

export type ComputeDetailRequest = {
    rankId: string;
    timeFlag: string;
    currentPage: number;
    pageSize: number;
    orderBy: string;
    order: string;
};

export type CommunicationDetailRequest = {
    rankId: string;
    currentPage: number;
    pageSize: number;
    orderBy: string;
    order: string;
};

export type CommunicationDetailResponse = {
    totalNum: number;
    communicationDetail: CommunicationDetail[];
};
export type CommunicationDetail = {
    communicationKernel: string;
    startTime: number;
    totalDuration: number;
    overlapDuration: number;
    notOverlapDuration: number;
};
export type ComputeDetailResponse = {
    totalNum: number;
    computeDetail: ComputeDetail[];
};

export type ComputeDetail = {
    name: string;
    type: string;
    startTime: number;
    duration: number;
    waitTime: number;
    blockDim: number;
    inputShapes: string;
    inputDataTypes: string;
    inputFormats: string;
    outputShapes: string;
    outputDataTypes: string;
    outputFormats: string;
};

export type SummaryItemVO = {
    rankId: String;
    totalTime: number;
    computingTime: number;
    communicationOverLappedTime: number;
    communicationNotOverLappedTime: number;
    freeTime: number;
};

export type SummaryVO = {
    rankCount: number;
    rankList: string[];
    dataSize: number;
    collectStartTime: number;
    filePath: string;
    collectDuration: number;
    stepNum: number;
    stepList: string[];
    summaryList: SummaryItemVO[];
};

export type SummaryStatisticsVO = {
    acceleratorCore: string;
    duration: number;
    utilization: number;
    overlapType: string;
};

export type SummaryRequest = {
    limit: number;
    stepIdList: string[];
    rankIdList: string[];
    orderBy: string;
};

export type StageAndBubbleTime = {
    stageOrRankId: string;
    stageTime: number;
    bubbleTime: string;
};

export type StageAndBubbleTimeResponse = {
    stageAndBubbleTimes: StageAndBubbleTime[];
};
