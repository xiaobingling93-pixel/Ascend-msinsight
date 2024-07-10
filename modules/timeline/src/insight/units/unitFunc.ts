import {
    InsightMetaData,
    MetaData,
    ProcessMetaData,
    ThreadTraceRequest,
    ThreadMetaData,
    CounterRequest,
} from '../../entity/data';
import { ChartDesc, InsightUnit, UnitHeight } from '../../entity/insight';
import { CounterUnit, ProcessUnit, ThreadUnit, LabelUnit } from './AscendUnit';

const paramsTree = new Map();

export function recursiveExpandUnit <T extends keyof MetaData> (metaDataList: Array<InsightMetaData<T>>, parentUnit: InsightUnit): void {
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

export function handleMap <T extends keyof MetaData> (insightMetaData: InsightMetaData<T>, dataSource: DataSource): void {
    paramsTree.clear();
    insightMetaData.children?.forEach(processInfo => {
        const processMetadata = (processInfo.metadata as ProcessMetaData);
        processMetadata.dataSource = dataSource;
        insightMetaData.metadata.dataSource = dataSource;
        paramsTree.set(processMetadata, insightMetaData.metadata);
        handleChildren(processInfo);
    });
}

function handleChildren<T extends keyof MetaData>(processInfo: InsightMetaData<T>): void {
    processInfo.children?.forEach(threadInfo => {
        paramsTree.set(threadInfo.metadata, processInfo.metadata);
        if (threadInfo.children && threadInfo.children.length > 0) {
            handleChildren(threadInfo);
        }
    });
};

function newLane (insightMetaData: InsightMetaData<any>, parentMetaData: any): InsightUnit | undefined {
    switch (insightMetaData.type) {
        case 'label': {
            const meta = generateMetaData(paramsTree.get(insightMetaData.metadata).cardId, insightMetaData.metadata.processId, insightMetaData.metadata.processName, '', '', paramsTree.get(insightMetaData.metadata).dataSource);
            meta.metaType = insightMetaData.metadata.metaType;
            return new LabelUnit(meta);
        }
        case 'process': {
            const meta = generateMetaData(insightMetaData.metadata.cardId, insightMetaData.metadata.processId, insightMetaData.metadata.processName, '', '', paramsTree.get(insightMetaData.metadata).dataSource);
            meta.label = insightMetaData.metadata.label;
            meta.metaType = insightMetaData.metadata.metaType;
            return new ProcessUnit(meta);
        }
        case 'thread': {
            const meta = generateMetaData(insightMetaData.metadata.cardId, (parentMetaData as ProcessMetaData).processId,
                (parentMetaData as ProcessMetaData).processName, insightMetaData.metadata.threadId, insightMetaData.metadata.threadName,
                paramsTree.get(insightMetaData.metadata).dataSource);
            meta.metaType = insightMetaData.metadata.metaType;
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
            const meta = generateMetaData(paramsTree.get(paramsTree.get(insightMetaData.metadata)).cardId, (parentMetaData as ProcessMetaData).processId,
                insightMetaData.metadata.processName, insightMetaData.metadata.threadId, insightMetaData.metadata.threadName,
                paramsTree.get(insightMetaData.metadata).dataSource);
            meta.dataType = insightMetaData.metadata.dataType;
            meta.metaType = insightMetaData.metadata.metaType;
            return new CounterUnit(meta);
        }
        default:
            return undefined;
    }
}

function generateMetaData(cardId: string, processId: string, processName: string, threadId: string, threadName: string, dataSource: DataSource): any {
    return {
        cardId,
        processId,
        processName,
        threadId,
        threadName,
        dataSource,
    };
};

function checkMetaData<T extends keyof MetaData>(unitMetaData: any, paramMetaData: InsightMetaData<T>): boolean {
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
    return `cardId${processParams.cardId}&processId${processParams.processId}`;
}

export function createStackStatusParam(method: string, params: Record<string, unknown>): string {
    const threadTracesParams = params as unknown as ThreadTraceRequest;
    return `cardId${threadTracesParams.cardId}&processId${threadTracesParams.processId}&threadId${threadTracesParams.threadId}`;
}

export function createCounterParam(method: string, params: Record<string, unknown>): string {
    const counterParams = params as unknown as CounterRequest;
    return `cardId${counterParams.rankId}&processId${counterParams.pid}&threadId${counterParams.threadName}`;
}
