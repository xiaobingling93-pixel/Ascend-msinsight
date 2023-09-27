import { chart, on, LinkDataDesc, singleData, TriggerEvent, unit, UnitHeight, MetaData, InsightUnit } from '../../entity/insight';
import { Session } from '../../entity/session';
import { hashToNumber } from '../../utils/colorUtils';
import {
    CardMetaData, ProcessMetaData, ThreadMetaData, ThreadTrace, AscendSliceDetail,
} from '../../entity/data';
import { createStackStatusParam } from './unitFunc';
import { SelectedDataBottomPanel } from '../../components/SelectedDataBottomPanel';
import { SimpleTabularDetail } from '../../components/details/SimpleDetail';
import { DetailTabs, TabPanes } from '../../components/details/TabPanes';
import { SelectSimpleTabularDetail } from '../../components/details/SelectSimpleDetail';
import { renderRadiusBorder } from '../../components/details/utils';
import { getTimestamp } from '../../utils/humanReadable';
import { slicesListDetail, generateLinkDetail, generateFlowParam } from './details';
import { colorPalette } from './utils';
import React from 'react';
import { observer } from 'mobx-react-lite';
import _ from 'lodash';
import { runInAction } from 'mobx';
import { SelectedDataBase } from '../../components/details/base/SelectedData';
import { offsetConfig } from './config/offsetConfig';
import { isPinned } from '../../components/ChartContainer/unitPin';

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
        [ 'Start', data => getTimestamp(data.startTime ?? 0, { precision: 'ns' }), isHiddenStartTime ],
        [ 'Wall Duration', data => getSliceTimeDisplay(data.duration), isHiddenDuration ],
        [ 'Self Time', data => getSliceTimeDisplay(data.selfTime), isHiddenSelfTime ],
    ],
    fetchData: async (session: Session, metadata: ThreadMetaData) => {
        const selectedSliceData = session.selectedData as ThreadTrace;
        const timestampOffset = metadata.cardId !== undefined
            ? (session?.unitsConfig.offsetConfig.timestampOffset as Record<string, number>)?.[metadata.cardId] ?? 0
            : 0;
        // 因为泳道chart数据减去了偏移，所有点选的时候得把偏移加回来
        const params = {
            rankId: metadata.cardId,
            pid: metadata.processId,
            tid: metadata.threadId,
            startTime: selectedSliceData.startTime + timestampOffset,
            depth: selectedSliceData.depth,
        };
        const result = await window.request(metadata.dataSource, { command: 'unit/threadDetail', params });
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
    renderInfo: (session: Session, thread: ThreadMetaData, thisUnit: InsightUnit) => {
        return isPinned(thisUnit) ? `${thread.cardId}_${thread.processName} (${thread.processId})_${thread.threadName}` : `${thread.threadName}`;
    },
    chart: chart({
        type: 'stackStatus',
        height: UnitHeight.STANDARD,
        mapFunc: async (session: Session, metaData: unknown) => {
            const threadMetaData = metaData as ThreadMetaData;
            // 查询泳道chart参数加上时间偏移
            const timestampOffset = threadMetaData.cardId !== undefined
                ? (session?.unitsConfig.offsetConfig.timestampOffset as Record<string, number>)?.[threadMetaData.cardId] ?? 0
                : 0;
            const requestParam = {
                cardId: threadMetaData.cardId,
                processId: threadMetaData.processId,
                threadId: threadMetaData.threadId,
                startTime: session.domainRange.domainStart + timestampOffset,
                endTime: Math.min(session.endTimeAll ?? 0, session.domainRange.domainEnd + timestampOffset),
                dataSource: threadMetaData.dataSource,
            };
            const requestKey = createStackStatusParam('unit/threadTraces', requestParam);
            try {
                const request = await session.simpleCache.tryFetchFromCache('unit/threadTraces', requestKey, requestParam);
                if (request === undefined) {
                    return [];
                }
                const threadTraceList = request.data as ThreadTrace[][];
                // 泳道chart返回数据减去时间偏移
                return threadTraceList.map(it => it.map((data) => ({
                    startTime: data.startTime - timestampOffset,
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
        onClick: async (data, session, metadata) => {
            if (data === undefined) { return; }
            runInAction(() => {
                session.selectedData = data;
                session.linkDetail = generateLinkDetail((metadata as ThreadMetaData).threadName.toLowerCase().includes('stream') ? 'Incoming flow' : 'Outgoing flow');
                session.linkFlow = generateFlowParam(metadata as ThreadMetaData, data.startTime);
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
    bottomPanelRender: (session: Session, triggerEvent: TriggerEvent, metadata) => {
        if (triggerEvent === 'SELECTED_DATA') {
            return {
                Detail: ({ session }) => <SelectedDataBottomPanel session={session} detail={singleSliceDetail}>{EmptyJSXElement}</SelectedDataBottomPanel>,
                DetailTitle: 'Slice Detail',
                // More: ({ session }) => <SliceRight session={session} detail={generateLinkDetail('Outgoing flow')} metadata={metadata} />,
            };
        }
        return TabPanes({ tabs, commonBottomPanel });
    },
});

export const ProcessUnit = unit<ProcessMetaData>({
    name: 'Process',
    tag: (_, metadata: { label?: string }) => `${metadata.label}`,
    pinType: 'move',
    chart: chart({
        type: 'status',
        mapFunc: async (session: Session, metaData: unknown) => {
            return [];
        },
        config: {
            rowHeight: UnitHeight.STANDARD,
        },
        height: UnitHeight.UPPER,
    }),
    renderInfo: (_, metadata: ProcessMetaData, thisUnit) => {
        return isPinned(thisUnit) ? `${metadata.cardId}_${metadata.processName} (${metadata.processId})` : `${metadata.processName} (${metadata.processId})`;
    },
});

export const CardUnit = unit<CardMetaData>({
    name: 'Card',
    configBar: offsetConfig,
    pinType: 'move',
    renderInfo: (session: Session, metadata: { cardId: string }) => `${metadata.cardId}`,
    spreadUnits: on(
        'create',
        async (self): Promise<void> => {
        }),
});

const useSliceRightDataUpdator = (session: Session, originDetail: LinkDataDesc<Record<string, unknown>>, linkFlow: unknown, metadata: unknown): Array<[string, string | JSX.Element]> | undefined => {
    const [ renderFields, setRenderFields ] = React.useState<Array<[string, string | JSX.Element]>>();
    const { selectedUnits } = session;
    const selectedUnit = selectedUnits.length > 0 ? selectedUnits[0] : undefined;
    const detail = (session.linkDetail as LinkDataDesc<Record<string, unknown>>) ?? originDetail;
    const fetchData = session.phase === 'error' ? undefined : detail?.fetchData;
    const onDataFetched = React.useMemo(() => ([ selectedUnits, linkFlow ].filter(_.isEmpty).length === 0
        ? fetchData?.(session, selectedUnit?.metadata as MetaData)
        : undefined), [ selectedUnits, linkFlow, detail, session.linkDetail ]);

    const recentUnits = React.useRef(selectedUnits);
    const recentData = React.useRef(linkFlow);
    recentUnits.current = selectedUnits;
    recentData.current = linkFlow;
    const loadData = (): void => {
        if (onDataFetched !== undefined && linkFlow !== undefined) {
            onDataFetched?.then(result => {
                if (recentUnits.current !== selectedUnits || recentData.current !== linkFlow) return;
                const state: Array<[string, string | JSX.Element]> = [];
                if (Array.isArray(result)) {
                    const templateField = detail.templateField;
                    if (!templateField) { return; }
                    result.forEach(res => {
                        const render = templateField[1];
                        if (templateField[2] !== undefined) {
                            const isHiden = templateField[2];
                            isHiden(res) && state.push([ templateField[0], render(res, session, metadata) ]);
                        } else {
                            state.push([ templateField[0], render(res, session, metadata) ]);
                        }
                    });
                } else {
                    detail.renderFields.forEach(renderField => {
                        const render = renderField[1];
                        if (renderField[2] !== undefined) {
                            const isHiden = renderField[2];
                            isHiden(result) && state.push([ renderField[0], render(result, session, metadata) ]);
                        } else {
                            state.push([ renderField[0], render(result, session, metadata) ]);
                        }
                    });
                }
                setRenderFields(state);
            });
        }
    };

    React.useEffect(loadData, [ selectedUnits, linkFlow, detail, session.linkDetail ]);
    return renderFields;
};

export const SliceRight = observer(({ session, detail, metadata }: { session: Session; detail: LinkDataDesc<Record<string, unknown>>; metadata: unknown }) => {
    const renderFields = useSliceRightDataUpdator(session, detail, session.linkFlow, metadata);
    return <SelectedDataBase renderer={renderFields} hasTitle />;
});
