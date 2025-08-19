/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
export interface BarData {
    blockData: BlockData;
    allocationData: AllocationData;
    isDark: boolean;
    getNewGraphData: any;
};
export interface BlockData {
    minTimestamp: number;
    maxTimestamp: number;
    minSize: number;
    maxSize: number;
    blocks: Block[];
};
export interface Block {
    id: number;
    addr: string;
    size: number;
    startTimestamp: number;
    endTimestamp: number;
    owner: string;
    attr: string;
    path?: number[][];
}
export interface AllocationData {
    minTimestamp: number;
    maxTimestamp: number;
    allocations: Allocation[];
}
export interface Allocation {
    id: number;
    timestamp: number;
    totalSize: number;
}
export interface GraphParam {
    deviceId: string;
    graph: 'blocks' | 'allocations';
    eventType: string;
    relativeTime?: boolean;
    startTimestamp?: number;
    endTimestamp?: number;
}
export interface FuncParam {
    deviceId: string;
    threadId: number;
    relativeTime?: boolean;
    startTimestamp?: number;
    endTimestamp?: number;
}

export interface BlockParam {
    deviceId: string;
    eventType: string;
    isTable: boolean;
    relativeTime?: boolean;
    startTimestamp?: number;
    endTimestamp?: number;
    orderBy?: string;
    desc?: boolean | string;
    currentPage?: number;
    pageSize?: number;
    filters?: { [key: string]: string };
    rangeFilters?: { [key: string]: number[] };
}
export interface EventParam {
    deviceId: string;
    relativeTime?: boolean;
    startTimestamp?: number;
    endTimestamp?: number;
    orderBy?: string;
    desc?: boolean;
    currentPage?: number;
    pageSize?: number;
    filters?: { [key: string]: string };
    rangeFilters?: { [key: string]: number[] };
}
export interface DetailData {
    size: number;
    name: string;
    subNodes?: DetailData[];
}
export interface Trace {
    depth: number;
    endTimestamp: number;
    startTimestamp: number;
    func: string;
}
export interface FuncData {
    minTimestamp: number;
    maxTimestamp: number;
    traces: Trace[];
    maxDepth: number;
}
interface TableHead {
    name: string;
    key: string;
    sortable: boolean;
    searchable: boolean;
}
interface TableDetail {
    id: number;
    event: string;
    eventType: string;
    name: string;
    timestamp: number;
    processId: number;
    threadId: number;
    deviceId: string;
    ptr: string;
    attr: string;
}
export interface BlocksTableData {
    headers: TableHead[];
    blocks: TableDetail[];
    total: number;
}
export interface EventsTableData {
    headers: TableHead[];
    events: TableDetail[];
    total: number;
}
/**
 * 获取图表信息
 * @param params 查询条件
 * @returns 查询结果
 */
export const getLeaksGraphData = async (params: GraphParam): Promise<BlockData | AllocationData> => {
    const { graph, ...rest } = params;
    return window.request({ command: `Memory/leaks/${graph}`, params: { ...rest } });
};
/**
 * 获取内存拆解详情
 * @param params 查询条件
 * @returns 查询结果
 */
export const getMemoryDetailData = async (deviceId: string, timestamp: number, eventType: string): Promise<DetailData> => {
    return window.request({ command: 'Memory/leaks/details', params: { deviceId, timestamp, eventType, relativeTime: true } });
};
/**
 * 获取函数调用详情
 * @param params 查询条件
 * @returns 查询结果
 */
export const getFuncData = async (params: FuncParam): Promise<FuncData> => {
    return window.request({ command: 'Memory/leaks/traces', params: { ...params } });
};
/**
 * 获取内存详情表（内存块视图）
 * @param params 查询条件
 * @returns 查询结果
 */
export const getBlockDetails = async (params: BlockParam): Promise<BlocksTableData> => {
    return window.request({ command: 'Memory/leaks/blocks', params: { ...params } });
};
/**
 * 获取内存详情表（内存事件视图）
 * @param params 查询条件
 * @returns 查询结果
 */
export const getEventDetails = async (params: EventParam): Promise<EventsTableData> => {
    return window.request({ command: 'Memory/leaks/events', params: { ...params } });
};
