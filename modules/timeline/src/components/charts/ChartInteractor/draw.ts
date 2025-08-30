/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import type { Theme } from '@emotion/react';
import type { CustomCrossRenderer } from './custom';
import type { Session } from '../../../entity/session';
import { getTimeDifference } from './common';
import type { Pos } from './common';
import type { InteractorMouseState, XReverseScaleRef } from './ChartInteractor';
import { THUMB_WIDTH_PX } from '../../base';
import { getTextParser } from '../TimelineAxis';
import { TIME_LINE_AXIS_HEIGHT_PX } from '../../ChartContainer/ChartContainer';
import type { DataBlock, FlowEvent } from '../../FilterLinkLine';
import { hashToNumber } from '../../../utils/colorUtils';
import { LinkLine, UnitHeight } from '../../../entity/insight';
import type { InsightUnit } from '../../../entity/insight';
import type { ThreadMetaData } from '../../../entity/data';
import { colorPalette } from '../../../insight/units/utils';
import { handlerEmptyString } from '../../../utils/string';
import { forEach, groupBy, isNil, keys } from 'lodash';
import { calculateLinkLines, LinkLineData } from './calculateLinkLines';
const UP_LINE: number = 30;
const DOWN_LINE: number = 45;
export const MIN_BRUSH_SIZE = 2;
export const PAGE_PADDING = 16;
const MAX_RECURSIVE_COUNT = 10;

interface DrawArrowOption {
    toX: number;
    toY: number;
    fromX: number;
    fromY: number;
    length: number;
    angle: number;
    color: string;
};

function drawArrowPath(ctx: CanvasRenderingContext2D, option: Omit<DrawArrowOption, 'color'>): void {
    const { toX, toY, fromX, fromY, length, angle } = option;
    const a = Math.atan2((toY - fromY), (toX - fromX));
    const xC = toX - (length * Math.cos(a + (angle * Math.PI / 180)));
    const yC = toY - (length * Math.sin(a + (angle * Math.PI / 180)));
    const xD = toX - (length * Math.cos(a - (angle * Math.PI / 180)));
    const yD = toY - (length * Math.sin(a - (angle * Math.PI / 180)));
    ctx.moveTo(toX, toY);
    ctx.lineTo(xC, yC);
    ctx.lineTo(xD, yD);
    ctx.lineTo(toX, toY);
}

export const drawArrow = (ctx: CanvasRenderingContext2D, option: DrawArrowOption): void => {
    ctx.save();
    ctx.fillStyle = option.color;
    ctx.beginPath();
    drawArrowPath(ctx, option);
    ctx.fill();
    ctx.closePath();
    ctx.restore();
};

interface DrawTimeDiffArgs {
    ctx: CanvasRenderingContext2D | null;
    maskRange: number[];
    xScale: (posX: number) => number;
    isNsMode: boolean;
    theme: Theme;
    selectedRange?: [number, number];
}
const drawTimeDiff = (props: DrawTimeDiffArgs): void => {
    const {
        ctx, maskRange, xScale,
        isNsMode, theme, selectedRange,
    } = props;
    if (!ctx) {
        return;
    }
    ctx.textBaseline = 'top';
    ctx.textAlign = 'center';
    let timeString;
    if (selectedRange !== undefined) {
        timeString = getTimeDifference(Math.floor(selectedRange[0]), Math.floor(selectedRange[1]), isNsMode);
    } else {
        timeString = getTimeDifference(Math.floor(xScale(maskRange[0])), Math.floor(xScale(maskRange[1])), isNsMode);
    }
    drawTimeDiffText(ctx, timeString, maskRange, theme, isNsMode);
    // left
    drawArrow(ctx, {
        toX: maskRange[0],
        toY: (UP_LINE + DOWN_LINE) / 2,
        fromX: maskRange[0] + 1,
        fromY: (UP_LINE + DOWN_LINE) / 2,
        length: 10,
        angle: 30,
        color: theme.selectedChartColor,
    });

    // right
    drawArrow(ctx, {
        toX: maskRange[1],
        toY: (UP_LINE + DOWN_LINE) / 2,
        fromX: maskRange[1] - 1,
        fromY: (UP_LINE + DOWN_LINE) / 2,
        length: 10,
        angle: 30,
        color: theme.selectedChartColor,
    });
};

