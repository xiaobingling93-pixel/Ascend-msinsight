import React, { RefObject, useEffect, useMemo, useRef, useState } from 'react';
import { ChartData, ChartDataEle, ChartType, MapFunc } from '../../entity/chart';
import { Session } from '../../entity/session';
import { Logger } from '../../utils/Logger';
import { autorun, runInAction } from 'mobx';
import { InsightUnit } from '../../entity/insight';

export type Pos = {
    x: number;
    y: number;
};

export type DataProcessor<T extends ChartType> = (data: ChartData<T>, width: number, domainStart: number, domainEnd: number) => ChartData<T>;

/**
 * Manages the data that are to be rendered as a state.
 *
 * @param session session
 * @param mapFunc mapFunc
 * @param metadata metadata
 * @param width width
 * @returns the data that this chart is currently rendering
 */
export const useData = <T extends ChartType>(session: Session, mapFunc: MapFunc<T>, metadata: unknown, width: number, processor?: DataProcessor<T>): ChartData<T> => {
    const { domainStart, domainEnd } = session.domainRange;
    const { endTimeAll } = session;
    const [ datasState, setDatasState ] = useState<ChartData<T>>([]);
    const requestedWidth = useRef(0);
    useEffect(() => {
        if (width === 0) {
            setDatasState([]);
            return;
        }
        requestedWidth.current = width;
        mapFunc(session, metadata).then(datas => {
            if (requestedWidth.current !== width) {
                // drop the data if width has been changed since when request was made
                return;
            }
            // the datas should be sorted by startTime(min -> max).
            setDatasState(processor?.(datas, width, domainStart, domainEnd) ?? datas);
        }).catch(() => {
            Logger('hooks useData', 'mapFunc occurred an exception.');
        });
    }, [ session.phase, domainStart, domainEnd, endTimeAll, width ]);
    return datasState;
};

export const useRangeAndDomain = (session: Session, width: number, margin: number): Array<[number, number]> => {
    const { domainStart, domainEnd } = session.domainRange;
    return useMemo<Array<[number, number]>>(
        () => [ [ margin, width - 2 * margin ], [ domainStart, domainEnd ] ],
        [ domainStart, domainEnd, width, margin ],
    );
};

/**
 * Calls renderer when deps are changed, but limit the frequency
 *
 * @param renderer the render function
 * @param deps the dependencies that triggers re-render
 */
export const useBatchedRender = (renderer: () => void, deps: React.DependencyList): void => {
    const [ render, setRender ] = useState(false);
    useEffect(() => {
        setRender(true);
    }, deps);
    useEffect(() => {
        if (!render) { return; }
        renderer();
        setRender(false);
    }, [render]);
};

export const useHoverPosX = (ref: React.RefObject<HTMLElement>): number | undefined => {
    return useHoverPos(ref)?.x ?? undefined;
};

export const useHoverPos = (ref: React.RefObject<HTMLElement>): Pos | undefined => {
    const [ mousePos, setMousePos ] = useState<Pos>();
    const onMouseMove = (e: MouseEvent): void => {
        if ((e.target as HTMLElement).className === 'clickable') {
            setMousePos(undefined);
        } else {
            setMousePos({ x: e.offsetX, y: e.offsetY });
        }
    };
    const onMouseOut = (e: MouseEvent): void => { setMousePos(undefined); };
    useEffect(() => {
        if (ref.current === null) {
            return;
        }
        ref.current.addEventListener('mousemove', onMouseMove);
        ref.current.addEventListener('mouseout', onMouseOut);
        return () => {
            ref.current?.removeEventListener('mousemove', onMouseMove);
            ref.current?.removeEventListener('mouseout', onMouseOut);
        };
    }, []);
    return mousePos;
};

export const useClick = <T extends ChartType>(canvasContainer: RefObject<HTMLElement>, datasState: ChartData<T>,
    rangeAndDomain: Array<[number, number]>, session: Session, metadata: unknown, handleMouseUp: (e: MouseEvent) => void): void => {
    useEffect(() => {
        if (canvasContainer.current === null) {
            return;
        }
        let mousedownX: number | null = null;
        let mouseMoved = false;
        const onMouseDownListener = (e: MouseEvent): void => {
            mousedownX = e.offsetX;
            mouseMoved = false;
        };
        const onMouseMoveListener = (): void => {
            mouseMoved = true;
        };
        const onMouseUpListener = (e: MouseEvent): void => {
            if (mousedownX !== e.offsetX) {
                runInAction(() => {
                    session.selectedData = undefined;
                });
                return;
            }
            if (!mouseMoved) {
                handleMouseUp(e);
            }
        };
        // shouldn't call onClick if mouseup point != mousedown point
        canvasContainer.current.addEventListener('mousedown', onMouseDownListener);
        canvasContainer.current.addEventListener('mouseup', onMouseUpListener);
        canvasContainer.current.addEventListener('mousemove', onMouseMoveListener);
        return () => {
            canvasContainer.current?.removeEventListener('mousedown', onMouseDownListener);
            canvasContainer.current?.removeEventListener('mouseup', onMouseUpListener);
            canvasContainer.current?.removeEventListener('mousemove', onMouseMoveListener);
        };
    }, [ datasState, rangeAndDomain, session, metadata ]);
};

export const useLocateChart = <T extends ChartType>(session: Session, curUnit: InsightUnit, dataState: ChartData<T>, getHeight: (data: ChartDataEle<T>) => number): void => {
    React.useEffect(() => autorun(
        () => {
            if (session.linkCharts.length === 0) { return; }
            for (const linkChart of session.linkCharts) {
                if (linkChart.res?.metadata?.unit !== curUnit || linkChart?.res.isFinished) {
                    continue;
                }
                const calculateHeight = (data: ChartDataEle<T>): void => {
                    if (linkChart.locateChart.target(data)) {
                        linkChart.locateChart.onSuccess(data);
                        runInAction(() => {
                            linkChart.res = {
                                ...linkChart.res,
                                isFinished: true,
                                metadata: { ...linkChart.res?.metadata, data },
                                chartHeight: getHeight(data),
                            };
                        });
                    }
                };
                dataState.forEach(data => {
                    if (Array.isArray(data)) {
                        data.forEach((it) => calculateHeight(it as ChartDataEle<T>));
                    } else {
                        calculateHeight(data as ChartDataEle<T>);
                    }
                });
            }
        },
    ), [session]);
};
