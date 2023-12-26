import {
    InsightMetaData,
    MetaData,
    ProcessMetaData,
    ThreadTraceRequest,
    ThreadMetaData,
    CounterRequest,
} from '../../entity/data';
import { ChartDesc, InsightUnit, UnitHeight } from '../../entity/insight';
import { CounterUnit, ProcessUnit, ThreadUnit } from './AscendUnit';

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
        processInfo.children?.forEach(threadInfo => {
            paramsTree.set((threadInfo.metadata as ThreadMetaData), processMetadata);
        });
    });
}

function newLane (insightMetaData: InsightMetaData<any>, parentMetaData: any): InsightUnit | undefined {
    switch (insightMetaData.type) {
        case 'process': {
            return new ProcessUnit({
                cardId: paramsTree.get(insightMetaData.metadata).cardId,
                processId: insightMetaData.metadata.processId,
                processName: insightMetaData.metadata.processName,
                label: insightMetaData.metadata.label,
                dataSource: paramsTree.get(insightMetaData.metadata).dataSource,
            });
        }
        case 'thread': {
            const threadUnit = new ThreadUnit({
                cardId: paramsTree.get(paramsTree.get(insightMetaData.metadata)).cardId,
                processId: (parentMetaData as ProcessMetaData).processId,
                processName: (parentMetaData as ProcessMetaData).processName,
                threadId: insightMetaData.metadata.threadId,
                threadName: insightMetaData.metadata.threadName,
                dataSource: paramsTree.get(insightMetaData.metadata).dataSource,
            });
            const chart = threadUnit.chart as ChartDesc<'stackStatus'>;
            if (insightMetaData.metadata.maxDepth === 1) {
                chart.height = UnitHeight.STANDARD;
                (chart.config as any).isCollapse = false;
                threadUnit.collapsible = false;
            }
            (chart.config as any).maxDepth = insightMetaData.metadata.maxDepth;
            return threadUnit;
        }
        case 'counter': {
            return new CounterUnit({
                cardId: paramsTree.get(paramsTree.get(insightMetaData.metadata)).cardId,
                processId: (parentMetaData as ProcessMetaData).processId,
                threadName: insightMetaData.metadata.threadName,
                dataType: insightMetaData.metadata.dataType,
                dataSource: paramsTree.get(insightMetaData.metadata).dataSource,
            });
        }
        default:
            return undefined;
    }
}

function checkMetaData<T extends keyof MetaData>(unitMetaData: any, paramMetaData: InsightMetaData<T>): boolean {
    if (paramMetaData.type === 'thread' && (unitMetaData as ThreadMetaData).threadId === (paramMetaData.metadata as ThreadMetaData).threadId) {
        return true;
    } else if (paramMetaData.type === 'process' && (unitMetaData as ProcessMetaData).processId === (paramMetaData.metadata as ProcessMetaData).processId) {
        return true;
    }
    return false;
}

export function createStackStatusParam(method: string, params: Record<string, unknown>): string {
    const threadTracesParams = params as ThreadTraceRequest;
    return `cardId${threadTracesParams.cardId}&processId${threadTracesParams.processId}&threadId${threadTracesParams.threadId}`;
}

export function createCounterParam(method: string, params: Record<string, unknown>): string {
    const threadTracesParams = params as CounterRequest;
    return `cardId${threadTracesParams.rankId}&processId${threadTracesParams.pid}&threadId${threadTracesParams.threadName}`;
}