// hover时的蓝色圆角方框以及时间字符串
const drawHoverTimeRect = (ctx: CanvasRenderingContext2D,
    width: number, mousePosNow: Pos, xScale: (posX: number) => number, { isNsMode, domainRange }: Session): void => {
    ctx.textAlign = 'center';
    ctx.textBaseline = 'top';
    ctx.font = '12px -apple-system, BlinkMacSystemFont, "Segoe UI", "Roboto", "Oxygen", "Ubuntu", "Cantarell", "Fira Sans", "Droid Sans", "Helvetica Neue", sans-serif';
    const displayText = getTextParser(isNsMode)(Math.floor(xScale(mousePosNow.x)), [domainRange.domainStart, domainRange.domainEnd]);
    const textLength = ctx.measureText(displayText).width;
    const roundRectWidth = textLength + 10;
    const roundRectHeight = 16;
    ctx.fillStyle = '#3778ED';
    if (mousePosNow.x < 0 || mousePosNow.x > ctx.canvas.clientWidth - THUMB_WIDTH_PX) {
        return;
    }
    if (mousePosNow.x <= roundRectWidth / 2) {
        ctx.fillRect(0, 0, roundRectWidth, roundRectHeight);
        ctx.fillStyle = 'white';
        ctx.fillText(displayText, roundRectWidth / 2, 3);
        return;
    }

    if (mousePosNow.x >= width - (roundRectWidth / 2) && mousePosNow.x <= ctx.canvas.clientWidth - THUMB_WIDTH_PX) {
        ctx.fillRect(width - roundRectWidth, 0, roundRectWidth, roundRectHeight);
        ctx.fillStyle = 'white';
        ctx.fillText(displayText, width - (roundRectWidth / 2), 3);
        return;
    }
    ctx.fillRect(mousePosNow.x - (roundRectWidth / 2), 0, roundRectWidth, roundRectHeight);
    ctx.fillStyle = 'white';
    ctx.fillText(displayText, mousePosNow.x, 3);
};

export const drawTimeDiffText = (ctx: CanvasRenderingContext2D, timeString: string, maskRange: number[], theme: Theme, isNsMode: boolean): void => {
    ctx.font = '12px sans-serif';
    const textLength = ctx.measureText(timeString).width + 20;
    if (textLength >= Math.abs(maskRange[1] - maskRange[0])) {
        ctx.beginPath();
        ctx.moveTo(maskRange[0], (UP_LINE + DOWN_LINE) / 2);
        ctx.lineTo(maskRange[1], (UP_LINE + DOWN_LINE) / 2);
        ctx.strokeStyle = theme.timeDiffPictureColor;
        ctx.stroke();

        ctx.beginPath();
        ctx.moveTo((maskRange[1] + maskRange[0]) / 2, (UP_LINE + DOWN_LINE) / 2);
        ctx.lineTo(((maskRange[1] + maskRange[0]) / 2) - 5, DOWN_LINE);
        ctx.lineTo(((maskRange[1] + maskRange[0]) / 2) + 5, DOWN_LINE);
        ctx.fillStyle = theme.timeDiffBackgroundColor;
        ctx.fill();
        // back
        ctx.beginPath();
        ctx.rect((maskRange[1] + maskRange[0] - textLength) / 2, DOWN_LINE, textLength, DOWN_LINE - UP_LINE);
        ctx.fillStyle = theme.timeDiffBackgroundColor;
        ctx.fill();

        // text
        ctx.fillStyle = theme.fontColor;
        ctx.font = '12px sans-serif';
        ctx.fillText(timeString, (maskRange[1] + maskRange[0]) / 2, DOWN_LINE + 4);
    } else {
        // back
        ctx.beginPath();
        ctx.rect(maskRange[0], UP_LINE, maskRange[1] - maskRange[0], DOWN_LINE - UP_LINE);
        ctx.fillStyle = theme.timeDiffBackgroundColor;
        ctx.fill();
        // text
        ctx.fillStyle = theme.fontColor;
        ctx.font = '12px sans-serif';
        ctx.fillText(timeString, (maskRange[1] + maskRange[0]) / 2, ((UP_LINE + DOWN_LINE) / 2) - 5);
        // line
        ctx.beginPath();
        ctx.moveTo(maskRange[0], (UP_LINE + DOWN_LINE) / 2);
        ctx.lineTo(((maskRange[0] + maskRange[1]) / 2) - (textLength / 2), (UP_LINE + DOWN_LINE) / 2);
        ctx.moveTo(maskRange[1], (UP_LINE + DOWN_LINE) / 2);
        ctx.lineTo(((maskRange[0] + maskRange[1]) / 2) + (textLength / 2), (UP_LINE + DOWN_LINE) / 2);
        ctx.strokeStyle = theme.timeDiffPictureColor;
        ctx.stroke();
    }
};

