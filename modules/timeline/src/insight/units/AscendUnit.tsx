import {
    chart, ChartDesc,
    InsightUnit,
    LinkDataDesc,
    MetaData,
    on,
    singleData,
    TriggerEvent,
    unit,
    UnitHeight,
} from '../../entity/insight';
import { Session } from '../../entity/session';
import { hashToNumber } from '../../utils/colorUtils';
import type {
    AscendSliceDetail,
    CardMetaData,
    CounterMetaData,
    ProcessData,
    ProcessMetaData,
    ThreadMetaData,
    ThreadTrace,
} from '../../entity/data';
import { createCounterParam, createStackStatusParam, createStatusParam } from './unitFunc';
import { SelectedDataBottomPanel } from '../../components/SelectedDataBottomPanel';
import { SelectSimpleTabularDetail } from '../../components/details/SelectSimpleDetail';
import { renderRadiusBorder } from '../../components/details/utils';
import { getTimestamp } from '../../utils/humanReadable';
import { generateFlowParam, generateLinkDetail, slicesListDetail } from './details';
import { colorPalette } from './utils';
import React, { useEffect, useState, useMemo } from 'react';
import { observer } from 'mobx-react-lite';
import _ from 'lodash';
import { runInAction } from 'mobx';
import { SelectedDataBase } from '../../components/details/base/SelectedData';
import { offsetConfig } from './config/offsetConfig';
import { isPinned, isSonPinned } from '../../components/ChartContainer/unitPin';
import type { Theme } from '@emotion/react';
import { ChartType, StackStatusData, StatusData } from '../../entity/chart';
import { StyledTooltip } from '../../components/base/StyledTooltip';
import ResizeTable from '../../components/resize/ResizeTable';
import { getDefaultColumData, GetPageData } from '../../components/detailViews/Common';
import { calculateDomainRange } from '../../components/CategorySearch';

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

const nsToNs = (ns: number): string => {
    const ms = Math.floor(ns / 1000000);
    const us = Math.floor((ns - ms * 1000000) / 1000);
    const nsRemainder = ns - ms * 1000000 - us * 1000;
    if (ms === 0 && us === 0) {
        return `${nsRemainder}ns`;
    }
    if (ms === 0) {
        return `${us}us${nsRemainder}ns`;
    }
    return `${ms}ms${us}us${nsRemainder}ns`;
};

export const getSliceTimeDisplay = (startTime: number | undefined): string => {
    if (startTime === undefined) {
        return '';
    }
    return `${nsToMs(startTime).toFixed(6).toString() + ' ms'}`;
};

export const getDetailTimeDisplay = (startTime: number | undefined): string => {
    if (startTime === undefined) {
        return '';
    }
    return nsToNs(startTime);
};

export const getDisplay = (val: string | undefined): string => {
    return val === undefined ? '' : val;
};

const isHidden = (val: string | undefined): boolean => {
    return val === undefined || val === '';
};

const singleSliceDetail = singleData({
    name: 'SingleSlice',
    renderFields: [
        ['Title', data => data.title === undefined ? '' : `${data.title}`, isHiddenTitle],
        ['Start', data => getTimestamp(data.startTime ?? 0, { precision: 'ns' }), isHiddenStartTime],
        ['Wall Duration', data => getDetailTimeDisplay(data.duration), isHiddenDuration],
        ['Self Time', data => getDetailTimeDisplay(data.selfTime), isHiddenSelfTime],
        ['Input Shapes', (data: AscendSliceDetail): string => getDisplay(data.inputShapes), (data: AscendSliceDetail): boolean => isHidden(data.inputShapes)],
        ['Input Data Types', (data: AscendSliceDetail): string => getDisplay(data.inputDataTypes), (data: AscendSliceDetail): boolean => isHidden(data.inputDataTypes)],
        ['Input Formats', (data: AscendSliceDetail): string => getDisplay(data.inputFormats), (data: AscendSliceDetail): boolean => isHidden(data.inputDataTypes)],
        ['Output Shapes', (data: AscendSliceDetail): string => getDisplay(data.outputShapes), (data: AscendSliceDetail): boolean => isHidden(data.inputDataTypes)],
        ['Output Data Types', (data: AscendSliceDetail): string => getDisplay(data.outputDataTypes), (data: AscendSliceDetail): boolean => isHidden(data.inputDataTypes)],
        ['Output Formats', (data: AscendSliceDetail): string => getDisplay(data.outputFormats), (data: AscendSliceDetail): boolean => isHidden(data.inputDataTypes)],
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
            startTime: Math.floor(selectedSliceData.startTime + timestampOffset),
            depth: selectedSliceData.depth,
            timePerPx: session.domain.timePerPx,
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
            inputShapes: result?.data?.inputShapes,
            inputDataTypes: result?.data?.inputDataTypes,
            inputFormats: result?.data?.inputFormats,
            outputShapes: result?.data?.outputShapes,
            outputDataTypes: result?.data?.outputDataTypes,
            outputFormats: result?.data?.outputFormats,
        };
        return data;
    },
});

