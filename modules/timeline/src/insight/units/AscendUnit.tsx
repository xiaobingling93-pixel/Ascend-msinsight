/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { useTranslation } from 'react-i18next';
import {
    chart,
    on,
    singleData,
    unit,
    UnitHeight,
} from '../../entity/insight';
import type {
    ChartDesc, InsightUnit, LinkDataDesc, LinkLine, LinkLines, MetaData, renderFieldsType,
} from '../../entity/insight';
import type { Session } from '../../entity/session';
import { hashToNumber } from '../../utils/colorUtils';
import type {
    AscendSliceDetail,
    CardMetaData,
    CounterMetaData,
    ProcessData,
    ProcessMetaData,
    ThreadMetaData,
    ThreadTrace, HostMetaData, SliceMeta,
} from '../../entity/data';
import { createCounterParam, createStatusParam } from './unitFunc';
import { SelectedDataBottomPanel } from '../../components/SelectedDataBottomPanel';
import { SelectSimpleTabularDetail } from '../../components/details/SelectSimpleDetail';
import { renderRadiusBorder } from '../../components/details/utils';
import { generateFlowParam, generateSummary, slicesListDetail } from './details';
import { colorPalette, getTimeOffset } from './utils';
import React, { useEffect, useState, useMemo } from 'react';
import { observer } from 'mobx-react-lite';
import _ from 'lodash';
import { runInAction } from 'mobx';
import { SelectedDataBase } from '../../components/details/base/SelectedData';
import { offsetConfig } from './config/offsetConfig';
import { isPinned, isSonPinned } from '../../components/ChartContainer/unitPin';
import type { Theme } from '@emotion/react';
import type { ChartHandle, ChartType, Scale, StackStatusConfig, StackStatusData, StatusData } from '../../entity/chart';
import { Tooltip } from 'ascend-components';
import { ResizeTable } from 'ascend-resize';
import { getDefaultColumData, getPageData } from '../../components/detailViews/Common';
import { calculateDomainRange } from '../../components/CategorySearch';
import { safeJSONParse } from 'ascend-utils';

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
    const us = Math.floor((ns - (ms * 1000000)) / 1000);
    const nsRemainder = ns - (ms * 1000000) - (us * 1000);
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
    return `${nsToMs(startTime).toFixed(6).toString()} ms`;
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
        ['Title', (data): string => data.title === undefined ? '' : `${data.title}`, isHiddenTitle],
        ['Start', (data: AscendSliceDetail): string => getDetailTimeDisplay(data.startTime ?? 0), isHiddenStartTime],
        ['Wall Duration', (data): string => getDetailTimeDisplay(data.duration as number), isHiddenDuration],
        ['Self Time', (data): string => getDetailTimeDisplay(data.selfTime as number), isHiddenSelfTime],
        ['Input Shapes', (data: AscendSliceDetail): string => getDisplay(data.inputShapes), (data: AscendSliceDetail): boolean => isHidden(data.inputShapes)],
        ['Input Data Types', (data: AscendSliceDetail): string => getDisplay(data.inputDataTypes), (data: AscendSliceDetail): boolean => isHidden(data.inputDataTypes)],
        ['Input Formats', (data: AscendSliceDetail): string => getDisplay(data.inputFormats), (data: AscendSliceDetail): boolean => isHidden(data.inputDataTypes)],
        ['Output Shapes', (data: AscendSliceDetail): string => getDisplay(data.outputShapes), (data: AscendSliceDetail): boolean => isHidden(data.inputDataTypes)],
        ['Output Data Types', (data: AscendSliceDetail): string => getDisplay(data.outputDataTypes), (data: AscendSliceDetail): boolean => isHidden(data.inputDataTypes)],
        ['Output Formats', (data: AscendSliceDetail): string => getDisplay(data.outputFormats), (data: AscendSliceDetail): boolean => isHidden(data.inputDataTypes)],
        ['Attr Info', (data: AscendSliceDetail): string => getDisplay(data.attrInfo), (data: AscendSliceDetail): boolean => isHidden(data.attrInfo)],
    ],
    fetchData: async (session: Session, metadata: ThreadMetaData) => {
        const selectedSliceData = session.selectedData as ThreadTrace;
        const timestampOffset = getTimeOffset(session, metadata);
        // 因为泳道chart数据减去了偏移，所有点选的时候得把偏移加回来
        const params = {
            rankId: metadata.cardId,
            metaType: metadata.metaType,
            pid: metadata.processId,
            tid: metadata.threadId,
            id: selectedSliceData.id,
            startTime: Math.floor(selectedSliceData.startTime + timestampOffset),
            depth: selectedSliceData.depth,
            timePerPx: session.domain.timePerPx,
        };
        const result = await window.request(metadata.dataSource, { command: 'unit/threadDetail', params });
        const res = result?.data ?? {};
        const data: AscendSliceDetail = {
            pid: metadata?.processId,
            tid: metadata?.threadId,
            startTime: selectedSliceData?.startTime,
            depth: selectedSliceData?.depth,
            ...res,
        };
        return data;
    },
});

