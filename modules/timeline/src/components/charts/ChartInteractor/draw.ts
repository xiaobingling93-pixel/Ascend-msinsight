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
import { UnitHeight } from '../../../entity/insight';
import type { InsightUnit } from '../../../entity/insight';
import type { ThreadMetaData } from '../../../entity/data';
import { colorPalette, getTimeOffset } from '../../../insight/units/utils';
import * as d3 from 'd3';
import { handlerEmptyString } from '../../../utils/string';
import { isNil } from 'lodash';
const UP_LINE: number = 30;
const DOWN_LINE: number = 45;
export const MIN_BRUSH_SIZE = 2;
export const PAGE_PADDING = 16;

interface DrawArrowOptions {
    toX: number;
    toY: number;
    fromX: number;
    fromY: number;
    length: number;
    angle: number;
    color: string;
};
export const drawArrow = (ctx: CanvasRenderingContext2D, { toX, toY, fromX, fromY, length, angle, color }: DrawArrowOptions): void => {
    const a = Math.atan2((toY - fromY), (toX - fromX));
    const xC = toX - (length * Math.cos(a + (angle * Math.PI / 180)));
    const yC = toY - (length * Math.sin(a + (angle * Math.PI / 180)));
    const xD = toX - (length * Math.cos(a - (angle * Math.PI / 180)));
    const yD = toY - (length * Math.sin(a - (angle * Math.PI / 180)));
    ctx.save();
    ctx.beginPath();
    ctx.fillStyle = color;
    ctx.moveTo(toX, toY);
    ctx.lineTo(xC, yC);
    ctx.lineTo(xD, yD);
    ctx.lineTo(toX, toY);
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
    if (clickPos !== undefined && mousePosNow !== undefined && clickPos.x !== mousePosNow.x) {
        // 1st priority
        // is brushing
        maskRange = [clickPos.x, mousePosNow.x];
    } else if (selectedRange !== undefined) {
        // 2nd priority
        // not brushing now but has selected range
        maskRange = [xReverseScaleRef.current(selectedRange[0]), xReverseScaleRef.current(selectedRange[1])];
    } else {
        maskRange = undefined;
    }
    const elements = document.getElementsByClassName('chart-selected');
    if (maskRange !== undefined) {
        maskRange.sort((a, b) => a - b);
        ctx.fillStyle = 'rgba(0,0,0,0.3)';
        if (session.selectedUnits.length !== 0) {
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
    // clear all
    ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height);

    // draw mask
    // 因为拖动结束时normal canvas也会绘制mask，避免绘制双层mask，这里限制只有在拖动过程中才hover canvas才绘制mask
    if (clickPos !== undefined) {
        drawMaskRange({ ctx, width, height, xReverseScaleRef, xScale, interactorMouseState, selectedRange, isNsMode, session, theme });
        // should filter on data type
        drawSelectedRange(ctx, selectedRange, xReverseScaleRef);
    }

    // draw hoverline and timeaxis highlight
    if (clickPos !== undefined) {
        ctx.strokeStyle = '#3778ED';
        ctx.beginPath();
        ctx.moveTo(clickPos.x, 0);
        ctx.lineTo(clickPos.x, height);
        ctx.stroke();
    }

    if (mousePosNow !== undefined) {
        // ---hover line---
        ctx.strokeStyle = '#3778ED';
        ctx.beginPath();
        ctx.moveTo(mousePosNow.x, 0);
        ctx.lineTo(mousePosNow.x, height);
        ctx.stroke();

        // timeRect & text
        drawHoverTimeRect(ctx, width, mousePosNow, xScale, session);
    }
};

const heightMap = new Map();
// 是否是线程缩略图
const threadIsCol: Map<string, boolean> = new Map();
// 是否是进程缩略图
const processIsCol: Map<string, boolean> = new Map();
// 泳道是否已隐藏
const unitIsHidden: Map<string, boolean> = new Map();

const setHiddenUnit = (unit: InsightUnit, metadata: ThreadMetaData): void => {
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
const updateUnitHeight = (session: Session, pinnedAreaHeight: number): void => {
    const height = pinnedAreaHeight;

    const computeUnitHeight = (props: {units: InsightUnit[]; height: number}): number => {
        for (const unit of props.units) {
            if (!unit.isDisplay) {
                continue;
            }
            const metadata = unit.metadata as ThreadMetaData;
            if (metadata.processId !== undefined) {
                heightMap.set(`${metadata.cardId}-${metadata.processId}`, props.height);
            }
            if (metadata.threadId !== undefined && metadata.processId !== undefined) {
                heightMap.set(`${metadata.cardId}-${metadata.processId}-${metadata.threadId}`, props.height);
                if (unit.collapsible && !unit.isExpanded) {
                    threadIsCol.set(`${metadata.cardId}-${metadata.processId}-${metadata.threadId}`, true);
                }
            }
            if (unit.isUnitVisible) {
                props.height += unit.height() + 1;
            }
            if (unit.children && unit.isExpanded) {
                props.height = computeUnitHeight({ ...props, units: unit.children });
            }
            setHiddenUnit(unit, metadata);
        }
        return props.height;
    };
    const rootUnits = Array.from(new Set<InsightUnit>(session.units.flatMap(unit => unit.parent ?? unit)));
    computeUnitHeight({ units: rootUnits, height });
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
    drawMaskRange({ ctx, width, height, interactorMouseState, xReverseScaleRef, xScale, selectedRange, session, isNsMode, theme });

    // should filter on data type
    drawSelectedRange(ctx, selectedRange, xReverseScaleRef);

    heightMap.clear();
    threadIsCol.clear();
    processIsCol.clear();
    unitIsHidden.clear();
    const pinnedScrollArea = document.getElementsByClassName('pinnedScrollArea');
    const pinnedAreaHeight = pinnedScrollArea[0]?.clientHeight ?? 0;
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

const UNDRAW_HEIGHT = 45;
const getHeight = (session: Session, data: DataBlock, cardId: string): number | undefined => {
    let height;
    const unitHeight = heightMap.get(`${cardId}-${data.pid}-${data.tid}`);
    if (unitHeight === undefined) {
        // 进程折叠的情况
        const processUnitHeight = heightMap.get(`${cardId}-${data.pid}`);
        height = UNDRAW_HEIGHT + processUnitHeight - session.scrollTop + (0.5 * UnitHeight.UPPER);
        processIsCol.set(`${cardId}-${data.pid}`, true);
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

function drawSingleLinkLine(data: Record<string, unknown>, checkedCategory: string, session: Session, ctx: CanvasRenderingContext2D, theme: Theme): void {
    const { category, from, to, cardId } = data as unknown as FlowEvent;
    if (category !== checkedCategory) {
        return;
    }
    const li = d3.scaleLinear().range([0, ctx.canvas.clientWidth]).domain([session.domainRange.domainStart, session.domainRange.domainEnd]);
    const [targetCardId, sourceCardId] = [handlerEmptyString(to.rankId ?? '', cardId), handlerEmptyString(from.rankId ?? '', cardId)];
    const [targetX, targetY] = [li(to.timestamp - getTimeOffset(session, { cardId: targetCardId, processId: to.pid })), getHeight(session, to, targetCardId)];
    const [sourceX, sourceY] = [li(from.timestamp - getTimeOffset(session, { cardId: sourceCardId, processId: from.pid })),
        getHeight(session, from, sourceCardId)];

    if (sourceOrTargetLinkUnitIsHidden({ targetCardId, sourceCardId, to, from })) {
        return;
    }
    if (processIsCol.get(`${targetCardId}-${to.pid}`) && processIsCol.get(`${sourceCardId}-${from.pid}`)) {
        return;
    }
    if ((sourceY === undefined || targetY === undefined)) {
        return;
    }
    if (sourceY < UNDRAW_HEIGHT && targetY < UNDRAW_HEIGHT) {
        return;
    }
    const targetPos: Array<[x: number, y: number]> = [[targetX, targetY]];

    const offset = ((targetX - sourceX) / 2);
    ctx.beginPath();
    ctx.moveTo(sourceX, sourceY);
    ctx.bezierCurveTo(sourceX + offset, sourceY, targetX - offset, targetY, targetX, targetY);
    ctx.stroke();
    ctx.closePath();
    if (targetY >= UNDRAW_HEIGHT || sourceY >= UNDRAW_HEIGHT) {
        const len = targetPos.length;
        let [fromX, fromY] = targetPos.reduce(([prevX, prevY], [x, y]) => [prevX + x + offset, prevY + y], [0, 0]);
        fromX = fromX / len;
        fromY = fromY / len;
        const yLen = Math.abs(fromY - targetY);
        const xLen = Math.abs(fromX - targetX);
        if (xLen === 0) {
            return;
        }
        drawArrow(ctx, {
            toX: targetX,
            toY: targetY,
            fromX: targetX - offset,
            fromY: targetY + (((fromY - targetY) * Math.sqrt(yLen) / xLen) + Math.abs(fromY - targetY)),
            length: 10,
            angle: 30,
            color: theme.selectedChartColor,
        });
    }
}

const drawLinkLines = (ctx: CanvasRenderingContext2D, session: Session, theme: Theme, pinnedAreaHeight: number): void => {
    ctx.save();
    ctx.beginPath();
    const clipTop = pinnedAreaHeight + UNDRAW_HEIGHT;
    ctx.rect(-1, clipTop, ctx.canvas.width + 1, ctx.canvas.height + 1);
    ctx.clip();
    ctx.beginPath();
    const checkedCategories = session.linkLineCategories;
    for (const checkedCategory of checkedCategories) {
        ctx.strokeStyle = theme.colorPalette[colorPalette[hashToNumber(checkedCategory, colorPalette.length)]];
        Object.values(session.linkLines)
            .forEach(datas => {
                datas?.forEach((data) => {
                    drawSingleLinkLine(data, checkedCategory, session, ctx, theme);
                });
            });
    }
    Object.values(session.singleLinkLine)
        .forEach(datas => {
            datas?.forEach((data) => {
                const { category } = data as unknown as FlowEvent;
                ctx.strokeStyle = theme.colorPalette[colorPalette[hashToNumber(category, colorPalette.length)]];
                drawSingleLinkLine(data, category, session, ctx, theme);
            });
        });
    ctx.restore();
};
