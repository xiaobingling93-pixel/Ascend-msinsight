import * as d3 from 'd3';
import { observer } from 'mobx-react';
import React, { useRef } from 'react';
import { ChartProps, ScaleType } from '../../entity/chart';
import { Canvas, CanvasContainer, drawRoundedRect } from './common';
import { useBatchedRender, useData, useHoverPosX, useRangeAndDomain } from './hooks';
import { TooltipComponent, TooltipProps } from './TooltipComp';
import { useTheme } from '@emotion/react';
import { Session } from '../../entity/session';

type ScaleTypeDefinition = Record<ScaleType, d3.ScaleContinuousNumeric<number, number, never>>;

const getScale: ScaleTypeDefinition = {
    Linear: d3.scaleLinear(),
    SymLog: d3.scaleSymlog(),
};

/**
 * This function can iterate data and return sum of data's height.
 * @param data should dispose dataset
 * @param limited the limited iterate times, generally used for ignoring out of palette's boundary data.
 * @param callback execute process when iterate dataset
 * @returns [number] totalHeight
 */
const getTotalHeight = (data: Data, limited: number, callback?: (accHeight: number, curHeight: number, index: number, all: number[]) => void): number => {
    const { values: heights } = data;
    return heights.slice(0, limited + 1).reduce((accumulativeHeight, curHeight, index, all) => {
        accumulativeHeight += curHeight;
        callback?.(accumulativeHeight, curHeight, index, all);
        return accumulativeHeight;
    }, 0);
};

const drawAuxiliaryLine = (context: CanvasRenderingContext2D,
    yScale: d3.ScaleContinuousNumeric<number, number, never>,
    auxiliaryValue: number, width: number): void => {
    context.beginPath();
    context.setLineDash([ 10, 10 ]);
    context.moveTo(0, yScale(auxiliaryValue));
    context.lineTo(width, yScale(auxiliaryValue));
    context.strokeStyle = 'white';
    context.globalAlpha = 0.2;
    context.stroke();
};

type DrawAreaArgs = {
    ctx: CanvasRenderingContext2D;
    datas: Data[];
    minHeight: number;
    radius: number;
    xScale: d3.ScaleLinear<number, number, never>;
    yScale: d3.ScaleContinuousNumeric<number, number, never>;
    domainStart: number;
    barWidthStamp: number;
    barWidthPix?: number;
    palette: StackedBarChartProps['palette'];
};

const getThisDrawWidth = (index: number, datas: Data[], xScale: (x: number) => number, domainStart: number): number => {
    // find bar width pixel
    if (index > 0 && index < datas.length - 1) {
        // find min distance
        return xScale(Math.min(datas[index + 1].timestamp - datas[index].timestamp, datas[index].timestamp - datas[index - 1].timestamp) / 2 + domainStart);
    }
    if (datas.length === 1) {
        return 24;
    }
    if (index === 0) {
        return xScale((datas[index + 1].timestamp - datas[index].timestamp) / 2 + domainStart);
    }
    return xScale((datas[index].timestamp - datas[index - 1].timestamp) / 2 + domainStart);
};

