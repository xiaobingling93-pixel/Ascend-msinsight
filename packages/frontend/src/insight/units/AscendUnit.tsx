import { runInAction } from 'mobx';
import { ChartConfig } from '../../entity/chart';
import { chart, ChartDesc, InsightUnit, on, unit, UnitHeight } from '../../entity/insight';
import { Session } from '../../entity/session';
import { hashToNumber } from '../../utils/colorUtils';
import {
    CardMetaData, InsightMetaData, MetaData, ProcessMetaData, ThreadMetaData, ThreadTrace,
} from '../../entity/data';
import { colorPalette } from './utils';
import { getRange } from '../../cache/utils';

const paramsTree = new Map();

export const ThreadUnit = unit<ThreadMetaData>({
    name: 'Thread',
    tag: 'Thread',
    pinType: 'move',
    renderInfo: (session: Session, thread: ThreadMetaData) => {
        return `${thread.threadName} ${thread.threadId}`;
    },
    chart: chart({
        type: 'stackStatus',
        height: UnitHeight.STANDARD,
        mapFunc: async (session: Session, metaData: unknown) => {
            const threadMetaData = metaData as ThreadMetaData;
            const [ startTime, endTime ] = getRange(session);
            const requestParam = {
                cardId: threadMetaData.cardId,
                processId: threadMetaData.processId,
                threadId: threadMetaData.threadId,
                startTime: startTime,
                endTime: endTime,
            };
            try {
                const request = await window.request('unit/threadTraces', requestParam);
                const threadTraceList = request.data as ThreadTrace[][];
                return threadTraceList.map(it => it.map((data) => ({
                    startTime: data.startTime,
                    duration: data.duration,
                    name: data.name,
                    type: data.name,
                    color: colorPalette[hashToNumber(data.name, colorPalette.length)],
                    depth: data.depth,
                    threadId: data.threadId,
                })));
            } catch (e) {
                console.warn('request threadTrace info failed', e);
                return [];
            }
        },
        config: {
            rowHeight: UnitHeight.STANDARD,
        },
    }),
});

export const ProcessUnit = unit<ProcessMetaData>({
    name: 'Process',
    tag: 'Process',
    pinType: 'move',
    renderInfo: (session: Session, metadata: { processName: string }) => `${metadata.processName}`,
});

function newLane (insightMetaData: InsightMetaData<any>): InsightUnit | undefined {
    switch (insightMetaData.type) {
        case 'process': {
            return new ProcessUnit({
                cardId: paramsTree.get(insightMetaData.metadata).cardId,
                processId: insightMetaData.metadata.processId,
                processName: insightMetaData.metadata.processName,
            });
        }
        case 'thread': {
            const threadUnit = new ThreadUnit({
                cardId: paramsTree.get(paramsTree.get(insightMetaData.metadata)).cardId,
                processId: paramsTree.get(insightMetaData.metadata).processId,
                threadId: insightMetaData.metadata.threadId,
                threadName: insightMetaData.metadata.threadName,
            });
            const chart = threadUnit.chart as ChartDesc<'stackStatus'>;
            chart.height = insightMetaData.metadata.maxDepth * (chart.config as ChartConfig<'stackStatus'>).rowHeight;
            return threadUnit;
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

function recursiveExpandUnit <T extends keyof MetaData> (metaDataList: Array<InsightMetaData<T>>, parentUnit: InsightUnit): void {
    if (metaDataList === undefined || parentUnit === undefined) {
        return;
    }

    for (const metaData of metaDataList) {
        if (parentUnit.children !== undefined) {
            const existingUnit = parentUnit.children.find(unit => checkMetaData(unit.metadata, metaData));
            if (!existingUnit) {
                const newUnit = newLane(metaData);
                if (newUnit !== undefined) {
                    parentUnit.children?.push(newUnit);
                    recursiveExpandUnit(metaData.children ?? [], newUnit);
                }
            } else {
                recursiveExpandUnit(metaData.children ?? [], existingUnit);
            }
        } else {
            const newUnit = newLane(metaData);
            if (newUnit !== undefined) {
                parentUnit.children = [];
                parentUnit.children.push(newUnit);
                recursiveExpandUnit(metaData.children ?? [], newUnit);
            }
        }
    }
}

function handleMap <T extends keyof MetaData> (insightMetaData: InsightMetaData<T>): void {
    paramsTree.clear();
    insightMetaData.children?.forEach(processInfo => {
        paramsTree.set((processInfo.metadata as ProcessMetaData), insightMetaData.metadata);
        processInfo.children?.forEach(threadInfo => {
            paramsTree.set((threadInfo.metadata as ThreadMetaData), (processInfo.metadata as ProcessMetaData));
        });
    });
}

export const CardUnit = unit<CardMetaData>({
    name: 'Card',
    tag: 'Card',
    pinType: 'move',
    renderInfo: (session: Session, metadata: { cardId: string }) => `${metadata.cardId}`,
    spreadUnits: on(
        'create',
        async (self): Promise<void> => {
            runInAction(() => {
                const insightData: InsightMetaData<keyof MetaData> = {
                    type: 'card',
                    metadata: { cardId: '1', cardName: 'card1' },
                    children: [
                        {
                            type: 'process',
                            metadata: { processId: '10001', processName: 'process10001' },
                            children: [
                                {
                                    type: 'thread',
                                    metadata: { threadId: 10010, threadName: 'threadId10010', maxDepth: 9 },
                                },
                            ],
                        },
                    ],
                };
                handleMap(insightData);
                recursiveExpandUnit(insightData.children ?? [], self);
            });
        }),
});