const drawSelectedRange = (ctx: CanvasRenderingContext2D | null, selectedRange: Session['selectedRange'], xReverseScaleRef: XReverseScaleRef): void => {
    if (ctx !== null && selectedRange !== undefined) {
        ctx.beginPath();
        ctx.moveTo(xReverseScaleRef.current(selectedRange[0]), 0);
        ctx.setLineDash([4, 2]);
        ctx.strokeStyle = '#5291FF';
        ctx.lineTo(xReverseScaleRef.current(selectedRange[0]), 9999);
        ctx.stroke();
        ctx.setLineDash([]);

        ctx.beginPath();
        ctx.moveTo(xReverseScaleRef.current(selectedRange[1]), 0);
        ctx.setLineDash([4, 2]);
        ctx.strokeStyle = '#5291FF';
        ctx.lineTo(xReverseScaleRef.current(selectedRange[1]), 9999);
        ctx.stroke();
        ctx.setLineDash([]);
    }
};

const getParentNodeByClassName = (el: Element, parentClassName: string): Element | null => {
    const parent = el.parentElement;
    if (parent !== null) {
        if (parent.classList.contains(parentClassName)) {
            return parent;
        } else {
            return getParentNodeByClassName(parent, parentClassName);
        }
    }
    return parent;
};

const drawMaskRange = ({
    ctx, width, height, xReverseScaleRef, xScale, interactorMouseState: {
        clickPos: { current: clickPos },
        lastPos: { current: mousePosNow },
    }, selectedRange, isNsMode, session, theme,
}: DrawArgs & { ctx: CanvasRenderingContext2D }): void => {
    let maskRange: number[] | undefined;
    if (session.selectedRangeIsLock && session.lockRange !== undefined) {
        maskRange = [xReverseScaleRef.current(session.lockRange[0]), xReverseScaleRef.current(session.lockRange[1])];
    } else if (clickPos !== undefined && mousePosNow !== undefined && clickPos.x !== mousePosNow.x) {
        // 1st priority
        // is brushing
        maskRange = [xReverseScaleRef.current(clickPos.timeAxisX), mousePosNow.x];
    } else if (selectedRange !== undefined) {
        // 2nd priority
        // not brushing now but has selected range
        maskRange = [xReverseScaleRef.current(selectedRange[0]), xReverseScaleRef.current(selectedRange[1])];
    } else {
        maskRange = undefined;
    }
    const elements = document.getElementsByClassName('chart-selected');
    const unitLength = session.selectedRangeIsLock ? session.lockUnitCount : session.selectedUnits.length;
    if (maskRange !== undefined) {
        maskRange.sort((a, b) => a - b);
        ctx.fillStyle = 'rgba(0,0,0,0.3)';
        if (unitLength !== 0) {
            ctx.fillRect(0, TIME_LINE_AXIS_HEIGHT_PX, width, height);
            if (elements.length !== 0) {
                Array.of(...elements).forEach(element => {
                    const rect = element.getBoundingClientRect();
                    const scrollContainer = getParentNodeByClassName(element, 'laneWrapper');
                    const containerRect = scrollContainer?.getBoundingClientRect();
                    let top = containerRect ? Math.max(rect.top, containerRect.top) : rect.top;
                    let bottom = containerRect ? Math.min(rect.bottom, containerRect.bottom) : rect.bottom;
                    top = top - PAGE_PADDING;
                    bottom = bottom - PAGE_PADDING;
                    if (bottom > top) {
                        const maskRangeTemp = maskRange as number[];
                        ctx.clearRect(maskRangeTemp[0], top, maskRangeTemp[1] - maskRangeTemp[0], bottom - top);
                    }
                });
            }
        } else {
            ctx.fillRect(0, TIME_LINE_AXIS_HEIGHT_PX, maskRange[0], height);
            ctx.fillRect(maskRange[1], TIME_LINE_AXIS_HEIGHT_PX, width - maskRange[1], height);
        }
        drawTimeDiff({ ctx, maskRange, xScale, isNsMode, theme, selectedRange });
    }
};

