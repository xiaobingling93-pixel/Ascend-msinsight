/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { MutableRefObject, useCallback, useEffect, useMemo, useRef, useState } from 'react';
import { queryParallelismArrangementCancelable } from '../../utils/RequestUtils';
import { ParallelismArrangementResult } from '../../utils/interface';
import { Session } from '../../entity/session';
import { observer } from 'mobx-react';
import { CanvasDrawer, Frame, Line, Rectangle } from './shape';
import { TooltipComponent } from './Tooltips';
import { GenerateConditions } from './CommunicatorContainer';
import { runInAction } from 'mobx';
import { getDyeingColor, groupFrames } from './ContainerUtils';
import eventBus from '../../utils/eventBus';
import styled from '@emotion/styled';
import { useParallelSwitchConditions } from './Context';
import { useTheme } from '@emotion/react';
import { throttle } from 'lodash';
import { Responsive } from 'ascend-components';
import { useTranslation } from 'react-i18next';
import { Spin } from 'antd';

const CanvasContainer = styled.div`
    max-height: 720px;
    overflow: auto;
    margin-top: 10px;
    background-color: ${(props): string => props.theme.rankContainerBackgroudColor};
`;

const Canvas = styled.canvas`
    position: absolute;
    top: 0;
    pointer-events: none;
    width: 100%;
    height: 100%;
`;

export const Loading = styled.div`
    z-index: 1;
    display: flex;
    position: absolute;
    width: 100%;
    height: 100%;
    background-color: ${(props): string => props.theme.maskColor};
    color: ${(props): string => props.theme.textColorPrimary};

    > div {
        margin: auto;
    }
`;

const resetPerformanceConditions = (): void => {
    eventBus.emit('resetPerformanceConditions');
};

interface UseFetchDataReturns {
    loading: boolean;
    isUpdated: MutableRefObject<boolean>;
    data?: ParallelismArrangementResult;
    error?: Error;
}

const useFetchData = (params: GenerateConditions | null): UseFetchDataReturns => {
    const [data, setData] = useState<ParallelismArrangementResult>();
    const [loading, setLoading] = useState<boolean>(false);
    const [error, setError] = useState<Error>();
    const isUpdated = useRef(false);

    const fetchData = async (): Promise<void> => {
        if (params === null) {
            return;
        }

        const { invoke } = queryParallelismArrangementCancelable;
        try {
            setLoading(true);
            const res = await invoke(params);
            isUpdated.current = !isUpdated.current;
            setData(res);
        } catch (err) {
            setError(err as Error);
        } finally {
            setLoading(false);
        }
    };

    useEffect(() => {
        const isDefaultParams = params?.dpSize === 1 && params?.tpSize === 1 && params?.ppSize === 1 && params.cpSize === 1 && params?.epSize === 1;
        if (params !== null && !isDefaultParams) {
            fetchData();
        }
    }, [JSON.stringify(params)]);

    return { data, loading, error, isUpdated };
};