const EmptyJSXElement = (): JSX.Element | null => {
    return <></>;
};

export const ThreadUnit = unit<ThreadMetaData>({
    name: 'Thread',
    pinType: 'copied',
    renderInfo: (session: Session, thread: ThreadMetaData, thisUnit: InsightUnit) => {
        return isPinned(thisUnit) && !isSonPinned(thisUnit) ? `${thread.cardId}_${thread.processName} (${thread.processId})_${thread.threadName}` : `${thread.threadName}`;
    },
    chart: chart({
        type: 'stackStatus',
        height: UnitHeight.COLL,
        mapFunc: async (session: Session, metaData: unknown) => {
            const threadMetaData = metaData as ThreadMetaData;
            // 查询泳道chart参数加上时间偏移
            const timestampOffset = threadMetaData.cardId !== undefined
                ? (session?.unitsConfig.offsetConfig.timestampOffset as Record<string, number>)?.[threadMetaData.cardId] ?? 0
                : 0;
            const requestParam: Record<string, unknown> & { timePerPx: number } = {
                cardId: threadMetaData.cardId,
                processId: threadMetaData.processId,
                threadId: threadMetaData.threadId,
                startTime: Math.floor(session.domainRange.domainStart + timestampOffset),
                endTime: Math.ceil(Math.min(session.endTimeAll ?? 0, session.domainRange.domainEnd + timestampOffset)),
                dataSource: threadMetaData.dataSource,
                timePerPx: session.domain.timePerPx,
            };
            const requestKey = createStackStatusParam('unit/threadTraces', requestParam);
            try {
                const request = await session.simpleCache.tryFetchFromCache('unit/threadTraces', requestKey, requestParam);
                if (request === undefined) {
                    return [];
                }
                const threadTraceList = request.data as ThreadTrace[][];
                // 泳道chart返回数据减去时间偏移
                return _.map(threadTraceList, (it) => _.map(it, (data) => {
                    return {
                        startTime: data.startTime - timestampOffset,
                        duration: data.duration,
                        name: data.name,
                        type: data.name,
                        color: colorPalette[hashToNumber(data.name, colorPalette.length)],
                        depth: data.depth,
                        threadId: data.threadId,
                        cardId: threadMetaData.cardId,
                        cname: data.cname,
                        cColor: colorPalette[hashToNumber(data.cname, colorPalette.length)],
                    } as StackStatusData;
                }));
            } catch (e) {
                console.warn('request threadTrace info failed', e);
                return [];
            }
        },
        decorator: (session: Session, metaData: unknown) => {
            return {
                action: async (handle, xScale, yScale, theme) => {
                    if (session.searchData) {
                        const name = session.searchData.content.toLocaleLowerCase();
                        const data = handle.findAll(it => !it.name.toLocaleLowerCase().includes(name)).map(it => it.map(data => ({ ...data, color: 'transparentMask' as const })));
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
                    session?.searchData,
                ],
            };
        },
        onClick: async (data, session, metadata) => {
            if (data === undefined) { return; }
            runInAction(() => {
                session.selectedData = { ...data, threadId: (metadata as ThreadMetaData).threadId };
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
            ['Name', data.name],
        ]),
        config: {
            rowHeight: UnitHeight.STANDARD,
            isCollapse: true,
        },
    }),
    bottomPanelRender: (session: Session, triggerEvent: TriggerEvent, metadata) => {
        if (triggerEvent === 'SELECTED_DATA') {
            return {
                Detail: ({ session }) => <SelectedDataBottomPanel session={session} detail={singleSliceDetail}>{EmptyJSXElement}</SelectedDataBottomPanel>,
                DetailTitle: 'Slice Detail',
                More: ({ session }) => <SliceRight session={session} detail={generateLinkDetail('Outgoing flow')} metadata={metadata} />,
            };
        }
        return {
            Detail: ({ session, height }) => <SelectSimpleTabularDetail session={session} height={height} detail={slicesListDetail}></SelectSimpleTabularDetail>,
            DetailTitle: 'Slices List',
            More: (): JSX.Element => <SliceRightOpDetail session={session} metadata={metadata} />,
            MoreWh: 320,
        };
    },
    collapseAction: (unit) => {
        const chart = (unit.chart as ChartDesc<ChartType>);
        const config = (unit.chart as ChartDesc<ChartType>).config;
        runInAction(() => {
            (config as any).isCollapse = !((config as any).isCollapse as boolean);
            const collapseHeight = UnitHeight.COLL;
            const expandedHeight = (config as any).maxDepth * (config as any).rowHeight;
            chart.height = ((config as any).isCollapse as boolean) ? collapseHeight : expandedHeight;
        });
    },
});

export const ProcessUnit = unit<ProcessMetaData>({
    name: 'Process',
    tag: (_, metadata: { label?: string }) => `${metadata.label}`,
    pinType: 'copied',
    chart: chart({
        type: 'status',
        mapFunc: async (session: Session, metaData: unknown) => {
            const processMetaData = metaData as ProcessMetaData;
            // 查询泳道chart参数加上时间偏移
            const timestampOffset = processMetaData.cardId !== undefined
                ? (session?.unitsConfig.offsetConfig.timestampOffset as Record<string, number>)?.[processMetaData.cardId] ?? 0
                : 0;
            const requestParam = {
                cardId: processMetaData.cardId,
                processId: processMetaData.processId,
                startTime: Math.floor(session.domainRange.domainStart + timestampOffset),
                endTime: Math.ceil(Math.min(session.endTimeAll ?? 0, session.domainRange.domainEnd + timestampOffset)),
                dataSource: processMetaData.dataSource,
                timePerPx: session.domain.timePerPx,
            };
            const requestKey = createStatusParam('unit/threadTracesSummary', requestParam);
            try {
                const request = await session.simpleCache.tryFetchFromCache('unit/threadTracesSummary', requestKey, requestParam);
                if (request === undefined) {
                    return [];
                }
                const threadTraceList = request.data as ProcessData[];
                const res: StatusData[] = [];
                // 泳道chart返回数据减去时间偏移
                threadTraceList.forEach((data) => {
                    res.push({
                        startTime: data.startTime - timestampOffset,
                        duration: data.duration,
                        name: '',
                        type: '',
                        color: '#7d7d7d',
                    });
                });
                return res;
            } catch (e) {
                console.warn('request process data failed', e);
                return [];
            }
        },
        config: {
            rowHeight: UnitHeight.STANDARD,
        },
        height: UnitHeight.UPPER,
    }),
    renderInfo: (_, metadata: ProcessMetaData, thisUnit) => {
        return isPinned(thisUnit) && !isSonPinned(thisUnit) ? `${metadata.cardId}_${metadata.processName} (${metadata.processId})` : `${metadata.processName} (${metadata.processId})`;
    },
});

export const CardUnit = unit<CardMetaData>({
    name: 'Card',
    configBar: offsetConfig,
    pinType: 'copied',
    renderInfo: (session: Session, metadata: { cardId: string; cardPath: string}) => <StyledTooltip placement="leftBottom" title={metadata.cardPath}><span style={{ marginLeft: 8, overflow: 'hidden', textOverflow: 'ellipsis', whiteSpace: 'nowrap' }}>{metadata.cardId}</span></StyledTooltip>,
    spreadUnits: on(
        'create',
        async (self): Promise<void> => {
        }),
});

export const CounterUnit = unit<CounterMetaData>({
    name: 'Counter',
    pinType: 'move',
    renderInfo: (session: Session, metadata) => `${metadata.threadName}`,
    chart: chart({
        type: 'filledLine',
        height: UnitHeight.SUPER_UPPER,
        mapFunc: async (session: Session, metadata) => {
            const countMetaData = metadata as CounterMetaData;
            // 查询泳道chart参数加上时间偏移
            const requestParam = {
                rankId: countMetaData.cardId,
                pid: countMetaData.processId,
                threadName: countMetaData.threadName,
                startTime: session.domainRange.domainStart,
                endTime: Math.min(session.endTimeAll ?? 0, session.domainRange.domainEnd),
                dataSource: countMetaData.dataSource,
                timePerPx: session.domain.timePerPx,
            };
            const requestKey = createCounterParam('unit/counter', requestParam);
            const request = await session.simpleCache.tryFetchFromCache('unit/counter', requestKey, requestParam, metadata);
            return request?.data as number[][];
        },
        config: (session: Session, metadata) => {
            const palette: Array<keyof Theme['colorPalette']> = [];
            (metadata as CounterMetaData).dataType.forEach(item => {
                palette.push(colorPalette[hashToNumber(item + (metadata as CounterMetaData).threadName, colorPalette.length)]);
            });
            return {
                palette,
            };
        },
        renderTooltip: (data, metadata) => {
            const tooltipMap = new Map();
            (metadata as CounterMetaData).dataType.forEach((item, index) => {
                tooltipMap.set(item, `${data[index + 1]}`);
            });
            return tooltipMap;
        },
    }),
});

const getFlowName = (res: any): string | undefined => {
    let flowName;
    if (res.type === 's') {
        flowName = 'Outgoing flow';
    } else if (res.type === 'f') {
        flowName = 'Incoming flow';
    }
    return flowName;
};

const useSliceRightDataUpdator = (session: Session, originDetail: LinkDataDesc<Record<string, unknown>>, linkFlow: unknown, metadata: unknown): Array<[string, string | JSX.Element]> | undefined => {
    const [renderFields, setRenderFields] = React.useState<Array<[string, string | JSX.Element]>>();
    const { selectedUnits } = session;
    const selectedUnit = selectedUnits.length > 0 ? selectedUnits[0] : undefined;
    const detail = (session.linkDetail as LinkDataDesc<Record<string, unknown>>) ?? originDetail;
    const fetchData = session.phase === 'error' ? undefined : detail?.fetchData;
    const onDataFetched = React.useMemo(() => ([selectedUnits, linkFlow].filter(_.isEmpty).length === 0
        ? fetchData?.(session, selectedUnit?.metadata as MetaData)
        : undefined), [selectedUnits, linkFlow, detail, session.linkDetail]);

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
                            isHiden(res) && state.push([templateField[0], render(res, session, metadata)]);
                        } else {
                            state.push([getFlowName(res) ?? templateField[0], render(res, session, metadata)]);
                        }
                    });
                } else {
                    detail.renderFields.forEach(renderField => {
                        const render = renderField[1];
                        if (renderField[2] !== undefined) {
                            const isHiden = renderField[2];
                            isHiden(result) && state.push([renderField[0], render(result, session, metadata)]);
                        } else {
                            state.push([renderField[0], render(result, session, metadata)]);
                        }
                    });
                }
                setRenderFields(state);
            });
        }
    };

    React.useEffect(loadData, [selectedUnits, linkFlow, detail, session.linkDetail]);
    return renderFields;
};

