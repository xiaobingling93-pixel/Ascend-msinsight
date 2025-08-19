/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/
import { makeAutoObservable } from 'mobx';
import { type BlocksTableData, EventsTableData } from '../utils/RequestUtils';
import { type MenuItemModel } from '../components/ContextMenu';
interface TypeOption {
    label: string | number;
    value: string | number;
}
interface ContextMenu {
    visible: boolean;
    xPos: number;
    yPos: number;
}
interface MarkStamps {
    first: number;
    last: number;
}
export class Session {
    language: 'zhCN' | 'enUS' = 'enUS';
    deviceIds: any = {};
    threadIds: number[] = [];
    blockData: any = { blocks: [], minSize: 0, maxSize: 0, minTimestamp: 0, maxTimestamp: 0 }; // 内存块图数据源
    allocationData: any = { allocations: [], maxTimestamp: 0, minTimestamp: 0 };// 内存折线图数据源
    memoryData: any = { size: 0, name: '', subNodes: [] };// 内存拆解图数据源
    funcData: any = { traces: [], maxTimestamp: 0, minTimestamp: 0 };// 调用栈图数据源
    memoryStamp: number = 0;
    deviceId: string = '';
    eventType: string = '';
    threadId: number | string = '';
    maxTime: number = 0; // x轴最大值
    minTime: number = 0; // x轴最小值
    deviceIdOpts: TypeOption[] = [];
    typeOpts: TypeOption[] = [];
    threadOps: TypeOption[] = [];
    legendSelect: any = {}; // 选择的图例
    synStartTime: number = 0;
    synEndTime: number = 0;
    searchFunc: string[] = [];
    funcOptions: any = [];
    threadFlag: boolean = false; // 判断是否由双击事件触发的thread的变化
    maxDepth: number = 0; // 火焰图最大深度
    tableType: 'blocks' | 'events' = 'blocks';
    blocksTableData: BlocksTableData['blocks'] = []; // 内存详情表blocks数据源
    eventsTableData: EventsTableData['events'] = []; // 内存详情表events数据源
    blocksTableHeader: BlocksTableData['headers'] = [];
    eventsTableHeader: EventsTableData['headers'] = [];
    blocksOrder: boolean | string = '';
    eventsOrder: boolean | string = '';
    blocksOrderBy: string = '';
    eventsOrderBy: string = '';
    blocksFilters: { [key: string]: string } = {};
    blocksRangeFilters: { [key: string]: number[] } = {};
    eventsFilters: { [key: string]: string } = {};
    eventsRangeFilters: { [key: string]: number[] } = {};
    //  底部表格分页相关变量
    blocksCurrentPage: number = 1;
    eventsCurrentPage: number = 1;
    blocksPageSize: number = 10;
    eventsPageSize: number = 10;
    blocksTotal: number = 0;
    eventsTotal: number = 0;
    tableKey: number = 0;
    firstOffset: number = 0;
    lastOffset: number = 0;
    firstLastStamps: MarkStamps = { first: 0, last: 0 };
    markLineshow: 'none' | 'block' = 'none';
    allowMark: boolean = true;
    contextMenu: ContextMenu = { visible: false, xPos: 0, yPos: 0 };
    menuItems: MenuItemModel[] = [];
    constructor() {
        makeAutoObservable(this);
    }
}
