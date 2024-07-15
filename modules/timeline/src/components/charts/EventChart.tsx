/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import type { Theme } from '@emotion/react';
import { useTheme } from '@emotion/react';
import * as d3 from 'd3';
import { observer } from 'mobx-react';
import React, { useMemo, useRef } from 'react';
import type { ChartProps, EventData } from '../../entity/chart';
import { Canvas, CanvasContainer } from './common';
import { type Pos, useBatchedRender, useData, useHoverPos, useRangeAndDomain } from './hooks';
import { TooltipComponent, TooltipProps } from './TooltipComp';

type EventChartProps = ChartProps<'keyEvent'>;
const EXTEND_AREA = 80; // 鼠标点击图形区域
const ICON_SIZE = 5; // 菱形对角线的一半、圆半径
const Y_DISTANCES = 5; // 图形距离y轴距离
const RECT_HEIGHT = 10; // Hover Rect宽度
const borderWidth = 2;

function drawData(context: CanvasRenderingContext2D, happenTime: number, endTime: number, rangeAndDomain: Array<[number, number]>, theme: Theme): void {
    const xScale = d3.scaleLinear().range(rangeAndDomain[0]).domain(rangeAndDomain[1]).clamp(true);
    const [domainStart, domainEnd] = rangeAndDomain[1];
    if (happenTime === endTime) {
        if (happenTime >= domainStart && happenTime <= domainEnd) {
            drawCircle(theme, context, xScale(happenTime));
        }
        return;
    }
    if (happenTime >= domainStart && happenTime <= domainEnd) {
        drawRhombus(theme, context, xScale(happenTime));
    }
    if (endTime >= domainStart && endTime <= domainEnd) {
        drawSquares(theme, context, xScale(endTime));
    }
}

function drawCircle(theme: Theme, context: CanvasRenderingContext2D, x: number): void {
    context.beginPath();
    context.arc(x, ICON_SIZE + Y_DISTANCES, 5, 0, 2 * Math.PI);
    context.fillStyle = theme.systemEventColor;
    context.fill();
}
function drawSquares(theme: Theme, context: CanvasRenderingContext2D, x: number): void {
    context.fillStyle = theme.systemEventColor;
    context.fillRect(x - ICON_SIZE, Y_DISTANCES, 2 * ICON_SIZE, 2 * ICON_SIZE);
    context.strokeStyle = theme.contentBackgroundColor;
    context.strokeRect(x - ICON_SIZE - borderWidth, Y_DISTANCES - borderWidth, (2 * ICON_SIZE) + borderWidth, (2 * ICON_SIZE) + borderWidth);
}
function drawRhombus(theme: Theme, context: CanvasRenderingContext2D, x: number): void {
    context.beginPath();
    context.moveTo(x, Y_DISTANCES);
    context.lineTo(x + ICON_SIZE, Y_DISTANCES + ICON_SIZE);
    context.lineTo(x, Y_DISTANCES + (2 * ICON_SIZE));
    context.lineTo(x - ICON_SIZE, Y_DISTANCES + ICON_SIZE);
    context.closePath();
    context.fillStyle = theme.systemEventColor;
    context.fill();
}

function draw(ctx: CanvasRenderingContext2D | null, dataState: EventData[], rangeAndDomain: Array<[number, number]>, theme: Theme): void {
    if (ctx === null) {
        return;
    }
    dataState.forEach((item) => {
        drawData(ctx, item.happenTime, item.endTime, rangeAndDomain, theme);
    });
}

function drawRect(theme: Theme, context: CanvasRenderingContext2D, rectStart: number, rectEnd: number): void {
    context.fillStyle = theme.systemEventColor;
    context.fillRect(rectStart, Y_DISTANCES, rectEnd - rectStart, RECT_HEIGHT);
}

