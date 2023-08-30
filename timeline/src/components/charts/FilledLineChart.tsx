import { useTheme } from '@emotion/react';
import styled from '@emotion/styled';
import * as d3 from 'd3';
import { observer } from 'mobx-react';
import React, { useMemo, useRef, useState } from 'react';
import { ChartProps } from '../../entity/chart';
import { Readable } from '../../utils/humanReadable';
import { Canvas, CanvasContainer, LegendArea, search, zipTimeSeriesData } from './common';
import { useBatchedRender, useData, useHoverPosX, useRangeAndDomain } from './hooks';
import { TooltipComponent, TooltipProps } from './TooltipComp';

type FilledLineChartProps = ChartProps<'filledLine'>;

const findHeights = (datas: number[][]): [number, number] => {
    if (datas.length === 0) { return [ 0, 0 ]; }
    let minHeight = 0;
    let maxHeight = 0;
    datas.forEach(data => {
        let height = 0;
        data.forEach((val, index) => {
            if (index === 0) { return; }
            height += val;
        });
        minHeight = Math.min(minHeight, height);
        maxHeight = Math.max(maxHeight, height);
    });
    return [ minHeight, maxHeight * 1.2 ];
};

const drawAuxiliaryLine = (context: CanvasRenderingContext2D, yScale: d3.ScaleLinear<number, number, never>,
    auxiliaryValue: number, width: number): void => {
    context.beginPath();
    context.setLineDash([ 10, 10 ]);
    context.moveTo(0, yScale(auxiliaryValue));
    context.lineTo(width, yScale(auxiliaryValue));
    context.strokeStyle = 'white';
    context.globalAlpha = 0.2;
    context.stroke();
};

const drawArea = (context: CanvasRenderingContext2D, datas: number[][], minHeight: number, maxHeight: number,
    xScale: (n: number) => number, yScale: (n: number) => number,
    palette: string[], width: number): (undefined | number[]) => {
    if (datas.length === 0) { return; }

    const x = [ ...datas.map(it => xScale(it[0])), 1 + xScale(datas[datas.length - 1][0]) ];
    const y0 = yScale(minHeight);
    for (let i = 1; i < datas[0].length; i++) {
        context.beginPath();
        context.moveTo(x[0], y0);
        for (let j = 0; j < datas.length; j++) {
            const y = yScale(datas[j].slice(i, datas[0].length).reduce((prev, cur) => prev + cur, 0));
            context.lineTo(x[j], y);
            context.lineTo(x[j + 1], y);
        }
        context.lineTo(x[datas.length], y0);
        context.closePath();
        context.fillStyle = palette[i - 1];
        context.fill();
    };
};

const draw = (ctx: CanvasRenderingContext2D | null, datas: number[][], width: number, height: number,
    palette: string[], rangeAndDomain: Array<[ number, number ]>, hideLayer: number[],
    valueRange?: [ number, number ], auxiliaryValue?: number, legend?: string[], valueFormat?: Readable): (HTMLCanvasElement | undefined) => {
    if (!ctx) { return; }
    if (datas.length === 0 || datas[0].length > palette.length + 1) { return; }
    let drawDatas: number[][];
    [ drawDatas, palette ] = removeHideLayerDatas(datas, palette, hideLayer);
    let minHeight = 0;
    let maxHeight = 0;
    [ minHeight, maxHeight ] = valueRange ?? findHeights(drawDatas);
    maxHeight = maxHeight === 0 ? 1 : maxHeight * 1.3;
    const xScale = d3.scaleLinear().range(rangeAndDomain[0]).domain(rangeAndDomain[1]).clamp(false);
    const yScale = d3.scaleLinear().range([ height, 0 ]).domain([ minHeight, maxHeight ]);
    if (auxiliaryValue !== undefined) { drawAuxiliaryLine(ctx, yScale, auxiliaryValue, width); }
    // draw line and area
    drawArea(ctx, drawDatas, minHeight, maxHeight, xScale, yScale, palette, width);
};

const findDataByX = (mousePosX: number | undefined, datasState: number[][],
    rangeAndDomain: Array<[number, number]>): undefined | number[] => {
    if (rangeAndDomain.length === 0 || datasState.length === 0 || mousePosX === undefined) {
        return undefined;
    }
    const reverseXScale = d3.scaleLinear().range(rangeAndDomain[1]).domain(rangeAndDomain[0]).clamp(false);
    const mouseTimestamp = reverseXScale(mousePosX);
    // 如果鼠标位置在所有关键点范围之外，不显示tooltip
    if (datasState[0][0] > mouseTimestamp || datasState[datasState.length - 1][0] < mouseTimestamp) {
        return undefined;
    }
    return datasState[search(datasState, mouseTimestamp, it => it[0])];
};

const flipLayerBit = (flipBit: number, hideLayer: number[],
    setHideLayer: React.Dispatch<React.SetStateAction<number[]>>): void => {
    let bitInLayer = false;
    hideLayer.forEach((value, index, array) => {
        if (value === flipBit) {
            array.splice(index, 1);
            bitInLayer = true;
        }
    });
    if (!bitInLayer) {
        hideLayer.push(flipBit);
    }
    const ret = [...hideLayer];
    setHideLayer(ret);
};