interface ParallelismGraphProps {
    session: Session;
    generateConditions: GenerateConditions | null;
}
export const ParallelismGraph = observer(({ session, generateConditions }: ParallelismGraphProps): JSX.Element => {
    const canvasContainerRef = useRef<HTMLDivElement>(null);
    const mainCanvasRef = useRef<HTMLCanvasElement>(null);
    const hoverCanvasRef = useRef<HTMLCanvasElement>(null);
    const lastMousePositionRef = useRef<{ x: number; y: number }>({ x: 0, y: 0 });
    const [canvasDrawer, setCanvasDrawer] = useState<CanvasDrawer | null>(null);
    const [lastRect, setLastRect] = useState<Rectangle>();
    const [activeRectIndex, setActiveRectIndex] = useState<number | null>(null);
    const [hoveredRectIndex, setHoveredRectIndex] = useState<number | null>(null);
    const [responsiveSize, setResponsiveSize] = useState({ width: 0, height: 0 });
    const { parallelTypeList, dyeingMode, startVal, endVal, reset: resetParallelSwitchConditions } = useParallelSwitchConditions();
    const theme = useTheme();
    const { data, loading, isUpdated } = useFetchData(generateConditions);
    const { tpSize = 1, dpSize = 1, cpSize = 1, epSize = 1, ppSize = 1, dimension } = generateConditions ?? {};
    const { t } = useTranslation('summary');

    const canvasSize = useMemo(() => {
        let width = 200;
        let height = 200;

        if (lastRect !== undefined) {
            const { x, y, width: rectWidth, height: rectHeight, textHeight } = lastRect;
            const actualWidth = x + rectWidth + CanvasDrawer.PADDING;
            const actualHeight = y + rectHeight + CanvasDrawer.PADDING + textHeight;

            width = Math.max(actualWidth, width);
            height = Math.max(actualHeight, height);
        }
        return { width, height };
    }, [lastRect]);

    // Tooltip内容
    const hoveredData = useMemo(
        () => {
            if (hoveredRectIndex === null || data === undefined) {
                return null;
            }

            const { name, index } = data.arrangements[hoveredRectIndex];
            const currentData = session.performanceDataMap?.get(hoveredRectIndex);
            if (currentData === undefined) {
                return null;
            }

            const updatedData: Record<string, any> = {};

            for (const indicatorItem of session.indicatorMap.values()) {
                const { key, name: indicatorName, unit } = indicatorItem;
                const value = currentData[key];
                if (value !== undefined) {
                    updatedData[t(indicatorName)] =
                        session.isCompare
                            ? <span>{value}<span className={currentData.diff[key] >= 0 ? 'positive-number' : 'negative-number'}>({currentData.diff[key]})</span> {unit}</span>
                            : `${value} ${unit}`;
                }
            }
            for (const key of Object.keys(currentData.commCompare)) {
                const diffValue = currentData.commDiff[key];
                const value = currentData.commCompare[key];
                const curName = dimension === 'ep-dp-pp-cp-tp' ? `${key.toUpperCase()}-${t('Communication')}` : `${key.toUpperCase()}-${t('Avg Communication')}`;
                updatedData[curName] = session.isCompare
                    ? <span>{value}<span className={diffValue >= 0 ? 'positive-number' : 'negative-number'}>({diffValue})</span> μs</span>
                    : `${value} μs`;
            }

            return {
                [t('Index')]: index,
                [t('Name')]: name,
                ...updatedData,
            };
        },
        [hoveredRectIndex],
    );

    const render = (): void => {
        const { scrollLeft = 0, scrollTop = 0 } = canvasContainerRef.current ?? {};
        canvasDrawer?.render(scrollLeft, scrollTop);
    };

    const addRectangles = (drawer: CanvasDrawer): void => {
        data?.arrangements.forEach(arrangement => {
            drawer?.addRectangle(
                new Rectangle({
                    index: arrangement.index,
                    rowAndCol: arrangement.position,
                    fillColor: dyeingMode === 'None'
                        ? undefined
                        : getDyeingColor({
                            session,
                            index: arrangement.index,
                            dyeingMode,
                            range: [startVal, endVal],
                        }),
                    backgroundColor: theme.bgColorLighter,
                    color: theme.textColorPrimary,
                    attribute: {
                        ...arrangement.attribute,
                        tpSize,
                        dpSize,
                        cpSize,
                        epSize,
                        ppSize,
                    },
                }));
        });
    };

    const addFrames = (drawer: CanvasDrawer): void => {
        if (drawer !== null && data !== undefined) {
            const framesGroup = groupFrames(data.arrangements, parallelTypeList, data.connections);

            drawer.clearFrames();
            framesGroup.forEach(frame => {
                const { type, list } = frame;
                const frameInst = new Frame(type, list, {
                    tpSize,
                    dpSize,
                    cpSize,
                    epSize,
                    ppSize,
                });

                drawer.addFrame(frameInst);
            });
        }
    };

    const addLines = (drawer: CanvasDrawer): void => {
        if (drawer !== null && data !== undefined && activeRectIndex !== null) {
            const linesGroup = data?.connections?.filter(connection => {
                const isPpLineInPpDimension = (dimension === 'ep-dp-pp-cp' || dimension === 'ep-dp-pp') &&
                    connection.type === 'pp';
                return connection.list.includes(activeRectIndex) && !isPpLineInPpDimension;
            }).map(item => {
                return {
                    type: item.type,
                    list: item.list.map(it => ({
                        index: it,
                        position: data.arrangements[it].position,
                        attribute: data.arrangements[it].attribute,
                    })),
                };
            }) ?? [];

            linesGroup?.forEach(line => {
                const { type, list } = line;
                const parallelismSize = {
                    tpSize,
                    dpSize,
                    cpSize,
                    epSize,
                    ppSize,
                };
                const lineInst = new Line(type, list, parallelismSize);
                drawer.addLine(lineInst);
            });
        }
    };

    useEffect(() => {
        if (mainCanvasRef.current && data !== undefined) {
            const drawer = new CanvasDrawer(mainCanvasRef, hoverCanvasRef);
            setCanvasDrawer(drawer);
            addRectangles(drawer);
            addFrames(drawer);
            setLastRect(drawer.rectangleList[drawer.rectangleList.length - 1]);
            setTimeout(() => {
                drawer.render(canvasContainerRef.current?.scrollLeft ?? 0, canvasContainerRef.current?.scrollTop ?? 0);
            });

            // 更新所有通信域数据、指标数据
            runInAction(() => {
                const connections = data.connections.map(item => item.list.toString());
                const ppConnections = data.connections.filter(item => item.type === 'pp').map(item => item.list.toString());
                const frames = groupFrames(data.arrangements, ['dp', 'ep', 'cp'])
                    .map(frameGroup => frameGroup.list.map(item => item.index).toString());

                session.communicationDomains = [...new Set([...connections, ...frames])];
                session.ppCommunicationDomains = ppConnections;
                session.indicatorList = data?.indicators.map(indicator => {
                    const unit = indicator.yAxisType === 'time' ? 'μs' : '%';
                    return {
                        ...indicator,
                        unit,
                    };
                }) ?? [];
                session.arrangementRankCount = data?.size || 0;
                session.setRankDyeingData();
            });

            setActiveRectIndex(null);
            resetParallelSwitchConditions();
            resetPerformanceConditions();
        }
    }, [isUpdated.current]);

    useEffect(() => {
        if (data !== undefined && canvasDrawer !== null) {
            canvasDrawer.clearRectangles();
            addRectangles(canvasDrawer);
            render();
        }
    }, [theme.mode, dyeingMode, startVal, endVal]);

    useEffect(() => {
        if (canvasDrawer !== null) {
            canvasDrawer.clearLines();
            addLines(canvasDrawer);
            render();
        }
    }, [activeRectIndex]);

    useEffect(() => {
        if (canvasDrawer !== null && data?.arrangements !== undefined) {
            addFrames(canvasDrawer);
            render();
        }
    }, [JSON.stringify(parallelTypeList)]);

    useEffect(() => {
        render();
    }, [responsiveSize.width, responsiveSize.height, canvasSize.width, canvasSize.height]);

    const onClickCanvas: React.MouseEventHandler<HTMLDivElement> = (event): void => {
        const { offsetX, offsetY } = event.nativeEvent;
        const { scrollLeft = 0, scrollTop = 0 } = canvasContainerRef.current ?? {};
        const x = offsetX - scrollLeft;
        const y = offsetY - scrollTop;

        if (canvasDrawer === null) {
            return;
        }

        // 判断矩形
        for (const rect of canvasDrawer.visibleRectangleList) {
            if (rect.isInside(x, y)) {
                setActiveRectIndex(rect.index);
                return;
            }
        }

        // 判断线
        for (const line of canvasDrawer.lineList) {
            if (line.isInside(x, y)) {
                eventBus.emit('activeCommunicator', {
                    name: line.type === 'pp' ? 'pipeline' : line.type,
                    value: line.rectList.map(rect => rect.index).join(','),
                });
                return;
            }
        }

        // 判断框
        for (const frame of canvasDrawer.frameList) {
            if (frame.isInside(x, y)) {
                eventBus.emit('activeCommunicator', {
                    name: frame.type,
                    value: frame.rectList.map(rect => rect.index).join(','),
                });
                return;
            }
        }

        setActiveRectIndex(null);
    };

    // 鼠标移入卡时，选中当前卡
    const setRectActive = useCallback(throttle((x, y): void => {
        const activeRect = canvasDrawer?.rectangleList.find(rect => rect.isInside(x, y));

        setHoveredRectIndex(activeRect === undefined ? null : activeRect.index);
    }, 100), [canvasDrawer]);

    const onMouseMove: React.MouseEventHandler<HTMLDivElement> = (event) => {
        const { offsetX, offsetY } = event.nativeEvent;
        const { scrollLeft = 0, scrollTop = 0 } = canvasContainerRef?.current ?? {};
        const x = offsetX - scrollLeft;
        const y = offsetY - scrollTop;

        setRectActive(x, y);
        lastMousePositionRef.current = { x, y };
        canvasDrawer?.renderHoverCanvas(x, y);
    };

    const onMouseOut: React.MouseEventHandler<HTMLDivElement> = (): void => {
        setHoveredRectIndex(null);
    };

    const onScroll = (): void => {
        requestAnimationFrame(render);

        // 获取当前鼠标位置
        const { x, y } = lastMousePositionRef.current;
        setRectActive(x, y);
    };

    const handleResize = (size: {width: number;height: number}): void => {
        setResponsiveSize(size);
    };

    return <div className="parallelism-graph" style={{ position: 'relative' }}>
        {loading && <Loading data-testid="parallelism-graph-loading"><Spin /></Loading>}
        <Responsive onChange={handleResize}>
            {
                ({ width, height }): React.ReactNode => {
                    return <CanvasContainer ref={canvasContainerRef} onScroll={onScroll}>
                        <Canvas
                            ref={mainCanvasRef}
                            width={width * devicePixelRatio}
                            height={height * devicePixelRatio}
                        ></Canvas>
                        <Canvas
                            ref={hoverCanvasRef}
                            width={width * devicePixelRatio}
                            height={height * devicePixelRatio}
                        ></Canvas>
                        <div
                            data-testid="parallelism-graph-placeholder"
                            style={{
                                width: canvasSize.width,
                                height: canvasSize.height,
                            }}
                            onClick={onClickCanvas}
                            onMouseMove={onMouseMove}
                            onMouseOut={onMouseOut}
                        ></div>
                    </CanvasContainer>;
                }
            }
        </Responsive>
        <TooltipComponent
            mouseX={lastMousePositionRef.current.x}
            mouseY={lastMousePositionRef.current.y}
            content={hoveredData}
        />
    </div>;
});
