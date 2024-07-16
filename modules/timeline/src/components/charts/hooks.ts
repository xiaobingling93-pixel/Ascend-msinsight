/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import type React from 'react';
import { type RefObject, useEffect, useMemo, useRef, useState } from 'react';
import { ChartData, ChartType, MapFunc } from '../../entity/chart';
import type { Session } from '../../entity/session';
import { logger } from '../../utils/Logger';
import { runInAction } from 'mobx';
import type { InsightUnit } from '../../entity/insight';

export interface Pos {
    x: number;
    y: number;
};

export type DataProcessor<T extends ChartType> = (data: ChartData<T>, width: number, domainStart: number, domainEnd: number) => ChartData<T>;
const CLICK_TOLERANCE = 1;

interface UseDataParams<T extends ChartType> {
    session: Session;
    mapFunc: MapFunc<T>;
    unit: InsightUnit;
    metadata: unknown;
    width: number;
    processor?: DataProcessor<T>;
}

/**
 * Manages the data that are to be rendered as a state.
 *
 * @param session session
 * @param mapFunc mapFunc
 * @param metadata metadata
 * @param width width
 * @returns the data that this chart is currently rendering
 */
export const useData = <T extends ChartType>({ session, mapFunc, unit, metadata, width, processor }: UseDataParams<T>): ChartData<T> => {
    const { domainStart, domainEnd } = session.domainRange;
    const { endTimeAll } = session;
    const [datasState, setDatasState] = useState<ChartData<T>>([]);
    const requestedWidth = useRef(0);
    useEffect(() => {
        if (width === 0) {
            setDatasState([]);
            return;
        }
        requestedWidth.current = width;
        mapFunc(session, metadata, unit).then(datas => {
            if (requestedWidth.current !== width) {
                // drop the data if width has been changed since when request was made
                return;
            }
            // the datas should be sorted by startTime(min -> max).
            setDatasState(processor?.(datas, width, domainStart, domainEnd) ?? datas);
        }).catch(() => {
            logger('hooks useData', 'mapFunc occurred an exception.');
        }).finally(() => {
            runInAction(() => { unit.phase = 'download'; });
        });
    }, [session.phase, domainStart, domainEnd, endTimeAll, width,
        session.unitsConfig.offsetConfig.timestampOffset,
        session.unitsConfig.filterConfig.pythonFunction]);
    return datasState;
};

export const useRangeAndDomain = (session: Session, width: number, margin: number): Array<[number, number]> => {
    const { domainStart, domainEnd } = session.domainRange;
    return useMemo<Array<[number, number]>>(
        () => [[margin, width - (2 * margin)], [domainStart, domainEnd]],
        [domainStart, domainEnd, width, margin],
    );
};

/**
 * Calls renderer when deps are changed, but limit the frequency
 *
 * @param renderer the render function
 * @param deps the dependencies that triggers re-render
 */
export const useBatchedRender = (renderer: () => void, deps: React.DependencyList): void => {
    useEffect(() => {
        renderer();
    }, deps);
};

export const useHoverPosX = (ref: React.RefObject<HTMLElement>): number | undefined => {
    return useHoverPos(ref)?.x ?? undefined;
};

export const useHoverPos = (ref: React.RefObject<HTMLElement>): Pos | undefined => {
    const [mousePos, setMousePos] = useState<Pos>();
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
            return () => { };
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

interface UseClickParams<T extends ChartType> {
    canvasContainer: RefObject<HTMLElement>;
    datasState: ChartData<T>;
    rangeAndDomain: Array<[number, number]>;
    session: Session;
    metadata: unknown;
    handleMouseUp: (e: MouseEvent) => void;
    handleMouseMoveUp?: (xArr: number[]) => void;
}

export const useClick = <T extends ChartType>({
    canvasContainer, datasState, rangeAndDomain, session, metadata, handleMouseUp, handleMouseMoveUp,
}: UseClickParams<T>): void => {
    useEffect(() => {
        if (canvasContainer.current === null) {
            return () => { };
        }
        let mousedownX: number | null = null;
        let mouseMoved = false;
        const onMouseDownListener = (e: MouseEvent): void => {
            mousedownX = e.offsetX;
            mouseMoved = false;
        };
        const onMouseMoveListener = (e: MouseEvent): void => {
            if (mousedownX !== null && Math.abs(mousedownX - e.offsetX) > CLICK_TOLERANCE) {
                mouseMoved = true;
            }
        };
        const onMouseUpListener = (e: MouseEvent): void => {
            if (mousedownX !== null && Math.abs(mousedownX - e.offsetX) > CLICK_TOLERANCE) {
                runInAction(() => {
                    session.selectedData = undefined;
                });
                if (mouseMoved && mousedownX !== null) {
                    handleMouseMoveUp?.([mousedownX, e.offsetX]);
                }
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
    }, [datasState, rangeAndDomain, session, metadata]);
};