const EmptyJSXElement = (): JSX.Element | null => {
    return <></>;
};

interface FlowPoint {
    depth: number;
    duration: number;
    id: string;
    name: string;
    pid: string;
    tid: string;
    timestamp: number;
}

interface FlowEvent {
    cat: string;
    from: FlowPoint;
    to: FlowPoint;
    id: string;
    title: string;
}

interface CategoryFlows {
    cat: string;
    flows: FlowEvent[];
}

const drawRectBorder = (selectedData: ThreadTrace,
    session: Session, xScale: (num: number) => number, yScale: (num: number) => number, ctx: CanvasRenderingContext2D): void => {
    const duration = selectedData.duration < 0 ? session.endTimeAll as number : selectedData.startTime + selectedData.duration;
    const bottomRight = xScale(duration) - xScale(selectedData.startTime);
    renderRadiusBorder({
        topLeft: xScale(selectedData.startTime),
        topRight: yScale(0),
        bottomRight,
        bottomLeft: yScale(1),
        depth: selectedData.depth,
        ctx,
    });
};

interface DrawBorderArgs {
    item: Record<string, unknown>;
    threadMetaData: ThreadMetaData;
    session: Session;
    xScale: (num: number) => number;
    yScale: (num: number) => number;
    ctx: CanvasRenderingContext2D;
};