export interface DrawArgs {
    ctx: CanvasRenderingContext2D | null;
    width: number;
    height: number;
    xReverseScaleRef: XReverseScaleRef;
    xScale: (posX: number) => number;
    interactorMouseState: InteractorMouseState;
    selectedRange?: [number, number];
    isNsMode: boolean;
    session: Session;
    theme: Theme;
}
export const drawOnMove = ({
    ctx, width, height, xReverseScaleRef, xScale, interactorMouseState, selectedRange, isNsMode, session, theme,
}: DrawArgs): void => {
    if (ctx === null) { return; }
    const { clickPos: { current: clickPos }, lastPos: { current: mousePosNow } } = interactorMouseState;
    if (mousePosNow !== undefined && clickPos !== undefined && Math.abs(mousePosNow.x - clickPos.x) < MIN_BRUSH_SIZE) {
        return;
    }
    if (session.mKeyRender) {
        return;
    }
    ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height);

    // draw mask
    // 因为拖动结束时normal canvas也会绘制mask，避免绘制双层mask，这里限制只有在拖动过程中才hover canvas才绘制mask
    // session.selectedRangeIsLock 时 normal canvas 的 mask 不会删除，避免绘制双层 mask
    if (clickPos !== undefined && !session.selectedRangeIsLock) {
        drawMaskRange({ ctx, width, height, xReverseScaleRef, xScale, interactorMouseState, selectedRange, isNsMode, session, theme });
        // should filter on data type
        drawSelectedRange(ctx, selectedRange, xReverseScaleRef);
    }

    // draw hoverline and timeaxis highlight
    // session.selectedRangeIsLock 时绘制基准线没有意义
    if (clickPos !== undefined && !session.selectedRangeIsLock) {
        const startX = xReverseScaleRef.current(clickPos.timeAxisX);
        ctx.strokeStyle = '#3778ED';
        ctx.beginPath();
        ctx.moveTo(startX, 0);
        ctx.lineTo(startX, height);
        ctx.stroke(); // 绘制基准连线
    }

    if (mousePosNow !== undefined) {
        // ---hover line---
        ctx.strokeStyle = '#3778ED';
        ctx.beginPath();
        ctx.moveTo(mousePosNow.x, 0);
        ctx.lineTo(mousePosNow.x, height);
        ctx.stroke(); // 绘制鼠标位置连线

        // timeRect & text
        drawHoverTimeRect(ctx, width, mousePosNow, xScale, session); // 绘制鼠标连线上方的时间方框
    }
};

const heightMap = new Map();
// 是否是线程缩略图
const threadIsCol: Map<string, boolean> = new Map();
// 是否是进程缩略图
export const processIsCol: Map<string, boolean> = new Map();
// 是否是卡折叠
export const cardIsCol: Map<string, boolean> = new Map();
// 泳道是否已隐藏
const unitIsHidden: Map<string, boolean> = new Map();

