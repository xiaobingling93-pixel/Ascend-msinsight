// #region cputime
export type ThreadInfo = {
    tid: number;
    name: string;
};

export type ProcessInfo = {
    pid: number;
    name: string;
    CpuCores: number;
};

export type FrameProcessInfo = {
    processId: number;
    processName: string;
    tag: boolean;
};

export type UserTraceSliceList = {
    name: string;
    timestamp: number;
    duration: number;
    cookie: number;
    depth: number;
    taskName: string;
};

export type UserTraceMetadata = {
    taskNameList: string[];
    maxDepthList: number[];
    pidList: number[];
};

export type UserTraceList = {
    id: number;
    task: string;
    startTime: number;
    endTime: number;
    duration: number;
};

export type UserTraceStatistics = {
    taskName: string;
    occurrences: number;
    wallDuration: number;
    avgDuration: number;
    maxDuration: number;
    minDuration: number;
};

export type ProcessLaneCpuUsage = {
    duration: number;
    startTime: number;
};

export type CpuMultiSliceProcessDetail = {
    name: string;
    id: number;
    type: 'PROCESS' | 'THREAD';
    duration: number;
    percentage: number;
    avgDuration: number;
    occurrences: number;
    children: CpuMultiSliceProcessDetail[];
};

export type CpuStateProcessThreadDetail = {
    avgDuration: number;
    type: 'PROCESS' | 'THREAD' | 'STATE';
    children?: CpuStateProcessThreadDetail[] | undefined;
    duration: number;
    id: number;
    maxDuration: number;
    minDuration: number;
    name: string;
    occurrences: number;
};

export type CpuProcessThreadStateDetail = {
    avgDuration: number;
    type: 'PROCESS' | 'THREAD' | 'STATE';
    children?: CpuProcessThreadStateDetail[] | undefined;
    duration: number;
    id: number;
    maxDuration: number;
    minDuration: number;
    name: string;
    occurrences: number;
};

export type CpuMultiSliceList = {
    cpu: number;
    duration: number;
    endState: string;
    hasLatency: boolean;
    itid: number;
    priority: number;
    processCmdLine?: string;
    processId: number;
    processName?: string;
    startTime: number;
    threadId: number;
    threadName: string;
};

export type ThreadTraceMetadata = {
    threadNameList: string[];
    tidList: number[];
    maxDepthList: number[];
};

export type ThreadStateList = {
    cpu: number;
    duration: number;
    endTime: number;
    process: string;
    processId: number;
    startTime: number;
    state: string;
    thread: string;
    threadId: number;
};

export type ThreadWakeUpFrom = {
    threadId: number;
    cpu: number;
    startTime: number;
    processId: number;
};

export type ThreadWakeUpList = {
    threadId: number;
    startTime: number;
    processId: number;
};

export type ThreadStateDetail = {
    cpu: number;
    duration: number;
    endTime: number;
    process: string;
    processId: number;
    startTime: number;
    state: string;
    thread: string;
    threadId: number;
    threadWakeUpFrom?: ThreadWakeUpFrom | undefined;
    threadWakeUpList?: ThreadWakeUpList[] | undefined;
};

export type ThreadTraceList = {
    name: string;
    duration: number;
    startTime: number;
    endTime: number;
    depth: number;
    threadId: number;
};

export type ThreadTraceDetail = ThreadTraceList & { relateTraceList?: RelateTrace[] } & { relateFrameDetail?: RelateFrameDetail };

export type RelateTrace = {
    relateKey: string;
    name: string;
    startTime: number;
    duration: number;
    depth: number;
    threadId: number;
};

export type RelateFrameDetail = {
    processId: number;
    startTime: number;
    duration: number;
    depth: number;
    isJank: boolean;
    jankType: string;
    frameNo: number;
};

export type ThreadStateStatistics = {
    avgDuration: number;
    occurrences: number;
    processId: number;
    process: string;
    state: string;
    thread: string;
    threadId: number;
    duration: number;
    maxDuration: number;
    minDuration: number;
};

export type ThreadTraceStatistics = {
    name: string;
    wallDuration: number;
    avgDuration: number;
    occurrences: number;
};

export type ProfileNode = {
    name: string;
    globalTotal: number;
    self: number;
    globalChildrenTotal: number;
    selfCostPercent: number;
    totalCostPercent: number;
    posTicks: Array<{
        ticks: number;
        line: number;
    }>;
    url: string;
    lineNumber?: number;
    columnNumber?: number;
    children?: ProfileNode[];
    category?: 'js' | 'system' | 'napi' | 'native';
} & NativeCall;
// #endregion

interface SysEventBase {
    id: number;
    name: string;
    time: number;
    tz: string;
    pid: number;
    tid: number;
    timestamp: number;
}

export interface KeyEventInfo extends SysEventBase{
    happenTime: number;
    upTime?: string;
    duration: number;
}