type LegendProps = {
    legend: string[] | undefined;
    palette: string[];
    hideLayer: number[];
    setHideLayer: React.Dispatch<React.SetStateAction<number[]>>;
};

type ColorBlockProps = {
    bgColor: string;
    isHiding: boolean;
};

const ColorBlock = styled.div<ColorBlockProps>`
    background-color: ${props => (props.isHiding ? 'none' : props.bgColor)};
    height: 12px;
    width: 12px;
    margin-right: 8px;
    border-width: 1px;
    display: inline-block;
    padding: 0;
    border-radius: 4px;
    border-style: ${props => (props.isHiding ? 'dotted' : 'solid')};
    border-color: ${props => props.bgColor};
    pointer-events: none;
`;

const LegendJSX = ({ legend, palette, hideLayer, setHideLayer }: LegendProps): JSX.Element => {
    if (legend === undefined) { return <></>; }
    const table: JSX.Element[] = [];
    legend.forEach((name, index) => {
        table.push(<div style={{ marginRight: '16px', padding: 0, height: '12px' }} key={`legend_${index}`} onClick={() => {
            flipLayerBit(index, hideLayer, setHideLayer);
        }} className={'clickable'}>
            <ColorBlock isHiding={hideLayer.includes(index)} bgColor={palette[index]} key={`rect_${index}`}/>
            <div style={{ font: '12px "Microsoft YaHei", "PingFang SC", "Hiragino Sans GB", "Arial", "Narrow", "sans-serif"', verticalAlign: 'center', display: 'inline-block', pointerEvents: 'none' }} key={`text_${index}`}>{name}</div>
        </div>);
    });
    return <LegendArea id={'colorBox'}>{table.reverse()}</LegendArea>;
};

const removeHideLayerDatas = (datas: number[][], palette: string[], hideLayer: number[]): [number[][], string[]] => {
    if (hideLayer.length === 0 || datas.length === 0) { return [ datas, palette ]; }
    const ret: number[][] = [];
    let newPalette: string[] = [];
    datas.forEach(row => {
        const newRow = [...row];
        ret.push(newRow);
    });
    hideLayer = hideLayer.sort((x, y) => x - y);
    // for each row delete indexs in hideLayer
    // the 1st column is timestamp
    ret.forEach((row, index, array) => { array[index] = row.filter((elem, index) => !hideLayer.includes(index - 1)); });
    newPalette = palette.filter((elem, index) => !hideLayer.includes(index));
    return [ ret, newPalette ];
};

// eslint-disable-next-line max-lines-per-function
export const FilledLineChart = observer(({
    margin, session, mapFunc, valueFormat, valueRange, legend, auxiliaryValue, palette, renderTooltip, metadata, height, width,
}: FilledLineChartProps) => {
    const theme = useTheme();
    const canvasContainer = useRef<HTMLDivElement>(null);
    const canvas = useRef<HTMLCanvasElement>(null);

    const dataState = useData(session, mapFunc, metadata, width, zipTimeSeriesData);
    const rangeAndDomain = useRangeAndDomain(session, width, margin);

    const [ hideLayer, setHideLayer ] = useState<number[]>([]);
    const mousePosX = useHoverPosX(canvasContainer);
    const hoveredData = useMemo(() => findDataByX(mousePosX, dataState, rangeAndDomain), [ mousePosX, dataState, rangeAndDomain ]);
    const colorPalette = useMemo(() => palette.map(d => theme.colorPalette[d]), [ palette, theme ]);

    useBatchedRender(() => {
        if (canvasContainer.current === null || canvas.current === null || dataState.length === 0 || rangeAndDomain.length === 0 ||
            canvas.current.width === 0 || canvas.current.height === 0) {
            return;
        }
        const ctx = canvas.current.getContext('2d');
        ctx?.clearRect(0, 0, width, height);
        draw(ctx, dataState, width, height, colorPalette, rangeAndDomain, hideLayer, valueRange, auxiliaryValue, legend, valueFormat);
    }, [ dataState, rangeAndDomain, theme, valueRange, hideLayer, colorPalette ]);

    const tooltipProp: TooltipProps<number[], number[][]> = {
        data: hoveredData,
        mouseX: mousePosX ?? null,
        session,
        dataset: dataState,
        calcHeight: () => height / 2,
        dom: canvasContainer,
        renderContent: (data) => renderTooltip ? renderTooltip(data, metadata) : undefined,
    };
    return <CanvasContainer ref={canvasContainer} className={'canvasContainer'} width={width} height={height}>
        <TooltipComponent {...tooltipProp} />
        <Canvas className={'drawCanvas'} ref={canvas} width={width} height={height}/>
        <LegendJSX legend={legend} palette={colorPalette} hideLayer={hideLayer} setHideLayer={setHideLayer}/>
    </CanvasContainer>;
});