const markUnitHidden = (unit: InsightUnit, metadata: ThreadMetaData): void => {
    if (isNil(metadata.cardId) || metadata.cardId === '') {
        return;
    }
    if (!unit.isUnitVisible) {
        let key = metadata.cardId;
        if (!isNil(metadata.processId) && metadata.processId !== '') {
            key += `-${metadata.processId}`;
        }
        if (!isNil(metadata.threadId) && metadata.threadId !== '') {
            key += `-${metadata.threadId}`;
        }
        unitIsHidden.set(key, true);
    }
};

const markUnitCollapsed = (metadata: ThreadMetaData): void => {
    if (metadata.threadIdList?.length) {
        for (const threadId of metadata.threadIdList) {
            threadIsCol.set(`${metadata.cardId}-${metadata.processId}-${threadId}`, true);
        }
    } else if (metadata.threadId !== undefined) {
        threadIsCol.set(`${metadata.cardId}-${metadata.processId}-${metadata.threadId}`, true);
    }
};

const recordUnitHeight = (unit: InsightUnit, height: number): void => {
    const metadata = unit.metadata as ThreadMetaData;

    if (metadata.cardId !== undefined) {
        heightMap.set(`${metadata.cardId}`, height);
    }
    if (metadata.processId !== undefined) {
        heightMap.set(`${metadata.cardId}-${metadata.processId}`, height);
    }
    if (metadata.threadId !== undefined && metadata.processId !== undefined) {
        if (metadata.threadIdList) {
            for (const threadId of metadata.threadIdList) {
                heightMap.set(`${metadata.cardId}-${metadata.processId}-${threadId}`, height);
            }
        } else {
            heightMap.set(`${metadata.cardId}-${metadata.processId}-${metadata.threadId}`, height);
        }

        if (unit.collapsible && !unit.isExpanded) {
            markUnitCollapsed(metadata);
        }
    }
};

const updateUnitHeight = (session: Session, pinnedAreaHeight: number): void => {
    const rootUnits = Array.from(new Set<InsightUnit>(session.units.flatMap(unit => unit.parent ?? unit)));

    const computeUnitHeight = (units: InsightUnit[], initialHeight: number, count: number = 1): number => {
        let height = initialHeight;

        for (const unit of units) {
            if (!unit.isDisplay || unit.isMerged || unit.isMultiDeviceHidden) {
                continue;
            }

            const metadata = unit.metadata as ThreadMetaData;

            recordUnitHeight(unit, height);

            if (unit.isUnitVisible) {
                height += unit.height() + 1;
            }

            if (unit.children && unit.isExpanded && count <= MAX_RECURSIVE_COUNT) {
                height = computeUnitHeight(unit.children, height, count + 1);
            }
            markUnitHidden(unit, metadata);
        }
        return height;
    };

    computeUnitHeight(rootUnits, pinnedAreaHeight);
};

export interface DrawCanvasArgs {
    ctx: CanvasRenderingContext2D | null;
    width: number;
    height: number;
    xReverseScaleRef: XReverseScaleRef;
    xScale: (posX: number) => number;
    interactorMouseState: InteractorMouseState;
    selectedRange?: [number, number];
    isNsMode: boolean;
    session: Session;
    customRenderers: CustomCrossRenderer[];
    theme: Theme;
}
export const draw = (props: DrawCanvasArgs): void => {
    const {
        ctx, width, height,
        xReverseScaleRef, xScale,
        interactorMouseState, selectedRange, isNsMode,
        session, theme,
    } = props;

    if (ctx === null) { return; }
    if (session.mKeyRender) {
        return;
    }
    // clear all
    ctx.clearRect(0, 0, width, height);
    if (selectedRange !== undefined) {
        drawMaskRange({ ctx, width, height, interactorMouseState, xReverseScaleRef, xScale, selectedRange, session, isNsMode, theme });
        // should filter on data type
        drawSelectedRange(ctx, selectedRange, xReverseScaleRef);
    }

    heightMap.clear();
    threadIsCol.clear();
    processIsCol.clear();
    unitIsHidden.clear();
    const pinnedScrollArea = document.querySelector('#main-container .topC');
    const pinnedAreaHeight = pinnedScrollArea?.clientHeight ?? 0;
    updateUnitHeight(session, pinnedAreaHeight);
    drawLinkLines(ctx, session, theme, pinnedAreaHeight);
};

