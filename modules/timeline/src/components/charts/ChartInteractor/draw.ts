import type { Theme } from '@emotion/react';
import { CustomCrossRenderer } from './custom';
import { Session } from '../../../entity/session';
import { Pos, getTimeDifference } from './common';
import { drawRoundedRect } from '../common';
import { InteractorMouseState } from './ChartInteractor';
import { THUMB_WIDTH_PX } from '../../base';
import { getTextParser } from '../TimelineAxis';
import { Scale } from '../../../entity/chart';
import { TIME_LINE_AXIS_HEIGHT_PX } from '../../ChartContainer/ChartContainer';
import { DataBlock, FlowEvent } from '../../FilterLinkLine';
import { hashToNumber } from '../../../utils/colorUtils';
import { InsightUnit, UnitHeight } from '../../../entity/insight';
import { ThreadMetaData } from '../../../entity/data';
import { colorPalette } from '../../../insight/units/utils';
import * as d3 from 'd3';
const UP_LINE: number = 30;
const DOWN_LINE: number = 45;

type DrawArrowOptions = {
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
    const xC = toX - length * Math.cos(a + angle * Math.PI / 180);
    const yC = toY - length * Math.sin(a + angle * Math.PI / 180);
    const xD = toX - length * Math.cos(a - angle * Math.PI / 180);
    const yD = toY - length * Math.sin(a - angle * Math.PI / 180);
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

const drawTimeDiff = (ctx: CanvasRenderingContext2D | null, maskRange: number[], xScale: (posX: number) => number,
    isNsMode: boolean, theme: Theme, selectedRange: [number, number] | undefined): void => {
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
    const rouned = 8;
    ctx.fillStyle = '#3778ED';
    if (mousePosNow.x < 0 || mousePosNow.x > ctx.canvas.clientWidth - THUMB_WIDTH_PX) {
        return;
    }
    if (mousePosNow.x <= roundRectWidth / 2) {
        drawRoundedRect([0, 0, roundRectWidth, roundRectHeight], ctx, rouned);
        ctx.fill();
        ctx.fillStyle = 'white';
        ctx.fillText(displayText, roundRectWidth / 2, 3);
        return;
    }

    if (mousePosNow.x >= width - roundRectWidth / 2 && mousePosNow.x <= ctx.canvas.clientWidth - THUMB_WIDTH_PX) {
        drawRoundedRect([width - roundRectWidth, 0, roundRectWidth, roundRectHeight], ctx, rouned);
        ctx.fill();
        ctx.fillStyle = 'white';
        ctx.fillText(displayText, width - roundRectWidth / 2, 3);
        return;
    }
    drawRoundedRect([mousePosNow.x - roundRectWidth / 2, 0, roundRectWidth, roundRectHeight], ctx, rouned);
    ctx.fill();
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
        ctx.lineTo((maskRange[1] + maskRange[0]) / 2 - 5, DOWN_LINE);
        ctx.lineTo((maskRange[1] + maskRange[0]) / 2 + 5, DOWN_LINE);
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
        ctx.fillText(timeString, (maskRange[1] + maskRange[0]) / 2, (UP_LINE + DOWN_LINE) / 2 - 5);
        // line
        ctx.beginPath();
        ctx.moveTo(maskRange[0], (UP_LINE + DOWN_LINE) / 2);
        ctx.lineTo((maskRange[0] + maskRange[1]) / 2 - textLength / 2, (UP_LINE + DOWN_LINE) / 2);
        ctx.moveTo(maskRange[1], (UP_LINE + DOWN_LINE) / 2);
        ctx.lineTo((maskRange[0] + maskRange[1]) / 2 + textLength / 2, (UP_LINE + DOWN_LINE) / 2);
        ctx.strokeStyle = theme.timeDiffPictureColor;
        ctx.stroke();
    }
};

