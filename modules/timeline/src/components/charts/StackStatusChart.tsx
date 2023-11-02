import type { Theme } from '@emotion/react';
import { useTheme } from '@emotion/react';
import * as d3 from 'd3';
import { runInAction } from 'mobx';
import { observer } from 'mobx-react';
import React, { useEffect, useMemo, useRef } from 'react';
import { ChartProps, Scale, StackStatusData, TextConfig, ChartReaction } from '../../entity/chart';
import { UnitHeight } from '../../entity/insight';
import { Session } from '../../entity/session';
import { Canvas, CanvasContainer, drawMultiBgRoundedRect, drawRoundedRect, zipStatusData } from './common';
import { useBatchedRender, useClick, useData, useHoverPos, useRangeAndDomain } from './hooks';
import { TooltipComponent, TooltipProps } from './TooltipComp';

type StackStatusChartProps = ChartProps<'stackStatus'>;
type OverflowType = 'hidden' | 'ellipsis';
type DrawTextType = Array<StackStatusData & {width: number} >;

const FONT_SIZE = 12;
const DFT_PADDING = 8;
const MIN_WIDTH = 8;
const MAX_RADIUS = 4;

const getMaxText = (text: string, maxWidth: number, ctx: CanvasRenderingContext2D, overflow: OverflowType): string => {
    if (ctx.measureText(text).width <= maxWidth) { return text; }
    if (overflow === 'hidden') { return ''; }
    let left = 0;
    let right = text.length;
    let mid = 0;
    while (left < right) {
        mid = Math.floor((left + right) / 2);
        if (ctx.measureText(text.slice(0, mid) + '...').width > maxWidth) {
            right = mid;
        } else {
            if (left === mid) { break; }
            left = mid;
        }
    }
    return text.slice(0, mid) + '...';
};

// 计算当前节点宽度及圆角半径大小，同时将需要写的文字提前放入数组保存
const drawRect = (ctx: CanvasRenderingContext2D, dataObj: { data: StackStatusData; textToDraw: DrawTextType },
    config: { height: number; right: number; xScale: Scale; yScale: Scale; overflow: OverflowType; minTextWidth: number; order?: number }): void => {
    const { data, textToDraw } = dataObj;
    const { height, right, xScale, yScale, overflow, minTextWidth, order } = config;
    let startTime = xScale(data.startTime);
    let width = Math.max(1, xScale(data.duration < 0 ? right : (data.startTime + data.duration)) - startTime);
    const radius = width >= MIN_WIDTH ? MAX_RADIUS : width / 2;
    const minWidth = overflow === 'ellipsis' ? minTextWidth : ctx.measureText(data.type).width + DFT_PADDING;

    if (width >= minWidth) {
        textToDraw.push({ ...data, width });
    }
    if (order !== undefined && data.color instanceof Array) {
        // 第一个圆角矩形块加0.5是为了解决canvas绘图毛边的问题和红色块会在分割线前一点点开始绘制的问题
        width = order === 0 ? Math.abs(xScale(data.color[0][0]) - startTime) + 0.5 : Math.abs(xScale(data.color[1][0]) - xScale(data.color[0][0]));
        startTime = order === 0 ? startTime : xScale(data.color[0][0]) + 0.5;
    }

    // leave 1px space in both top and bottom
    if (radius < 1) {
        ctx.fillRect(startTime, yScale(data.depth) + 1, width, height - 2);
    } else {
        order === undefined
            ? drawRoundedRect([ startTime, yScale(data.depth) + 1, width, height - 2 ], ctx, radius)
            : drawMultiBgRoundedRect([ startTime, yScale(data.depth) + 1, width, height - 2 ], ctx, radius, order);
        ctx.fill();
    }
};