/**
 *
 * @param props
 */
export const drawMEventMask = (props: DrawCanvasArgs): void => {
    const {
        ctx, width, height,
        xReverseScaleRef, xScale,
        selectedRange, isNsMode,
        session, theme,
    } = props;
    if (ctx === null) { return; }
    // clear all
    ctx.clearRect(0, 0, width, height);
    if (session.mKeyRender) {
        session.mMaskRange.sort((a, b) => a - b);
        ctx.fillStyle = 'rgba(0,0,0,0.3)';
        const start = xReverseScaleRef.current(session.mMaskRange[0]);
        const end = xReverseScaleRef.current(session.mMaskRange[1]);
        const maskRange = [start, end];
        ctx.fillRect(0, TIME_LINE_AXIS_HEIGHT_PX, maskRange[0], height);
        ctx.fillRect(maskRange[1], TIME_LINE_AXIS_HEIGHT_PX, width - maskRange[1], height);
        drawTimeDiff({ ctx, maskRange, xScale, isNsMode, theme, selectedRange });
    }
};

export const UNDRAW_HEIGHT = 45 + 2; // 45 指时间轴+旗帜轴的高度之和，2 指 useDraggableContainerEx css 中的 border-top: ${(p): string => p.theme.dividerColor} 2px solid;
export const getHeight = (session: Session, data: DataBlock, cardId: string): number | undefined => {
    let height;
    const unitHeight = heightMap.get(`${cardId}-${data.pid}-${data.tid}`);
    const processHeight = heightMap.get(`${cardId}-${data.pid}`);
    // 卡折叠的情况
    if (unitHeight === undefined && processHeight === undefined) {
        return undefined;
    }
    // 进程折叠的情况
    if (unitHeight === undefined && processHeight !== undefined) {
        height = UNDRAW_HEIGHT + processHeight - session.scrollTop + (0.5 * UnitHeight.UPPER);
        if (!session.isFullDb) { // 非全量 db 情况，进程折叠时隐藏
            processIsCol.set(`${cardId}-${data.pid}`, true);
        }
        return height;
    }
    const isCol = threadIsCol.get(`${cardId}-${data.pid}-${data.tid}`);
    if (isCol) {
        // 缩略泳道连线位置
        height = UNDRAW_HEIGHT + unitHeight - session.scrollTop + (0.5 * UnitHeight.COLL);
    } else {
        // 展开泳道连线位置
        height = UNDRAW_HEIGHT + unitHeight - session.scrollTop + ((data.depth + 0.5) * UnitHeight.STANDARD);
    }
    return height;
};

function sourceOrTargetLinkUnitIsHidden(
    { targetCardId, sourceCardId, to, from }: {targetCardId: string; sourceCardId: string; to: DataBlock; from: DataBlock},
): boolean {
    const unitKeys = [
        targetCardId,
        `${targetCardId}-${to.pid}`,
        `${targetCardId}-${to.pid}-${to.tid}`,
        sourceCardId,
        `${sourceCardId}-${from.pid}`,
        `${sourceCardId}-${from.pid}-${from.tid}`,
    ];

    for (const key of unitKeys) {
        if (unitIsHidden.get(key)) {
            return true;
        }
    }

    return false;
}

function filterToShowLinkLine(data: Record<string, unknown>, checkedCategory: string): boolean {
    const { category, from, to, cardId } = data as unknown as FlowEvent;
    if (category !== checkedCategory) {
        return false;
    }
    const [targetCardId, sourceCardId] = [handlerEmptyString(to.rankId ?? '', cardId), handlerEmptyString(from.rankId ?? '', cardId)];
    if (sourceOrTargetLinkUnitIsHidden({ targetCardId, sourceCardId, to, from })) {
        return false;
    }
    return true;
}

