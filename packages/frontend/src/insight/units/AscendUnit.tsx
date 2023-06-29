import { chart, on, unit, UnitHeight } from '../../entity/insight';
import { Session } from '../../entity/session';
import { hashToNumber } from '../../utils/colorUtils';
import {
    CardMetaData, ProcessMetaData, ThreadMetaData, ThreadTrace,
} from '../../entity/data';
import { colorPalette } from './utils';
import { simpleCache } from '../../cache/simplecache';
import { createStackStatusParam } from './unitFunc';

export const ThreadUnit = unit<ThreadMetaData>({
    name: 'Thread',
    pinType: 'move',
    renderInfo: (session: Session, thread: ThreadMetaData) => {
        return `${thread.threadName} ${thread.threadId}`;
    },
    chart: chart({
        type: 'stackStatus',
        height: UnitHeight.STANDARD,
        mapFunc: async (session: Session, metaData: unknown) => {
            const threadMetaData = metaData as ThreadMetaData;
            const requestParam = {
                cardId: threadMetaData.cardId,
                processId: threadMetaData.processId,
                threadId: threadMetaData.threadId,
                startTime: session.domainRange.domainStart + session.startRecordTime,
                endTime: Math.min(session.endTimeAll ?? 0, session.domainRange.domainEnd) + session.startRecordTime,
            };
            const requestKey = createStackStatusParam('unit/threadTraces', requestParam);
            try {
                const request = await simpleCache.tryFetchFromCache('unit/threadTraces', requestKey, requestParam);
                if (request === undefined) {
                    return [];
                }
                const threadTraceList = request.data as ThreadTrace[][];
                return threadTraceList.map(it => it.map((data) => ({
                    startTime: data.startTime - session.startRecordTime,
                    duration: data.duration,
                    name: data.name,
                    type: data.name,
                    color: colorPalette[hashToNumber(data.name, colorPalette.length)],
                    depth: data.depth,
                    threadId: 0,
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

export const CardUnit = unit<CardMetaData>({
    name: 'Card',
    tag: 'Card',
    pinType: 'move',
    renderInfo: (session: Session, metadata: { cardId: number }) => `${metadata.cardId}`,
    spreadUnits: on(
        'create',
        async (self): Promise<void> => {
        }),
});