export const SliceRight = observer(({ session, detail, metadata }: { session: Session; detail: LinkDataDesc<Record<string, unknown>>; metadata: unknown }) => {
    const renderFields = useSliceRightDataUpdator(session, detail, session.linkFlow, metadata);
    return <SelectedDataBase renderer={renderFields} hasTitle />;
});

interface OpData {
    timestamp: number;
    duration: number;
}

const colums = [
    { title: 'index', dataIndex: 'index', ellipsis: true, width: 60 },
    { title: 'timestamp', dataIndex: 'timestamp', ...getDefaultColumData('time') },
    { title: 'duration', dataIndex: 'duration', ...getDefaultColumData('duration') },
];
// eslint-disable-next-line max-lines-per-function
export const SliceRightOpDetail = observer(({ session, metadata }: { session: Session; metadata: unknown }) => {
    const defaultPage = { current: 1, pageSize: 10, total: 0 };
    const defaultSorter = { field: 'duration', order: 'descend' };
    const [dataSource, setDataSource] = useState<any[]>([]);
    const [page, setPage] = useState(defaultPage);
    const [sorter, setSorter] = useState(defaultSorter);
    const [isLoading, setLoading] = useState(false);
    const slice = useMemo(() => session.selectedMultiSlice === '' ? undefined : JSON.parse(session.selectedMultiSlice), [session.selectedMultiSlice]);

    const jumpTo = async (record: OpData): Promise<void> => {
        if (slice === undefined) {
            return;
        }
        const params = {
            rankId: slice.rankId,
            name: slice.name,
            timestamp: record.timestamp,
        };
        const res = await window.requestData('unit/one/kernelDetail', params, 'timeline');
        runInAction(() => {
            session.locateUnit = {
                target: (iunit: InsightUnit): boolean => {
                    return (iunit.metadata as MetaData).threadId === res.threadId && (iunit.metadata as MetaData).processId === res.pid;
                },
                onSuccess: (): void => {
                    const selectedMultiSlice = JSON.parse(session.selectedMultiSlice);
                    const [rangeStart, rangeEnd] = calculateDomainRange(session, record.timestamp, record.duration);
                    session.domainRange = { domainStart: rangeStart, domainEnd: rangeEnd };
                    session.selectedData = {
                        startTime: record.timestamp,
                        name: selectedMultiSlice.name,
                        duration: record.duration,
                        depth: res.depth,
                        threadId: res.threadId,
                        startRecordTime: session.startRecordTime,
                        showSelectedData: false,
                    };
                },
                showDetail: false,
            };
        });
    };

    async function queryDetails(): Promise<void> {
        if (slice === undefined || slice.name === 'Totals') {
            setDataSource([]);
            return;
        }
        const params = {
            ...slice,
            ...sorter,
            ...page,
            orderBy: sorter.field,
        };
        const res = await window.requestData('query/all/same/operators/duration', params, 'timeline');
        const { sameOperatorsDetails = [], count, currentPage, pageSize } = res;
        setDataSource((sameOperatorsDetails as OpData[]).map((item, index) => ({ ...item, index: ((currentPage - 1) * pageSize) + index + 1 })));
        setPage({ total: count, current: currentPage, pageSize });
        setLoading(false);
    }

    useEffect(() => {
        if (page.current !== 1) {
            setPage({ ...page, current: 1 });
        } else {
            queryDetails();
        }
    }, [slice]);

    useEffect(() => {
        queryDetails();
    }, [sorter.field, sorter.order, page.current, page.pageSize]);
    return <div style={{ height: '100%', overflow: 'auto', padding: '5px 5px 15px 5px' }}>
        <ResizeTable
            onChange={(pagination: unknown, filters: unknown, newsorter: unknown, extra: {action: string}): void => {
                if (extra.action === 'sort') {
                    setSorter(newsorter as typeof sorter);
                }
            }}
            pagination={GetPageData(page, setPage)}
            dataSource={dataSource}
            columns={colums}
            size="small"
            loading = {isLoading}
            onRow={(record: OpData): {onClick: () => void} => {
                return {
                    onClick: (): void => {
                        jumpTo(record);
                    },
                };
            }}
            rowClassName={'click-able'}
        />
    </div>;
});
