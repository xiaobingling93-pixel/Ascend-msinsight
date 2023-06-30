import { Session } from '../../entity/session';
import { hashToNumber } from '../../utils/colorUtils';
import { colorPalette } from './utils';
import { runInAction } from 'mobx';
import { chart, on, singleData, TriggerEvent, unit, UnitHeight } from '../../entity/insight';
import {
    CardMetaData, ProcessMetaData, ThreadMetaData, ThreadTrace, AscendSliceDetail,
} from '../../entity/data';
import { createStackStatusParam } from './unitFunc';
import { SelectedDataBottomPanel } from '../../components/SelectedDataBottomPanel';
import React from 'react';
import { SimpleTabularDetail } from '../../components/details/SimpleDetail';
import { DetailTabs, TabPanes } from '../../components/details/TabPanes';
import { SelectSimpleTabularDetail } from '../../components/details/SelectSimpleDetail';
import { slicesListDetail } from './details';
import { renderRadiusBorder } from '../../components/details/utils';

const isHiddenTitle = (data: AscendSliceDetail): boolean => {
    return data.title === undefined;
};

const isHiddenStartTime = (data: AscendSliceDetail): boolean => {
    return data.startTime === undefined;
};

const isHiddenDuration = (data: AscendSliceDetail): boolean => {
    return data.duration === undefined;
};

const isHiddenSelfTime = (data: AscendSliceDetail): boolean => {
    return data.selfTime === undefined || data.selfTime === 0;
};

const nsToMs = (ns: number): number => {
    return ns / 1000000;
};

export const getSliceTimeDisplay = (startTime: number | undefined): string => {
    if (startTime === undefined) {
        return '';
    }
    return `${nsToMs(startTime).toString() + ' ms'}`;
};

const singleSliceDetail = singleData({
    name: 'SingleSlice',
    renderFields: [
        [ 'Title', data => data.title === undefined ? '' : `${data.title}`, isHiddenTitle ],
        [ 'Start', data => getSliceTimeDisplay(data.startTime), isHiddenStartTime ],
        [ 'Wall Duration', data => getSliceTimeDisplay(data.duration), isHiddenDuration ],
        [ 'Self Time', data => getSliceTimeDisplay(data.selfTime), isHiddenSelfTime ],
    ],
    fetchData: async (session: Session, metadata: ThreadMetaData) => {
        const selectedSliceData = session.selectedData as ThreadTrace;
        const params = {
            rankId: metadata.cardId,
            pid: metadata.processId,
            tid: metadata.threadId,
            startTime: selectedSliceData.startTime,
            depth: selectedSliceData.depth,
        };
        const result = await window.request('unit/threadDetail', params);
        const data: AscendSliceDetail = {
            pid: metadata?.processId,
            tid: metadata?.threadId,
            title: result?.data?.title,
            startTime: selectedSliceData?.startTime,
            depth: selectedSliceData?.depth,
            duration: result?.data?.duration,
            selfTime: result?.data?.selfTime,
            args: result?.data?.args,
        };
        return data;
    },
});

const EmptyJSXElement = (): JSX.Element | null => {
    return <></>;
};

const tabs: DetailTabs[] = [
    {
        title: 'Slices List',
        detail: slicesListDetail,
        bottomPanel: {
            Detail: SelectSimpleTabularDetail,
        },
    },
];

const commonBottomPanel = {
    Detail: SimpleTabularDetail,
};

export const ThreadUnit = unit<ThreadMetaData>({
    name: 'Thread',
    pinType: 'move',
    renderInfo: (session: Session, thread: ThreadMetaData) => {
        return `${thread.threadName}`;
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
                startTime: session.domainRange.domainStart,
                endTime: Math.min(session.endTimeAll ?? 0, session.domainRange.domainEnd),
            };
            const requestKey = createStackStatusParam('unit/threadTraces', requestParam);
            try {
                const request = await session.simpleCache.tryFetchFromCache('unit/threadTraces', requestKey, requestParam);
                if (request === undefined) {
                    return [];
                }
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
        decorator: (session: Session, metaData: unknown) => {
            const hoveredData = session.sharedState.threadTrace as ThreadTrace | undefined;
            return {
                action: async (handle, xScale, yScale, theme) => {
                    if (hoveredData && hoveredData.threadId === (metaData as ThreadMetaData).threadId) {
                        const name = hoveredData.name;
                        const data = handle.findAll(it => it.name !== name).map(it => it.map(data => ({ ...data, color: 'transparentMask' as const })));
                        handle.draw(data, xScale, yScale);
                    }
                    // click
                    const ctx = handle.context;
                    const selectedData = session.selectedData as ThreadTrace | undefined;
                    const selectedUnitMetaData = session.selectedUnits?.[0]?.metadata as ThreadMetaData;
                    const threadMetaData = metaData as ThreadMetaData;
                    if (ctx === null || selectedData === undefined || selectedUnitMetaData === undefined || selectedUnitMetaData !== threadMetaData) {
                        return;
                    }
                    // 来自本泳道点击的数据，给数据描边+画线
                    ctx.strokeStyle = theme.fontColor;
                    renderRadiusBorder(xScale(selectedData.startTime), yScale(0), xScale(selectedData.duration < 0 ? session.endTimeAll as number : selectedData.startTime + selectedData.duration) - xScale(selectedData.startTime), yScale(1), selectedData.depth, ctx);
                },
                triggers: [
                    session.selectedData,
                    session.selectedData?.duration,
                    hoveredData?.name,
                    hoveredData?.depth,
                    hoveredData?.threadId,
                ],
            };
        },
        onClick: async (data, session) => {
            if (data === undefined) { return; }
            runInAction(() => {
                session.selectedData = data;
            });
        },
        onHover: (data, session: Session): void => {
            runInAction(() => {
                session.sharedState.threadTrace = data;
            });
        },
        renderTooltip: (data) => new Map([
            [ 'Name', data.name ],
        ]),
        config: {
            rowHeight: UnitHeight.STANDARD,
        },
    }),
    bottomPanelRender: (session: Session, triggerEvent: TriggerEvent) => {
        console.info(triggerEvent);
        if (triggerEvent === 'SELECTED_DATA') {
            return {
                Detail: ({ session }) => <SelectedDataBottomPanel session={session} detail={singleSliceDetail}>{EmptyJSXElement}</SelectedDataBottomPanel>,
                DetailTitle: 'Slice Detail',
            };
        }
        return TabPanes({ tabs, commonBottomPanel });
    },
});

export const ProcessUnit = unit<ProcessMetaData>({
    name: 'Process',
    tag: (_, metadata: { label?: string }) => `${metadata.label}`,
    pinType: 'move',
    renderInfo: (_, metadata: { processName: string; processId: string; label?: string }) => `${metadata.processName} (${metadata.processId})`,
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