export interface EnergyAnomalyEventInfo {
    name: string;
    happenTime: number;
    endTime: number;
    duration: number;
}

export interface EnergyMonitorDetails {
    realTimeEnergy: number;
    totalEnergy: number;
}

export interface AbilityEvent extends SysEventBase{
    userId: number;
    appId: number;
    bundleName: string;
    moduleName: string;
    abilityName: string;
    processName: string;
}

export type NativeCall = {
    functionName?: string;
    threadName?: string;
    tid: number;
    depth: number;
    totalTime: number;
    selfTime: number;
    startTs: number;
    endTs: number;
    source?: string;
    url?: string;
    sourceAddr: number;
    children?: NativeCall[];
};

export type JsDetailMemory = {
    id?: number;
    className: string;
    childrenCount: number;
    distance: number;
    retainedSize: number;
    retainedSizePercent: number;
    shallowSize: number;
    shallowSizePercent: number;
    name: string;
    reachableFromWindow?: boolean;
    parentId?: number;
    pathNode?: number[];
    retainerNodes: JsDetailMemory[] | undefined;
    children?: JsDetailMemory[];
};

export type AssignStack = {
    column: number;
    functionInfoIndex: number;
    line: number;
    name: string;
    nodeId: number;
    scriptName: string;
    traceNodeId: number;
};

export type StatisticMemory = {
    total: number;
    js: number;
    native: number;
    stack: number;
    code: number;
    others: number;
    category: string;
};

export type NativeThread = {
    threadUsage: number;
    state: number;
    timestamp: number;
};

// Native Memory实时主泳道区域数据
export type NativeMemory = {
    timestamp: number;
    total: number;
};

// Native Memory详情数据，三个tab下的每条数据定义
export type NativeMemoryStatistics = {
    idList: number;
    laneType: number;
    eventType: string;
    total: number; // 总计大小
    totalNumber: number; // 总计涉及次数
    malloc: number; // 创建的内存大小
    mallocNumber: number; // 创建的内存次数
    free: number; // 释放的内存大小
    freeNumber: number; // 释放的内存次数
};

export type NativeMemoryCallInfo = {
    laneType: number;
    symbolName: string; // 内存分配的调用栈
    size: number; // 分配的大小
    sizePercent: number;
    count: number;
    category?: 'js' | 'system' | 'napi' | 'native'; // 函数类型
    filePath?: string;
    offset: number;
};

export type NativeMemoryObj = {
    index: number;
    id: number;
    laneType: number;
    address: string;
    memoryType: string;
    timestamp: number; // 申请时间
    state: string;
    size: number;
    lib: string;
    caller: string;
    callStack: NativeMemoryCallStack[];
};

export type NativeMemoryCallStack = {
    frameSeqId: number;
    lib: string;
    caller: string;
    filePath: string;
    offset: number;
};

export type JSCpuState = {
    timestamp: number;
    duration: number;
    name: string;
    type: 'js' | 'system' | 'napi' | 'native';
    dataKey: 'jsCpuState';
};

export type JsHeapMemory = {
    timestamp: number;
    usedSize: number;
    totalSize: number;
};

export type AppMemory = {
    timestamp: number;
    total: number;
    native: number;
    graphics: number;
    stack: number;
    code: number;
    others: number;
};

export type HeapSnapshotInfo = {
    id: number;
    rawId: number;
    timestamp: number;
    duration: number;
    fileSize: number;
};

export type TotalHeapSnapshot = HeapSnapshotInfo;

export type HeapDiffNode = {
    name: string;
    addedCount: number;
    removedCount: number;
    addedSize: number;
    childrenCount: number;
    countDelta: number;
    removedSize: number;
    sizeDelta: number;
    distance: number;
    retainedSize: number;
    retainedSizePercent: number;
    shallowSize: number;
    shallowSizePercent: number;
    reachableFromWindow?: boolean;
    parentId?: number;
    pathNode?: number[];
    retainerNodes: JsDetailMemory[] | undefined;
    className: string;
    id?: number;
    children?: HeapDiffNode[];
};

export type CpuInsightMetadata = {
    processTraceMetadata: {
        nameList: string[];
        idList: number[];
    };
    cpuSliceMetadata: number;
    cpuFreqMetadata: {
        freqLaneNumbers: number;
        maxFrequency: number;
    };
    dictData: {
        idList: number[];
        textList: string[];
    };
};

export type CpuUsageLaneData = {
    timestamp: number;
    coreUsage: number;
};

export type CpuSingleSliceDetail = {
    processName: string;
    processId: number;
    threadName: string;
    threadId: number;
    cmdLine: string;
    startTs: number;
    duration: number;
    priority: number;
    endState: string;
    hasLatency: boolean;
    schedulingData?: CpuSingleSliceSchedulingData;
};