const drawSingleAlignSlice = ({ item, threadMetaData, session, xScale, yScale, ctx }: DrawBorderArgs): void => {
    const singleSliceData = item as ThreadTrace | undefined;
    if (singleSliceData === undefined) {
        return;
    }
    const singleMeta = item as SliceMeta;
    const alignCheck = singleMeta.cardId === threadMetaData.cardId &&
        singleMeta.processId === threadMetaData.processId &&
        singleMeta.threadId === threadMetaData.threadId;
    if (alignCheck) {
        drawRectBorder(singleSliceData, session, xScale, yScale, ctx);
    }
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
        mapFunc: async (session: Session, metaData: unknown, thisUnit?: InsightUnit) => {
            const threadMetaData = metaData as ThreadMetaData;
            // 查询泳道chart参数加上时间偏移
            const timestampOffset = getTimeOffset(session, threadMetaData);
            const key = threadMetaData.cardId !== undefined ? `${threadMetaData.cardId}_${threadMetaData.threadName}` : null;
            const isFilterPythonFunction = key !== null ? (session?.unitsConfig.filterConfig.pythonFunction as Record<string, boolean>)?.[key] ?? false : false;
            const requestParam: Record<string, unknown> & { timePerPx: number } = {
                cardId: threadMetaData.cardId,
                processId: threadMetaData.processId,
                threadId: threadMetaData.threadId,
                metaType: threadMetaData.metaType,
                startTime: Math.floor(session.domainRange.domainStart + timestampOffset),
                endTime: Math.ceil(session.domainRange.domainEnd + timestampOffset),
                dataSource: threadMetaData.dataSource,
                timePerPx: session.domain.timePerPx,
                isFilterPythonFunction,
                isHideFlagEvents: session.areFlagEventsHidden,
            };
            try {
                const request = await window.request(requestParam.dataSource as DataSource, { command: 'unit/threadTraces', params: requestParam });
                if (request === undefined) {
                    return [];
                }

                const { data: threadTraceList, maxDepth, currentMaxDepth, havePythonFunction } = request;
                if (thisUnit) {
                    const activeMaxDepth = session.autoAdjustUnitHeight ? currentMaxDepth : maxDepth;
                    updateUnitData(thisUnit, activeMaxDepth, havePythonFunction);
                }
                // 泳道chart返回数据减去时间偏移
                return _.map(threadTraceList, (it) => _.map(it, (data) => {
                    let uintColor;
                    if (session.isSimulation) {
                        uintColor = colorPalette[hashToNumber(data.cname, colorPalette.length)];
                    } else {
                        uintColor = colorPalette[hashToNumber(data.name, colorPalette.length)];
                    }
                    return {
                        startTime: data.startTime - timestampOffset,
                        originalStartTime: data.startTime,
                        duration: data.duration,
                        name: data.name,
                        type: data.name,
                        color: uintColor,
                        depth: data.depth,
                        threadId: data.threadId,
                        cardId: threadMetaData.cardId,
                        cname: data.cname,
                        id: data.id,
                    } as StackStatusData;
                }));
            } catch (e) {
                return [];
            }
        },
        decorator: (session: Session, metaData: unknown) => {
            return {
                action: async (handle, xScale, yScale, theme): Promise<void> => {
                    maskedNotSelectData(session, handle, xScale, yScale);
                    // click
                    const ctx = handle.context;
                    const selectedData = session.selectedData as ThreadTrace | undefined;
                    const selectedUnitMetaData = session.selectedUnits?.[0]?.metadata as ThreadMetaData;
                    const threadMetaData = metaData as ThreadMetaData;
                    if (ctx === null) {
                        return;
                    }
                    const check = selectedUnitMetaData !== undefined && selectedData !== undefined && selectedUnitMetaData === threadMetaData;
                    // 来自本泳道点击的数据，给数据描边+画线
                    ctx.strokeStyle = theme.textColorPrimary;
                    if (check) {
                        drawRectBorder(selectedData, session, xScale, yScale, ctx);
                    }
                    const benchMarkData = session.benchMarkData as ThreadTrace | undefined;
                    if (benchMarkData === undefined) {
                        return;
                    }
                    const benchMarkMeta = session.benchMarkData as SliceMeta;
                    const benchCheck = benchMarkMeta.cardId === threadMetaData.cardId &&
                        benchMarkMeta.processId === threadMetaData.processId &&
                        benchMarkMeta.threadId === threadMetaData.threadId;
                    if (benchCheck) {
                        drawRectBorder(benchMarkData, session, xScale, yScale, ctx);
                    }
                    if (session.alignSliceData === undefined) {
                        return;
                    }
                    session.alignSliceData.forEach((item: Record<string, unknown>) => {
                        drawSingleAlignSlice({ item, threadMetaData, session, xScale, yScale, ctx });
                    });
                },
                triggers: [
                    session.selectedData,
                    session.selectedData?.duration,
                    session?.searchData,
                    session.alignRender,
                ],
            };
        },
        onClick: async (data, session, metadata) => {
            if (data === undefined) { return; }
            const linkFlow = generateFlowParam(metadata as ThreadMetaData, data);
            linkFlow.isSimulation = session.isSimulation;
            const timestampOffset = getTimeOffset(session, metadata as ThreadMetaData);
            linkFlow.startTime = timestampOffset + (linkFlow.startTime as number);
            linkFlow.endTime = timestampOffset + (linkFlow.endTime as number);
            const raw = await window.request((metadata as ThreadMetaData).dataSource as DataSource, { command: 'unit/flows', params: linkFlow as Record<string, unknown> }) as any;
            const categoryFlowEvents = raw.unitAllFlows as CategoryFlows[] ?? [];
            const newLines: LinkLines = {};
            for (const categoryFlowEvent of categoryFlowEvents) {
                const cat = categoryFlowEvent.cat;
                const singleCatLinkLine: LinkLine = [];
                for (const flow of categoryFlowEvent.flows) {
                    const singleLine: Record<string, unknown> = {
                        category: flow.cat,
                        cardId: linkFlow.rankId,
                        from: flow.from,
                        to: flow.to,
                    };
                    singleCatLinkLine.push(singleLine);
                }
                newLines[cat] = singleCatLinkLine;
            }
            runInAction(() => {
                session.selectedData = { ...data, threadId: (metadata as ThreadMetaData).threadId, processId: (metadata as ThreadMetaData).processId };
                session.linkLines = newLines;
                session.singleLinkLine = newLines;
                session.renderTrigger = !session.renderTrigger;
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
    bottomPanelRender: (newSession: Session, metadata) => {
        return [
            {
                Detail: ({ session }): JSX.Element => <SelectedDataBottomPanel
                    session={session} detail={singleSliceDetail}>{EmptyJSXElement}</SelectedDataBottomPanel>,
            },
            {
                Detail: ({ session, height }): JSX.Element => <SelectSimpleTabularDetail
                    session={session} height={height} detail={slicesListDetail} summaryBuilder={generateSummary}></SelectSimpleTabularDetail>,
                More: (): JSX.Element => <SliceRightOpDetail session={newSession} metadata={metadata} />,
                moreWh: 320,
            },
        ];
    },
    collapseAction: (insightUnit) => {
        const chartDesc = (insightUnit.chart as ChartDesc<ChartType>);
        const config = (insightUnit.chart as ChartDesc<ChartType>).config;
        runInAction(() => {
            (config as any).isCollapse = !((config as any).isCollapse as boolean);
            const collapseHeight = UnitHeight.COLL;
            const expandedHeight = (config as any).maxDepth * (config as any).rowHeight;
            chartDesc.height = ((config as any).isCollapse as boolean) ? collapseHeight : expandedHeight;
        });
    },
});

const recoverHistory = (currentUnit: InsightUnit, threadTraceMaxDepth: number): void => {
    const currentChart = currentUnit.chart as ChartDesc<'stackStatus'>;
    const config = currentChart.config as StackStatusConfig;
    if (currentUnit.onceExpand !== undefined) {
        currentUnit.isExpanded = currentUnit.onceExpand;
        if (currentUnit.collapsible) {
            config.isCollapse = !currentUnit.onceExpand;
        }
        delete currentUnit.onceExpand;
        currentChart.height = config.isCollapse ? UnitHeight.COLL : threadTraceMaxDepth * config.rowHeight;
        config.maxDepth = threadTraceMaxDepth;
    }
};

const updateUnitData = (currentUnit: InsightUnit, threadTraceMaxDepth: number, havePythonFunction: boolean): void => {
    const currentChart = currentUnit.chart as ChartDesc<'stackStatus'>;
    const config = currentChart.config as StackStatusConfig;
    runInAction(() => {
        if (threadTraceMaxDepth) {
            // 根据该接口返回的最大深度重新渲染泳道高度
            if (threadTraceMaxDepth > 1 && !currentUnit.collapsible) {
                currentUnit.collapsible = true;
                currentUnit.isExpanded = true;
            }
            // 恢复历史数据
            recoverHistory(currentUnit, threadTraceMaxDepth);
            if (threadTraceMaxDepth !== config.maxDepth) {
                currentChart.height = config.isCollapse ? UnitHeight.COLL : threadTraceMaxDepth * config.rowHeight;
                config.maxDepth = threadTraceMaxDepth;
            }
            currentUnit.havePythonFunction = havePythonFunction;
        }
    });
};

function maskedNotSelectData<T extends ChartType>(session: Session, handle: ChartHandle<T>, xScale: Scale, yScale: Scale): void {
    if (session.searchData) {
        const name = session.searchData.content;
        const isAble = (item: any): boolean => {
            if (session.searchData?.isMatchCase === undefined) {
                return false;
            }
            const it = item as {name: string};
            if (session.searchData.isMatchExact && session.searchData.isMatchCase) {
                return it.name !== name;
            }
            if (session.searchData.isMatchExact) {
                return it.name?.toLocaleLowerCase() !== session.searchData.content.toLocaleLowerCase();
            } else if (session.searchData.isMatchCase) {
                return !it.name?.includes(name);
            } else {
                return !it.name?.toLocaleLowerCase().includes(session.searchData.content.toLocaleLowerCase());
            }
        };
        const data = handle.findAll(isAble).map((it: any) => it.map((notSelectedData: any) => ({
            ...notSelectedData,
            color: 'transparentMask' as const,
        })));
        handle.draw(data, xScale, yScale);
    }
}

const SummaryChart = chart({
    type: 'status',
    mapFunc: async (session: Session, metaData: unknown) => {
        const processMetaData = metaData as ProcessMetaData;
        const timestampOffset = getTimeOffset(session, processMetaData);
        const requestParam = {
            cardId: processMetaData.cardId,
            processId: processMetaData.processId,
            metaType: processMetaData.metaType,
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
                    color: '',
                });
            });
            return res;
        } catch (e) {
            return [];
        }
    },
    config: {
        rowHeight: UnitHeight.STANDARD,
    },
    height: UnitHeight.UPPER,
});