const draw = (ctx: CanvasRenderingContext2D | null, datas: StackStatusData[][], xScale: Scale, yScale: Scale, theme: Theme, right: number, textConfig?: TextConfig): void => {
    if (!ctx) return;
    const { overflow, textMarginLeft: marginLeft, textAlign } = textConfig ?? { overflow: 'ellipsis', textMarginLeft: DFT_PADDING, textAlign: 'start' };
    ctx.font = `${FONT_SIZE}px -apple-system, BlinkMacSystemFont, "Segoe UI", "Roboto", "Oxygen", "Ubuntu", "Cantarell", "Fira Sans", "Droid Sans", "Helvetica Neue", sans-serif`;
    ctx.textAlign = textAlign;
    ctx.textBaseline = 'top';
    const minTextWidth = ctx.measureText('...').width + DFT_PADDING;
    // draw by color order
    // change fillstyle as less as possible
    const dataColor = new Map<keyof Theme['colorPalette'], StackStatusData[]>();
    const dataMultiColor: StackStatusData[] = [];
    Object.keys(theme.colorPalette).forEach(key => { dataColor.set(key as keyof Theme['colorPalette'], []); });
    datas.forEach(it => it.forEach(data => {
        // 目前只支持最多2种背景颜色的情况，下面绘制多色背景时for循环两次也是这样原因
        data.color instanceof Array ? (data.color.length === 2 && dataMultiColor.push(data)) : dataColor.get(data.color)?.push(data);
    }));
    const height = yScale(1) - yScale(0);
    const textToDraw: DrawTextType = [];
    // 绘制一个背景的节点
    dataColor.forEach((arr, key) => {
        ctx.fillStyle = theme.colorPalette[key];
        arr.forEach(data => { drawRect(ctx, { data, textToDraw }, { height, right, xScale, yScale, overflow, minTextWidth }); });
    });
    // 绘制有多个背景色的节点
    if (dataMultiColor.length > 0) {
        for (let i = 0; i < 2; i++) {
            ctx.fillStyle = theme.colorPalette[dataMultiColor[0].color[i][1] as keyof Theme['colorPalette']];
            dataMultiColor.forEach(data => { drawRect(ctx, { data, textToDraw }, { height, right, xScale, yScale, overflow, minTextWidth, order: i }); });
        }
    }
    // 绘制节点上的文字
    ctx.fillStyle = '#FFFFFFE6';
    if (height >= FONT_SIZE) {
        textToDraw.forEach(data => {
            const text = getMaxText(data.type, data.width - DFT_PADDING, ctx, overflow);
            ctx.fillText(text, xScale(data.startTime) + (typeof marginLeft === 'number' ? marginLeft : marginLeft(data.width)), yScale(data.depth) + (height - FONT_SIZE) / 2 + 1);
        });
    }
};

const findDataByXY = (mousePos: {x: number; y: number} | undefined, datas: StackStatusData[][],
    rangeAndDomain: Array<[ number, number ]>, depthHeight: number, endTime: number): StackStatusData | undefined => {
    if (mousePos === undefined || datas.length === 0 || rangeAndDomain.length === 0) {
        return undefined;
    }
    const mouseTime = d3.scaleLinear().range(rangeAndDomain[1]).domain(rangeAndDomain[0]).clamp(false)(mousePos.x);
    const depth = Math.floor(mousePos.y / depthHeight);
    const data = datas[depth];
    if (data === undefined || data.length === 0) {
        return undefined;
    }
    if (data[0].startTime > mouseTime || endTime < mouseTime || (data[data.length - 1].duration > 0 && (data[data.length - 1].startTime + data[data.length - 1].duration < mouseTime))) {
        return undefined;
    }

    let lo = 0;
    let hi = data.length;
    while (lo < hi) {
        const mid = Math.floor((lo + hi) / 2);
        const elem = data[mid];
        if (mouseTime < elem.startTime) {
            hi = mid;
        } else if ((elem.duration > 0 && mouseTime > elem.startTime + elem.duration)) {
            lo = mid + 1;
        } else {
            return elem;
        }
    }
    return undefined;
};

const findDataByXXRange = ([ downX, upX ]: number[], datas: StackStatusData[][],
    rangeAndDomain: Array<[number, number]>): StackStatusData[] | undefined => {
    if (downX === undefined || upX === undefined || datas.length === 0 || rangeAndDomain.length === 0) {
        return undefined;
    }
    const sX = Math.min(downX, upX);
    const eX = Math.max(downX, upX);
    const mouseSTime = d3.scaleLinear().range(rangeAndDomain[1]).domain(rangeAndDomain[0]).clamp(false)(sX);
    const mouseETime = d3.scaleLinear().range(rangeAndDomain[1]).domain(rangeAndDomain[0]).clamp(false)(eX);

    const result = [] as StackStatusData[];
    datas.forEach((data) => {
        data.forEach((elem) => {
            if (elem.startTime < mouseETime && elem.startTime + elem.duration > mouseSTime) {
                result.push(elem);
            }
        });
    });
    return result.length > 0 ? result : undefined;
};