export type FrameSliceDetail = {
    frameNo: number;
    vsync: number;
    gpuDuration: number;
    actualStartTime: number;
    actualDuration: number;
    expectStartTime: number;
    expectDuration: number;
    processId: number;
    processName: string;
    jankType: string;
    frameTrace?: FrameRelateTrace;
    appFrameList?: RelateFrame[]; // 为RS帧时 appFrameList字段存在  (appFrameList/rsFrameList不同时存在，但肯定有一个字段存在（可能length===0），标识为app侧帧/RS侧帧）
    rsFrameList?: RelateFrame[]; // 为app帧时 rsFrameList字段存在
    otherAppFrameList?: RelateFrame[]; // 为app帧时 存放可能存在的其余app侧的数据（多对一的情况)
};

export type FrameRelateTrace = {
    name: string;
    threadId: number;
    threadName: string;
    startTime: number;
    duration: number;
    depth: number;
};

export type RelateFrame = {
    frameNo: number;
    vsync: number;
    processId: number;
    processName: string;
    startTime: number;
    duration: number;
    depth: number;
    isJank: boolean;
};

export type CpuSingleSliceSchedulingData = {
    wakeUpTime: number;
    cpu: number;
    processName: string;
    processId: number;
    threadName: string;
    threadId: number;
    schedulingLatency: number;
};

export type FrameTraceMetadata = {
    processId: number;
    processName: string;
    depth: number;
    isRenderService: boolean;
};

export type FrameLaneData = {
    frameNo: string;
    vsync: number;
    startTime: number;
    endTime: number;
    jankTime: number;
    depth: number;
    jankType: string;
    expectStartTime: number;
    isJank: boolean;
};

export type FrameAppStatistics = {
    frameNo: number;
    vsync: number;
    startTime: number;
    appDur: number;
    renderServiceDur: number;
    gpuDur: number;
    totalDur: number;
    jankType: string;
};

export type FrameRenderServiceStatistics = {
    frameNo: number;
    vsync: number;
    startTime: number;
    duration: number;
    gpuDur: number;
    jankType: string;
};

export type FrameTotalStatistics = {
    processId: number;
    processName: string;
    jankRate: number;
    jankNum: number;
    maxConsecutiveJankNum: number;
    maxDuration: number;
    avgJankDuration: number;
    avgNormalDuration: number;
};

export type FrameSliceList = {
    processId: number;
    processName: string;
    startTime: number;
    jankType: string;
    frameNo: number;
    duration: number;
    gpuDuration: number;
    vsync: number;
};

export type FrameMetadata = {
    hasRenderService: boolean;
    idList: number[];
};

export type HiperfStopResult = {
    result: boolean;
    body: string;
};

export type EnergyDetailsMap = {
    cpu: EnergyMonitorDetails;
    display: EnergyMonitorDetails;
    gpu: EnergyMonitorDetails;
    location: EnergyMonitorDetails;
    camera: EnergyMonitorDetails;
    bluetooth: EnergyMonitorDetails;
    flashlight: EnergyMonitorDetails;
    audio: EnergyMonitorDetails;
    wifiscan: EnergyMonitorDetails;
};

export type ProcessSearchResultType = {
    type: string;
    startTime: number;
    duration: number;
    depth: 0;
    pid: number;
    tid: number;
    taskName: string;
    cookie: number;
};

export type FrameSearchResultType = {
    processId: number;
    startTime: number;
    endTime: number;
    depth: number;
    isJank: boolean;
};

export type CPUCoreReqParams = {
    unit?: string;
    index?: number;
    endTimeAll?: number;
    processId?: number;
    threadId?: number;
    processName?: string;
    threadName?: string;
};

export type ProcessReqParams = {
    unit?: string;
    index?: number;
    functionName?: string;
    endTimeAll?: number;
};

export type FrameReqParams = {
    unit?: string;
    index?: number;
    startTimeAll?: number;
    endTimeAll?: number;
    processId?: number;
    processName?: string;
    jankType?: string;
};

export type ArkTSReqParams = {
    unit?: string;
    index: number;
    startTimeAll: number;
    endTimeAll: number;
    startRecordTime: number;
    functionName?: string;
};

export type NativeReqParams = {
    unit?: string;
    index: number;
    threadId?: number;
    startTimeAll: number;
    endTimeAll: number;
    startRecordTime: number;
    functionName?: string;
};

export type CpuSliceWakeType = {
    currentCpu: number;
    wakeUpCpu: number;
    wakeUpTime: number;
    startTs: number;
    schedulingLatency: number;
};

export type MetaData = {
    card: CardMetaData;
    process: ProcessMetaData;
    thread: ThreadMetaData;
}

export type InsightMetaData <T extends keyof MetaData> = {
    type: T;
    metadata: MetaData[T];
    children?: InsightMetaData <keyof MetaData> [];
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

export type ThreadTrace = {
    name: string;
    duration: number;
    startTime: number;
    endTime: number;
    depth: number;
    threadId: number;
};

export type ThreadTraceRequest = {
    cardId: number,
    processId: string,
    threadId: number,
    startTime: number,
    endTime: number,
};