export const ProcessUnit = unit<ProcessMetaData>({
    name: 'Process',
    configBar: (session: Session, metadata: ProcessMetaData) => {
        // Host侧第三级泳道不显示offset
        if ((metadata as ThreadMetaData).threadId !== '') {
            return <></>;
        }
        return offsetConfig(session, metadata);
    },
    tag: (session: Session, metadata: { label?: string }) => metadata.label === undefined ? '' : `${metadata.label}`,
    pinType: 'copied',
    chart: SummaryChart,
    renderInfo: (session: Session, metadata: ProcessMetaData, thisUnit) => {
        return isPinned(thisUnit) && !isSonPinned(thisUnit) ? `${metadata.cardId}_${metadata.processName}` : `${metadata.processName}`;
    },
});

export const LabelUnit = unit<ProcessMetaData>({
    name: 'Label',
    tag: (session: Session, metadata: { label?: string }) => metadata.label === undefined ? '' : `${metadata.label}`,
    pinType: 'copied',
    renderInfo: (session: Session, metadata: ProcessMetaData, thisUnit) => {
        return isPinned(thisUnit) && !isSonPinned(thisUnit) ? `${metadata.cardId}_${metadata.processName} (${metadata.processId})` : `${metadata.processName}`;
    },
});

export const CardUnit = unit<CardMetaData>({
    name: 'Card',
    configBar: offsetConfig,
    pinType: 'copied',
    renderInfo: (session: Session, metadata: { cardName: string; cardPath: string }) => <Tooltip placement="leftBottom" title={metadata.cardPath}>
        <span style={{ marginLeft: 8, overflow: 'hidden', textOverflow: 'ellipsis', whiteSpace: 'nowrap' }}>
            {metadata.cardName}
        </span>
    </Tooltip>,
    spreadUnits: on(
        'create',
        async (self): Promise<void> => {
        }),
});