const drawRectByXY = (context: CanvasRenderingContext2D | null, dataState: EventData[], rangeAndDomain: Array<[number, number]>,
    mousePos: Pos | undefined, theme: Theme): void => {
    const isContextInvalid = !context;
    const isRangeOrDataEmpty = rangeAndDomain.length === 0 || dataState.length === 0;
    const isMousePosInvalid = mousePos === undefined || mousePos.y < 8 || mousePos.y > 20;

    if (isContextInvalid || isRangeOrDataEmpty || isMousePosInvalid) {
        return;
    }

    const [domainStart, domainEnd] = rangeAndDomain[1];
    const xScale = d3.scaleLinear().range(rangeAndDomain[0]).domain(rangeAndDomain[1]).clamp(true);
    context.globalAlpha = 0.3;
    for (let i = 0; i < dataState.length; i++) {
        const happenTime = dataState[i].happenTime;
        const endTime = dataState[i].endTime;
        if (happenTime === endTime) {
            continue;
        }
        const happenPos = xScale(happenTime);
        if (Math.abs(mousePos.x - happenPos) < ICON_SIZE) {
            drawRect(theme, context, happenPos, xScale(Math.min(domainEnd, endTime)));
            break;
        }
        const endPos = xScale(endTime);
        if (Math.abs(mousePos.x - endPos) < ICON_SIZE) {
            drawRect(theme, context, xScale(Math.max(domainStart, happenTime)), endPos);
            break;
        }
    }
    context.globalAlpha = 1;
};

const findDataByX = (mousePosX: number | undefined, dataState: EventData[],
    rangeAndDomain: Array<[number, number]>): (undefined | EventData) => {
    if (rangeAndDomain.length === 0 || dataState.length === 0 || mousePosX === undefined) { return undefined; }
    const reverseXScale = d3.scaleLinear().range(rangeAndDomain[1]).domain(rangeAndDomain[0]).clamp(true);
    const mouseTimestamp = reverseXScale(mousePosX);
    for (let i = 0; i < dataState.length; i++) {
        const happenTime = dataState[i].happenTime;
        const endTime = dataState[i].endTime;
        const startLeft = happenTime - EXTEND_AREA;
        const startRight = happenTime + EXTEND_AREA;
        const endLeft = endTime - EXTEND_AREA;
        const endRight = endTime + EXTEND_AREA;
        const isWithinStartRange = mouseTimestamp >= startLeft && mouseTimestamp <= startRight;
        const isWithinEndRange = mouseTimestamp >= endLeft && mouseTimestamp <= endRight;

        if (isWithinStartRange || isWithinEndRange) {
            return dataState[i];
        }
    }
    return undefined;
};

export const EventChart = observer(({ margin, session, mapFunc, metadata, height, renderTooltip, width, unit }: EventChartProps) => {
    const theme = useTheme();
    const canvasContainer = useRef<HTMLDivElement>(null);
    const canvas = useRef<HTMLCanvasElement>(null);
    const dataState = useData(session, mapFunc, unit, metadata, width);
    const rangeAndDomain = useRangeAndDomain(session, width, margin);

    const mousePos = useHoverPos(canvasContainer);
    const hoveredData = useMemo(() => findDataByX(mousePos?.x, dataState, rangeAndDomain), [mousePos, dataState, rangeAndDomain]);

    const isCanvasInvalid = !canvasContainer.current || !canvas.current;
    const isDataOrRangeEmpty = dataState.length === 0 || rangeAndDomain.length === 0;
    const isCanvasSizeZero = canvas.current && (canvas.current.width === 0 || canvas.current.height === 0);

    useBatchedRender(() => {
        if (isCanvasInvalid || isDataOrRangeEmpty || isCanvasSizeZero) {
            return;
        }
        const ctx = canvas.current.getContext('2d');
        ctx?.clearRect(0, 0, width, height);
        draw(ctx, dataState, rangeAndDomain, theme);
        drawRectByXY(ctx, dataState, rangeAndDomain, mousePos, theme);
    }, [dataState, rangeAndDomain, theme, mousePos]);

    const tooltipProp: TooltipProps<EventData, EventData[]> = {
        data: hoveredData,
        mouseX: mousePos?.x ?? null,
        session,
        dataset: dataState,
        calcHeight: () => height / 2,
        dom: canvasContainer,
        renderContent: (data) => renderTooltip?.(data, metadata),
    };

    return <CanvasContainer ref={canvasContainer} className={'canvasContainer'} width={width} height={height}>
        <TooltipComponent {...tooltipProp} />
        <Canvas className={'drawCanvas'} ref={canvas} width={width} height={height}/>
    </CanvasContainer>;
});