const drawArea = ({
    ctx, datas, minHeight, radius, xScale, palette, yScale, domainStart, barWidthPix, barWidthStamp,
}: DrawAreaArgs): void => {
    if (palette === undefined) {
        return;
    }
    datas.forEach((data, index) => {
        if (data.timestamp + barWidthStamp / 2 < domainStart) {
            return;
        }
        let thisDrawWidth: number = barWidthPix as number;
        if (barWidthPix === undefined) {
            thisDrawWidth = getThisDrawWidth(index, datas, xScale, domainStart);
        }
        const drawRect = (accumulativeHeight: number, curHeight: number, index: number, allHeights: number[]): void => {
            ctx.fillStyle = palette[index];
            // setting chart transparency 1 -> totally nontransparent
            ctx.globalAlpha = 1;
            const thisDrawHeight = yScale(minHeight) - yScale(curHeight);
            const thisDrawLess = Math.min(thisDrawHeight, thisDrawWidth);
            const thisRadius = thisDrawLess < 2 * radius ? thisDrawLess / 2 : radius;
            const bottomRouned = index === 0 ? thisRadius : 0;
            const lastNonEmptyIndex = allHeights.length - 1 - allHeights.slice().reverse().findIndex((height) => height !== 0);
            const validLastNonEmptyIndex = lastNonEmptyIndex === -1 ? allHeights.length - 1 : lastNonEmptyIndex;
            const topRounded = index === validLastNonEmptyIndex ? thisRadius : 0;
            const startDrawWidth = xScale(data.timestamp) - thisDrawWidth / 2;
            const startDrawHeight = thisDrawHeight > 1 ? yScale(accumulativeHeight) : ctx.canvas.height - 1;
            // when draw height < 1px, draw 1px.
            drawRoundedRect([
                startDrawWidth,
                startDrawHeight,
                thisDrawWidth,
                thisDrawHeight > 1 ? thisDrawHeight : 1,
            ], ctx, bottomRouned, topRounded);
            ctx.fill();
        };
        getTotalHeight(data, palette.length - 1, drawRect);
    });
};
type DrawArgs = {
    ctx: CanvasRenderingContext2D | null;
    datas: Data[];
    palette: StackedBarChartProps['palette'];
    height: number;
    yScaleType: ScaleType;
    rangeAndDomain: Array<[ number, number ]>;
    radius: number;
    valueRange?: [number, number];
    auxiliaryValue?: number;
    barWidth?: number | string;
    barWidthStamp: number;
};
const draw = ({
    ctx, datas, height, yScaleType, rangeAndDomain,
    radius, valueRange, auxiliaryValue, barWidth, palette, barWidthStamp,
}: DrawArgs): void => {
    if (palette === undefined) {
        return;
    }
    if (!ctx) { return; }
    const xScale = d3.scaleLinear().range(rangeAndDomain[0]).domain(rangeAndDomain[1]).clamp(false);
    let barWidthPix: number | undefined;
    const domainStart = rangeAndDomain[1][0];
    if (barWidth !== undefined) {
        if (typeof barWidth === 'number') {
            barWidthPix = xScale(barWidth + domainStart);
        } else {
            barWidthPix = Number(barWidth.replace('px', ''));
        }
    }
    let maxHeight = 0;
    let minHeight = 0;
    const findHeights = (datas: Data[]): void => {
        datas.forEach((data) => {
            const curHeight = getTotalHeight(data, palette.length - 1);
            maxHeight = Math.max(maxHeight, curHeight);
            minHeight = Math.min(minHeight, curHeight);
        });
    };

    if (valueRange) {
        [ minHeight, maxHeight ] = valueRange;
    } else {
        findHeights(datas);
        maxHeight *= 2;
    }
    const yScale = getScale[yScaleType].range([ height, 0 ]).domain([ minHeight, maxHeight ]);
    if (auxiliaryValue !== undefined && auxiliaryValue !== 0) { drawAuxiliaryLine(ctx, yScale, auxiliaryValue, rangeAndDomain[0][1]); }
    // draw line and area
    drawArea({ ctx, datas, minHeight, radius, xScale, yScale, domainStart, barWidthStamp, barWidthPix, palette });
};

type ToolTipData = [ Data, number ];
const findDataByX = (mousePosX: number | undefined, datas: Data[], rangeAndDomain: Array<[number, number]>): ToolTipData | undefined => {
    if (rangeAndDomain.length === 0 || datas.length === 0 || mousePosX === undefined) { return; }
    const reverseXScale = d3.scaleLinear().range([ rangeAndDomain[1][0], rangeAndDomain[1][1] ])
        .domain([ rangeAndDomain[0][0], rangeAndDomain[0][1] ]).clamp(false);
    const xScale = d3.scaleLinear().domain([ rangeAndDomain[1][0], rangeAndDomain[1][1] ])
        .range([ rangeAndDomain[0][0], rangeAndDomain[0][1] ]).clamp(false);
    const mouseTimestamp = reverseXScale(mousePosX);
    let minDistance = Number.MAX_VALUE;
    let selectedData = null;
    for (let i = 0; i < datas.length; i++) {
        const distanceNow = Math.abs(datas[i].timestamp - mouseTimestamp);
        if (distanceNow < minDistance) {
            minDistance = distanceNow;
            selectedData = datas[i];
        }
    }
    if (!selectedData) { return; }
    return [ selectedData, xScale(selectedData.timestamp) ];
};