export const ROOT_UNIT = unit<HostMetaData>({
    name: 'Root',
    pinType: 'copied',
    renderInfo: (session: Session, metadata: { host: string }) => metadata.host,
});

export const CounterUnit = unit<CounterMetaData>({
    name: 'Counter',
    pinType: 'move',
    collapsible: false,
    renderInfo: (session: Session, metadata) => `${metadata.threadName}`,
    chart: chart({
        type: 'filledLine',
        height: UnitHeight.SUPER_UPPER,
        mapFunc: async (session: Session, metadata) => {
            const countMetaData = metadata as CounterMetaData;
            const timestampOffset = getTimeOffset(session, countMetaData);
            // 查询泳道chart参数加上时间偏移
            const requestParam = {
                rankId: countMetaData.cardId,
                pid: countMetaData.processId,
                threadName: countMetaData.threadName,
                threadId: countMetaData.threadId,
                metaType: countMetaData.metaType,
                startTime: 0,
                endTime: session.endTimeAll,
                dataSource: countMetaData.dataSource,
                timePerPx: session.domain.timePerPx,
            };
            const requestKey = createCounterParam('unit/counter', requestParam);
            const request = await session.simpleCache.tryFetchFromCache('unit/counter', requestKey, requestParam, metadata);
            const res = request?.data as number[][];
            return res.map(([timestamp, ...rest]) => [timestamp - timestampOffset, ...rest]);
        },
        config: (session: Session, metadata) => {
            const palette: Array<keyof Theme['colorPalette']> = [];
            const countMetaData = metadata as CounterMetaData;
            countMetaData.dataType.forEach((item, index): void => {
                const colorIndex = hashToNumber(`${item}${countMetaData.threadName}`, colorPalette.length);
                const color = colorPalette[colorIndex];
                if (color === palette[index - 1]) {
                    // 相邻色值不能相同，否则堆叠图无法区分数据
                    palette.push(colorPalette[(colorIndex + 1) % colorPalette.length]);
                } else {
                    palette.push(color);
                }
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
    } else {
        flowName = undefined;
    }
    return flowName;
};

const handleArrayResult = (result: Array<Record<string, unknown>>, templateField: renderFieldsType<Record<string, unknown>>,
    state: Array<[string, string | JSX.Element]>, metadata: unknown): void => {
    result.forEach(res => {
        const render = templateField[1];
        if (templateField[2] !== undefined) {
            const isHiden = templateField[2];
            if (isHiden(res)) { state.push([templateField[0], render(res, session, metadata)]); }
        } else {
            state.push([getFlowName(res) ?? templateField[0], render(res, session, metadata)]);
        }
    });
};

const handleNonArrayResult = (result: Record<string, unknown>, state: Array<[string, (string | JSX.Element)]>,
    detailDesc: LinkDataDesc<Record<string, unknown>>, metadata: unknown): void => {
    detailDesc.renderFields.forEach(renderField => {
        const render = renderField[1];
        if (renderField[2] !== undefined) {
            const isHiden = renderField[2];
            if (isHiden(result)) { state.push([renderField[0], render(result, session, metadata)]); }
        } else {
            state.push([renderField[0], render(result, session, metadata)]);
        }
    });
};

const useSliceRightDataUpdator = (session: Session, originDetail: LinkDataDesc<Record<string, unknown>>,
    linkFlow: unknown, metadata: unknown): Array<[string, string | JSX.Element]> | undefined => {
    const [renderFields, setRenderFields] = React.useState<Array<[string, string | JSX.Element]>>();
    const { selectedUnits } = session;
    const selectedUnit = selectedUnits.length > 0 ? selectedUnits[0] : undefined;
    const detail = (session.linkDetail as unknown as LinkDataDesc<Record<string, unknown>>) ?? originDetail;
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
                if (recentUnits.current !== selectedUnits || recentData.current !== linkFlow) { return; }
                const state: Array<[string, string | JSX.Element]> = [];
                if (Array.isArray(result)) {
                    const templateField = detail.templateField;
                    if (!templateField) { return; }
                    handleArrayResult(result, templateField, state, metadata);
                } else {
                    handleNonArrayResult(result, state, detail, metadata);
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
    startTime: string;
    timestamp: number;
    duration: number;
    id: string;
    depth: number;
    tid: string;
    pid: string;
    metaType?: string;
}

const useColumns = (): any => {
    const { t } = useTranslation('timeline', { keyPrefix: 'sliceList' });
    return [
        { title: t('Index'), dataIndex: 'index', ellipsis: true, width: 60 },
        { title: t('Timestamp'), dataIndex: 'startTime', ...getDefaultColumData('time') },
        { title: t('Duration(ns)'), dataIndex: 'duration', ...getDefaultColumData('duration') },
    ];
};
// eslint-disable-next-line max-lines-per-function
export const SliceRightOpDetail = observer(({ session, metadata }: { session: Session; metadata: unknown }) => {
    const defaultPage = { current: 1, pageSize: 10, total: 0 };
    const defaultSorter = { field: 'duration', order: 'descend' };
    const [dataSource, setDataSource] = useState<any[]>([]);
    const [page, setPage] = useState(defaultPage);
    const [sorter, setSorter] = useState(defaultSorter);
    const [isLoading, setLoading] = useState(false);
    const slice = useMemo(() => session.selectedMultiSlice === '' ? undefined : safeJSONParse(session.selectedMultiSlice), [session.selectedMultiSlice]);
    const [selectedRowKey, setSelectedRowKey] = useState('');

    const jumpTo = async (record: OpData): Promise<void> => {
        if (slice === undefined) {
            return;
        }
        runInAction(() => {
            session.locateUnit = {
                target: (iunit: InsightUnit): boolean => {
                    return iunit instanceof ThreadUnit && (Boolean(iunit.metadata.cardId === (metadata as MetaData).cardId)) &&
                    (iunit.metadata as MetaData).threadId === record.tid && (iunit.metadata as MetaData).processId === record.pid;
                },
                onSuccess: (iunit): void => {
                    const selectedMultiSlice = safeJSONParse(session.selectedMultiSlice);
                    const startTime = record.timestamp - getTimeOffset(session, iunit.metadata as ThreadMetaData);
                    const [rangeStart, rangeEnd] = calculateDomainRange(session, startTime, record.duration);
                    session.domainRange = { domainStart: rangeStart, domainEnd: rangeEnd };
                    session.selectedData = {
                        id: record.id,
                        startTime,
                        name: selectedMultiSlice.name,
                        color: colorPalette[hashToNumber(selectedMultiSlice.name, colorPalette.length)],
                        duration: record.duration,
                        depth: record.depth,
                        threadId: record.tid,
                        startRecordTime: session.startRecordTime,
                        showSelectedData: true,
                    };
                },
                showDetail: false,
            };
        });
    };

    async function queryDetails(): Promise<void> {
        if (slice === undefined || slice.name === 'Totals') {
            setDataSource([]);
            setPage(defaultPage);
            return;
        }
        const params = {
            ...slice,
            ...sorter,
            ...page,
            orderBy: sorter.field === 'startTime' ? 'timestamp' : sorter.field,
        };
        const res = await window.requestData('query/all/same/operators/duration', params, 'timeline');
        const { currentPage, pageSize, sameOperatorsDetails } = res;
        const data = sameOperatorsDetails as OpData[];
        const timestampoffset = getTimeOffset(session, metadata as ThreadMetaData);
        data.forEach(item => {
            item.startTime = getDetailTimeDisplay(item.timestamp - timestampoffset);
            item.tid = slice.tid;
            item.pid = slice.pid;
        });
        setDataSource((data).map((item, index) => ({ ...item, index: ((currentPage - 1) * pageSize) + index + 1 })));
        setPage({ total: slice.count, current: currentPage, pageSize });
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
            pagination={getPageData(page, setPage)}
            dataSource={dataSource}
            columns={useColumns()}
            size="small"
            loading = {isLoading}
            onRow={(record: OpData): {onClick: () => void} => {
                return {
                    onClick: (): void => {
                        jumpTo(record);
                        setSelectedRowKey(record.id);
                    },
                };
            }}
            rowClassName={(record: OpData): string => {
                return record.id === selectedRowKey ? 'selected-row' : 'click-able';
            }}
        />
    </div>;
});