const drawSelectedRange = (ctx: CanvasRenderingContext2D | null, selectedRange: Session['selectedRange'], xScale: (posX: number) => number): void => {
    if (ctx !== null && selectedRange !== undefined) {
        ctx.beginPath();
        ctx.moveTo(xScale(selectedRange[0]), 0);
        ctx.setLineDash([4, 2]);
        ctx.strokeStyle = '#5291FF';
        ctx.lineTo(xScale(selectedRange[0]), 9999);
        ctx.stroke();
        ctx.setLineDash([]);

        ctx.beginPath();
        ctx.moveTo(xScale(selectedRange[1]), 0);
        ctx.setLineDash([4, 2]);
        ctx.strokeStyle = '#5291FF';
        ctx.lineTo(xScale(selectedRange[1]), 9999);
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
    ctx, width, height, xReverseScale, xScale, interactorMouseState: {
        clickPos: { current: clickPos },
        lastPos: { current: mousePosNow },
    }, selectedRange, isNsMode, session, theme,
}: DrawArgs & { ctx: CanvasRenderingContext2D }): void => {
    let maskRange: number[] | undefined;
    if (clickPos !== undefined && mousePosNow !== undefined) {
        // 1st priority
        // is brushing
        maskRange = [clickPos.x, mousePosNow.x];
    } else if (selectedRange !== undefined) {
        // 2nd priority
        // not brushing now but has selected range
        maskRange = [xReverseScale(selectedRange[0]), xReverseScale(selectedRange[1])];
    }
    const elements = document.getElementsByClassName('chart-selected');
    if (maskRange !== undefined) {
        maskRange.sort((a, b) => a - b);
        ctx.fillStyle = 'rgba(66,66,66,0.5)';
        if (session.selectedUnits.length !== 0) {
            ctx.fillRect(0, TIME_LINE_AXIS_HEIGHT_PX, width, height);
            if (elements.length !== 0) {
                Array.of(...elements).forEach(element => {
                    const rect = element.getBoundingClientRect();
                    const scrollContainer = getParentNodeByClassName(element, 'laneWrapper');
                    const containerRect = scrollContainer?.getBoundingClientRect();
                    const top = containerRect ? Math.max(rect.top, containerRect.top) : rect.top;
                    const bottom = containerRect ? Math.min(rect.bottom, containerRect.bottom) : rect.bottom;
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
        drawTimeDiff(ctx, maskRange, xScale, isNsMode, theme, selectedRange);
    }
};

export interface DrawArgs {
    ctx: CanvasRenderingContext2D | null;
    width: number;
    height: number;
    xReverseScale: (ts: number) => number;
    xScale: (posX: number) => number;
    interactorMouseState: InteractorMouseState;
    selectedRange: undefined | [number, number];
    isNsMode: boolean;
    session: Session;
    theme: Theme;
}
export const drawOnMove = ({
    ctx, width, height, xReverseScale, xScale, interactorMouseState, selectedRange, isNsMode, session, theme,
}: DrawArgs): void => {
    if (ctx === null) { return; }
    // clear all
    ctx.clearRect(0, 0, width, height);
    // draw mask

    drawMaskRange({ ctx, width, height, xReverseScale, xScale, interactorMouseState, selectedRange, isNsMode, session, theme });

    // should filter on data type
    drawSelectedRange(ctx, selectedRange, xReverseScale);
    const { clickPos: { current: clickPos }, lastPos: { current: mousePosNow } } = interactorMouseState;
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
// 是否是缩略图
const threadIsCol: Map<string, boolean> = new Map();
const updateUnitHeight = (session: Session, pinnedAreaHeight: number): void => {
    const height = pinnedAreaHeight;

    const computeUnitHeight = (units: InsightUnit[], height: number): number => {
        for (const unit of units) {
            if (!unit.isDisplay) {
                continue;
            }
            const metadata = unit.metadata as ThreadMetaData;
            if (metadata.threadId !== undefined && metadata.processId !== undefined) {
                heightMap.set(`${metadata.cardId}-${metadata.processId}-${metadata.threadId}`, height);
                if (unit.collapsible && !unit.isExpanded) {
                    threadIsCol.set(`${metadata.cardId}-${metadata.processId}-${metadata.threadId}`, true);
                }
            }
            height += unit.height() + 1;
            if (unit.children && unit.isExpanded) {
                height = computeUnitHeight(unit.children, height);
            }
        }
        return height;
    };
    computeUnitHeight(session.units, height);
};
export const draw = (ctx: CanvasRenderingContext2D | null, width: number, height: number,
    xReverseScale: (ts: number) => number, xScale: (posX: number) => number,
    interactorMouseState: InteractorMouseState, selectedRange: undefined | [number, number], isNsMode: boolean,
    session: Session, customRenderers: CustomCrossRenderer[], theme: Theme): void => {
    if (ctx === null) { return; }
    // clear all
    ctx.clearRect(0, 0, width, height);
    drawMaskRange({ ctx, width, height, interactorMouseState, xReverseScale, xScale, selectedRange, session, isNsMode, theme });

    // should filter on data type
    drawSelectedRange(ctx, selectedRange, xReverseScale);

    heightMap.clear();
    threadIsCol.clear();
    const pinnedScrollArea = document.getElementsByClassName('pinnedScrollArea');
    const pinnedAreaHeight = pinnedScrollArea[0]?.clientHeight ?? 0;
    updateUnitHeight(session, pinnedAreaHeight);
    drawLinkLines(ctx, session, xReverseScale, theme, pinnedAreaHeight);
};

const UNDRAW_HEIGHT = 45;
const getHeight = (session: Session, data: DataBlock, cardId: string): number | undefined => {
    let height;
    const unitHeight = heightMap.get(`${cardId}-${data.pid}-${data.tid}`);
    const isCol = threadIsCol.get(`${cardId}-${data.pid}-${data.tid}`);
    if (unitHeight !== undefined && isCol) {
        // 缩略泳道连线位置
        height = UNDRAW_HEIGHT + unitHeight - session.scrollTop + (0.5 * UnitHeight.COLL);
    } else {
        // 展开泳道连线位置
        height = UNDRAW_HEIGHT + unitHeight - session.scrollTop + (data.depth + 0.5) * UnitHeight.STANDARD;
    }
    return height;
};

function drawSingleLinkLine(data: Record<string, unknown>, checkedCategory: string, session: Session, ctx: CanvasRenderingContext2D, theme: Theme): void {
    const { category, from, to, cardId } = data as unknown as FlowEvent;
    if (category !== checkedCategory) {
        return;
    }
    const timestampOffset = cardId !== undefined
        ? (session?.unitsConfig.offsetConfig.timestampOffset as Record<string, number>)?.[cardId] ?? 0
        : 0;
    const li = d3.scaleLinear().range([0, ctx.canvas.width])
        .domain([session.domainRange.domainStart + timestampOffset, session.domainRange.domainEnd + timestampOffset]);
    const targetX = li(to.timestamp);
    const targetY = getHeight(session, to, cardId);
    const sourceX = li(from.timestamp);
    const sourceY = getHeight(session, from, cardId);
    if ((sourceY === undefined || targetY === undefined)) {
        return;
    }
    if (sourceY < UNDRAW_HEIGHT && targetY < UNDRAW_HEIGHT) {
        return;
    }
    const targetPos: Array<[x: number, y: number]> = [[targetX, targetY]];

    const offset = ((targetX - sourceX) / 2);
    ctx.moveTo(sourceX, sourceY);
    ctx.bezierCurveTo(sourceX + offset, sourceY, targetX - offset, targetY, targetX, targetY);
    ctx.stroke();
    if (targetY >= UNDRAW_HEIGHT && sourceY >= UNDRAW_HEIGHT) {
        const len = targetPos.length;
        let [fromX, fromY] = targetPos.reduce(([prevX, prevY], [x, y]) => [prevX + x + offset, prevY + y], [0, 0]);
        fromX = fromX / len;
        fromY = fromY / len;
        const yLen = Math.abs(fromY - targetY);
        const xLen = Math.abs(fromX - targetX);
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

const drawLinkLines = (ctx: CanvasRenderingContext2D, session: Session, xScale: Scale, theme: Theme, pinnedAreaHeight: number): void => {
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