const isTooltipXInDomain = (data: ToolTipData, session: Session): boolean => {
    const domainRange = session.domainRange;
    return data[0].timestamp > domainRange.domainStart && data[0].timestamp < domainRange.domainEnd;
};

const getBarWidthStamp = (barWidth: number | string, rangeAndDomain: Array<[number, number]>): number => {
    const xReverseScale = d3.scaleLinear().range(rangeAndDomain[1]).domain(rangeAndDomain[0]).clamp(false);
    const domainStart = rangeAndDomain[1][0];
    if (typeof barWidth === 'number') {
        return barWidth;
    }
    return xReverseScale(Number(barWidth.replace('px', ''))) - domainStart;
};

const isHoverPosOnBar = (data: ToolTipData, barWidthStamp: number, rangeAndDomain: Array<[number, number]>, mousePosX: number): boolean => {
    const drawStartStamp = data[0].timestamp - barWidthStamp / 2;
    const drawEndStamp = data[0].timestamp + barWidthStamp / 2;
    const reverseXScale = d3.scaleLinear().range([ rangeAndDomain[1][0], rangeAndDomain[1][1] ])
        .domain([ rangeAndDomain[0][0], rangeAndDomain[0][1] ]).clamp(false);
    const mouseTimestamp = reverseXScale(mousePosX);
    return mouseTimestamp >= drawStartStamp && mouseTimestamp <= drawEndStamp;
};

type Data = { timestamp: number; values: number[] };
type StackedBarChartProps = ChartProps<'stackedBar'>;
export const StackedBarChart = observer(({
    margin, session, mapFunc, palette,
    valueRange, barWidth, radius, yScaleType, auxiliaryValue,
    renderTooltip, width, height, metadata,
}: StackedBarChartProps) => {
    const canvasContainer = useRef<HTMLDivElement>(null);
    const canvas = useRef<HTMLCanvasElement>(null);
    const rangeAndDomain = useRangeAndDomain(session, width, margin);
    const barWidthStamp = getBarWidthStamp(barWidth, rangeAndDomain);
    const datas = useData(session, mapFunc, metadata, width, (data) => data.filter(item => item.timestamp + barWidthStamp / 2 >= rangeAndDomain[1][0] && item.timestamp - barWidthStamp / 2 <= rangeAndDomain[1][1]));
    const mousePosX = useHoverPosX(canvasContainer);
    const theme = useTheme();
    const defaultPalette = [ '#4183a2', '#549251', '#b09239', '#bb5f43', theme.colorPalette.otherColor ];
    const hoveredData = React.useMemo(() => findDataByX(mousePosX, datas, rangeAndDomain), [ mousePosX, datas, rangeAndDomain ]);
    useBatchedRender(() => {
        if (canvasContainer.current === null || canvas.current === null || rangeAndDomain.length === 0 ||
            canvas.current.width === 0 || canvas.current.height === 0) {
            return;
        }
        const ctx = canvas.current.getContext('2d');
        ctx?.clearRect(0, 0, width, height);
        if (palette === undefined) {
            palette = defaultPalette;
        }
        draw({ ctx, datas, height, yScaleType, rangeAndDomain, radius, valueRange, auxiliaryValue, barWidth, palette, barWidthStamp });
    }, [ datas, rangeAndDomain, valueRange ]);

    const tooltipProp: TooltipProps<ToolTipData, Data[]> = {
        data: (hoveredData !== undefined && !isTooltipXInDomain(hoveredData, session) && !isHoverPosOnBar(hoveredData, barWidthStamp, rangeAndDomain, mousePosX ?? 0)) ? undefined : hoveredData,
        x: (data) => isTooltipXInDomain(data, session) ? data[1] : mousePosX ?? 0,
        mouseX: mousePosX ?? null,
        session,
        dataset: datas,
        calcHeight: () => height / 2,
        dom: canvasContainer,
        renderContent: (data) => renderTooltip?.(data[0], metadata),
    };

    return <CanvasContainer ref={canvasContainer} className={'canvasContainer'} width={width} height={height}>
        {height === null ? null : <TooltipComponent {...tooltipProp} />}
        <Canvas className={'drawCanvas'} ref={canvas} width={width} height={height}/>
    </CanvasContainer>;
});