const mouseUpFunc = (e: MouseEvent, datasState: StackStatusData[][], rangeAndDomain: Array<[number, number]>, rowHeight: UnitHeight, session: Session, metadata: unknown, onClick: ChartReaction<'stackStatus'> | undefined): void => {
    const clickedData = findDataByXY({ x: e.offsetX, y: e.offsetY }, datasState, rangeAndDomain, rowHeight, session.endTimeAll ?? 0);
    runInAction(() => {
        session.selectedData = clickedData;
        onClick?.(clickedData, session, metadata);
        session.selectedRangeData = undefined;
    });
};

const mouseMoveUpFunc = ([ downX, upX ]: number[], datasState: StackStatusData[][], rangeAndDomain: Array<[number, number]>, session: Session): void => {
    const selectedRangeData = findDataByXXRange([ downX, upX ], datasState, rangeAndDomain);
    runInAction(() => {
        session.selectedRangeData = selectedRangeData;
    });
};

export const StackStatusChart = observer(({ session, unit, margin, mapFunc, metadata, renderTooltip, height, onHover, onClick, decorator, rowHeight, width, textConfig, isNeedClamp }: StackStatusChartProps) => {
    const theme = useTheme();
    const canvasContainer = useRef<HTMLDivElement>(null);
    const canvas = useRef<HTMLCanvasElement>(null);
    const { action: drawExt = () => {}, triggers = [] } = decorator?.(session, metadata) ?? {};
    const datasState = useData(session, mapFunc, metadata, width, (data, width, start, end) =>
        data.map(row => zipStatusData(row, width, start, end)));
    const rangeAndDomain = useRangeAndDomain(session, width, margin); const mousePos = useHoverPos(canvasContainer);
    const hoveredData = useMemo(() => findDataByXY(mousePos, datasState, rangeAndDomain, rowHeight, session.endTimeAll ?? 0), [ mousePos, datasState, rangeAndDomain ]);
    const handleMouseUp = (e: MouseEvent): void => { mouseUpFunc(e, datasState, rangeAndDomain, rowHeight, session, metadata, onClick); };
    const handleMouseMoveUp = ([ downX, upX ]: number[]): void => { mouseMoveUpFunc([ downX, upX ], datasState, rangeAndDomain, session); };
    useEffect(() => onHover?.(hoveredData, session, metadata), [ hoveredData, metadata ]);
    useClick(canvasContainer, datasState, rangeAndDomain, session, metadata, handleMouseUp, handleMouseMoveUp);
    const yScale = (depth: number): number => depth * rowHeight;
    useBatchedRender(() => {
        if (canvasContainer.current === null || canvas.current === null || datasState.length === 0 || rangeAndDomain.length === 0 ||
            canvas.current.width === 0 || canvas.current.height === 0) {
            return;
        }
        const ctx = canvas.current.getContext('2d');
        const xScale = d3.scaleLinear().range(rangeAndDomain[0]).domain(rangeAndDomain[1]).clamp(isNeedClamp ?? true);
        ctx?.clearRect(0, 0, width, height);
        draw(ctx, datasState, xScale, yScale, theme, session.endTimeAll ?? 0, textConfig);
        drawExt({
            context: ctx,
            draw: (data, xScale, yScale) => draw(ctx, data, xScale, yScale, theme, session.endTimeAll ?? 0, textConfig),
            findAll: (condition) => datasState.map(it => it.filter(condition)),
        }, xScale, yScale, theme);
    }, [ datasState, rangeAndDomain, ...triggers, theme ]);

    const tooltipProp: TooltipProps<StackStatusData, StackStatusData[][]> = {
        data: hoveredData,
        mouseX: mousePos?.x ?? null,
        session,
        dataset: datasState,
        calcHeight: (data) => data.depth * rowHeight + rowHeight / 2,
        dom: canvasContainer,
        renderContent: (data) => renderTooltip?.(data),
    };

    return <CanvasContainer ref={canvasContainer} className={'canvasContainer'} width={width} height={height}>
        <TooltipComponent {...tooltipProp} />
        <Canvas className={'drawCanvas'} ref={canvas} width={width} height={height}/>
    </CanvasContainer>;
});
