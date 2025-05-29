/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import type {
    InsightMetaData,
    MetaDataEnumType,
    ProcessMetaData,
    ThreadTraceRequest,
    ThreadMetaData,
    CounterRequest,
    MetaDataInnerBase,
    LabelMetaData,
    CounterMetaData,
} from '../../entity/data';
import { UnitHeight } from '../../entity/insight';
import type { ChartDesc, InsightUnit } from '../../entity/insight';
import { CounterUnit, ProcessUnit, ThreadUnit, LabelUnit } from './AscendUnit';

const parentMetaDataTree = new Map();

export function recursiveExpandUnit<T extends keyof MetaDataEnumType>(metaDataList: Array<InsightMetaData<T>>, parentUnit: InsightUnit): void {
    if (metaDataList === undefined || parentUnit === undefined) {
        return;
    }
    for (const metaData of metaDataList) {
        if (parentUnit.children !== undefined) {
            const existingUnit = parentUnit.children.find(unit => checkMetaData(unit.metadata, metaData));
            if (!existingUnit) {
                const newUnit = newLane(metaData, parentUnit.metadata);
                if (newUnit !== undefined) {
                    parentUnit.children?.push(newUnit);
                    recursiveExpandUnit(metaData.children ?? [], newUnit);
                }
            } else {
                recursiveExpandUnit(metaData.children ?? [], existingUnit);
            }
        } else {
            const newUnit = newLane(metaData, parentUnit.metadata);
            if (newUnit !== undefined) {
                parentUnit.children = [];
                parentUnit.children.push(newUnit);
                recursiveExpandUnit(metaData.children ?? [], newUnit);
            }
        }
    }
}

export function updateDataSourceAndParentMetaDataMap<T extends keyof MetaDataEnumType>(insightMetaData: InsightMetaData<T>, dataSource: DataSource): void {
    parentMetaDataTree.clear();
    insightMetaData.children?.forEach(processInfo => {
        const processMetadata = (processInfo.metadata as ProcessMetaData);
        processMetadata.dataSource = dataSource;
        insightMetaData.metadata.dataSource = dataSource;
        parentMetaDataTree.set(processMetadata, insightMetaData.metadata);
        handleChildren(processInfo);
    });
}

function handleChildren<T extends keyof MetaDataEnumType>(processInfo: InsightMetaData<T>): void {
    processInfo.children?.forEach(threadInfo => {
        parentMetaDataTree.set(threadInfo.metadata, processInfo.metadata);
        if (threadInfo.children && threadInfo.children.length > 0) {
            handleChildren(threadInfo);
        }
    });
};

function newLane(insightMetaData: InsightMetaData<any>, parentMetaData: any): InsightUnit | undefined {
    switch (insightMetaData.type) {
        case 'label': {
            const parentMetaDataFromTree = parentMetaDataTree.get(insightMetaData.metadata);
            const meta = generateMetaData<LabelMetaData>({ cardId: parentMetaDataFromTree.cardId, filePath: parentMetaDataFromTree.filePath },
                insightMetaData.metadata.processId, insightMetaData.metadata.processName);
            meta.dataSource = parentMetaDataFromTree.dataSource;
            meta.metaType = insightMetaData.metadata.metaType;
            return new LabelUnit(meta);
        }
        case 'process': {
            const meta = generateMetaData<ProcessMetaData>({ cardId: insightMetaData.metadata.cardId, filePath: insightMetaData.metadata.filePath },
                insightMetaData.metadata.processId, insightMetaData.metadata.processName, insightMetaData.metadata.threadId);
            meta.dataSource = parentMetaDataTree.get(insightMetaData.metadata).dataSource;
            meta.label = insightMetaData.metadata.label;
            meta.metaType = insightMetaData.metadata.metaType;
            return new ProcessUnit(meta);
        }
        case 'thread': {
            const meta = generateMetaData<ThreadMetaData>({ cardId: insightMetaData.metadata.cardId, filePath: insightMetaData.metadata.filePath },
                (parentMetaData as ProcessMetaData).processId, (parentMetaData as ProcessMetaData).processName,
                insightMetaData.metadata.threadId, insightMetaData.metadata.threadName);
            meta.dataSource = parentMetaDataTree.get(insightMetaData.metadata).dataSource;
            meta.metaType = insightMetaData.metadata.metaType;
            meta.groupNameValue = insightMetaData.metadata.groupNameValue;
            meta.rankList = insightMetaData.metadata.rankList;
            const threadUnit = new ThreadUnit(meta);
            const chart = threadUnit.chart as ChartDesc<'stackStatus'>;
            if (insightMetaData.metadata.maxDepth === 1 || insightMetaData.metadata.maxDepth === 0) {
                chart.height = UnitHeight.STANDARD;
                (chart.config as any).isCollapse = false;
                threadUnit.collapsible = false;
            }
            (chart.config as any).maxDepth = insightMetaData.metadata.maxDepth;
            return threadUnit;
        }
        case 'counter': {
            const grandParentMetaData = parentMetaDataTree.get(parentMetaDataTree.get(insightMetaData.metadata));
            const meta = generateMetaData<CounterMetaData>({ cardId: grandParentMetaData.cardId, filePath: grandParentMetaData.filePath },
                (parentMetaData as ProcessMetaData).processId, insightMetaData.metadata.processName, insightMetaData.metadata.threadId,
                insightMetaData.metadata.threadName,
            );
            meta.dataSource = parentMetaDataTree.get(insightMetaData.metadata).dataSource;
            meta.dataType = insightMetaData.metadata.dataType;
            meta.metaType = insightMetaData.metadata.metaType;
            return new CounterUnit(meta);
        }
        default:
            return undefined;
    }
}

function generateMetaData<T extends MetaDataInnerBase = MetaDataInnerBase>(
    cardInfo: { cardId: string; filePath: string }, processId: string, processName: string, threadId: string = '', threadName: string = ''): T {
    return {
        cardId: cardInfo.cardId,
        filePath: cardInfo.filePath,
        metaType: '',
        processId,
        processName,
        threadId,
        threadName,
    } as T;
}

function checkMetaData<T extends keyof MetaDataEnumType>(unitMetaData: any, paramMetaData: InsightMetaData<T>): boolean {
    if (paramMetaData.type === 'thread' && (unitMetaData as ThreadMetaData).threadId === (paramMetaData.metadata as ThreadMetaData).threadId) {
        return true;
    } else if (unitMetaData.type === 'process' && paramMetaData.type === 'process' && (unitMetaData as ProcessMetaData).processId === (paramMetaData.metadata as ProcessMetaData).processId) {
        return true;
    } else {
        return false;
    }
}

export function createStatusParam(method: string, params: Record<string, unknown>): string {
    const processParams = params as unknown as ThreadTraceRequest;
    return `cardId${processParams.cardId}&processId${processParams.processId}&s${processParams.startTime}&e${processParams.endTime}`;
}

export function createCounterParam(method: string, params: Record<string, unknown>): string {
    const counterParams = params as unknown as CounterRequest;
    return `cardId${counterParams.rankId}&processId${counterParams.pid}&threadId${counterParams.threadName}` +
        `&s${counterParams.startTime}&e${counterParams.endTime}`;
}