function drawLinkLinesByLayer(ctx: CanvasRenderingContext2D, dataList: LinkLineData[], theme: Theme): void {
    const layerMap = groupBy(dataList, ({ targetY }) => targetY);
    const sortedKeys = keys(layerMap).sort((a, b) => Number(a) - Number(b));
    forEach(sortedKeys, (key) => batchDrawLinkLines(ctx, layerMap[key], theme.selectedChartColor));
}

function batchDrawLinkLines(ctx: CanvasRenderingContext2D, dataList: LinkLineData[], fillStyle: string): void {
    const arrowOptions: Array<Omit<DrawArrowOption, 'color'>> = dataList.map(({ targetX, targetY, targetPos, offset }) => {
        const len = targetPos.length;
        let [fromX, fromY] = targetPos.reduce(([prevX, prevY], [x, y]) => [prevX + x + offset, prevY + y], [0, 0]);
        fromX = fromX / len;
        fromY = fromY / len;
        const yLen = Math.abs(fromY - targetY);
        const xLen = Math.abs(fromX - targetX);
        if (xLen === 0) {
            return undefined;
        }
        return {
            toX: targetX,
            toY: targetY,
            fromX: targetX - offset,
            fromY: targetY + (((fromY - targetY) * Math.sqrt(yLen) / xLen) + Math.abs(fromY - targetY)),
            length: 10,
            angle: 30,
        };
    }).filter((item) => item !== undefined) as Array<Omit<DrawArrowOption, 'color'>>;
    // draw line
    ctx.beginPath();
    for (const { targetX, targetY, sourceX, sourceY, offset } of dataList) {
        ctx.moveTo(sourceX, sourceY);
        ctx.bezierCurveTo(sourceX + offset, sourceY, targetX - offset, targetY, targetX, targetY);
    }
    ctx.stroke();
    ctx.closePath();
    // draw arrow
    ctx.fillStyle = fillStyle;
    ctx.beginPath();
    for (const option of arrowOptions) {
        drawArrowPath(ctx, option);
    }
    ctx.fill();
    ctx.closePath();
}

const drawLinkLines = (ctx: CanvasRenderingContext2D, session: Session, theme: Theme, pinnedAreaHeight: number): void => {
    ctx.save();
    ctx.beginPath();
    const clipTop = pinnedAreaHeight + UNDRAW_HEIGHT;
    ctx.rect(-1, clipTop, ctx.canvas.width + 1, ctx.canvas.height + 1);
    ctx.clip();
    ctx.beginPath();
    const tempCategories = session.linkLineCategories;
    const checkedCategories = [...tempCategories];
    checkedCategories.push(session.ridLineType);
    for (const checkedCategory of checkedCategories) {
        ctx.strokeStyle = theme.colorPalette[colorPalette[hashToNumber(checkedCategory, colorPalette.length)]];
        const rawList = Object.values(session.linkLines).flatMap((list) => list === undefined ? [] : list)
            .filter((data) => filterToShowLinkLine(data, checkedCategory));
        const dataList = calculateLinkLines(rawList, session, ctx);
        drawLinkLinesByLayer(ctx, dataList, theme);
    }
    const lineList: LinkLine = Object.values(session.singleLinkLine)
        .flatMap((list) => list === undefined ? [] as LinkLine : list);
    const categoryMap = groupBy(lineList, (item: FlowEvent) => item.category ?? '');
    forEach(categoryMap, (list: any[], category: string): void => {
        ctx.strokeStyle = theme.colorPalette[colorPalette[hashToNumber(category, colorPalette.length)]];
        const dataList = calculateLinkLines(list, session, ctx);
        drawLinkLinesByLayer(ctx, dataList, theme);
    });
    ctx.restore();
};
