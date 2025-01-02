/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import type { RefObject } from 'react';
import { ParallelismType } from '../../utils/interface';
import { FrameGroupItem } from './ContainerUtils';
import { FONT_FAMILY } from 'ascend-utils';

interface Position {
    x: number;
    y: number;
}

interface RectangleAttribute {
    dpSize: number;
    ppSize: number;
    tpSize: number;
    epSize: number;
    cpSize: number;
    dpIndex: number;
    ppIndex: number;
    tpIndex: number;
    epIndex: number;
    cpIndex: number;
}

interface RectangleProps {
    index: number;
    rowAndCol: Position;
    attribute: RectangleAttribute;
    name?: string;
    fillColor?: string;
    backgroundColor?: string;
    color?: string;
}

type LinePosition = [number, number, number, number];

const colorsMap = {
    tp: '#01CEB0',
    pp: '#0277FF',
    dp: '#6948C9',
    ep: '#EE891D',
    cp: '#FD2F2F',
};

export abstract class Shape {
    // 绘制图形
    abstract draw(context: CanvasRenderingContext2D, scrollLeft?: number, scrollTop?: number): void;

    // 判断点击是否在图形内部(线、框上)
    abstract isInside(x: number, y: number): boolean;
}

export class Rectangle extends Shape {
    gap = 20;
    yGap = 40;
    cpGap = 10;
    dpGap = 20;
    rowIndex: number;
    colIndex: number;
    textHeight = 0; // 底部文字

    index: number;
    rowAndCol: Position;
    name: string;
    attribute: RectangleAttribute;
    fillColor?: string;
    color: string;
    backgroundColor: string;
    scrollLeft: number = 0;
    scrollTop: number = 0;

    constructor({ index, rowAndCol, attribute, name = '', fillColor, color = 'black', backgroundColor = 'black' }: RectangleProps) {
        super();
        this.rowIndex = rowAndCol.x;
        this.colIndex = rowAndCol.y;
        this.index = index;
        this.rowAndCol = rowAndCol;
        this.name = name;
        this.fillColor = fillColor;
        this.backgroundColor = backgroundColor;
        this.color = color;
        this.attribute = attribute;
    }

    get originalX(): number {
        return (this.rowIndex * this.width) + this.totalGap + CanvasDrawer.PADDING;
    }

    get originalY(): number {
        const yIndex = this.colIndex;
        return (this.colIndex * this.height) + CanvasDrawer.PADDING + (this.colIndex * this.textHeight) + (yIndex * this.yGap);
    }

    get x(): number {
        return this.originalX - this.scrollLeft;
    }

    get y(): number {
        return this.originalY - this.scrollTop;
    }

    get width(): number {
        return 44;
    }

    get height(): number {
        return 44;
    }

    get totalGap(): number {
        let val = this.rowIndex * this.gap;

        if (this.attribute !== undefined) {
            const { cpIndex, dpIndex, cpSize } = this.attribute;
            const dpGaps = dpIndex * (this.dpGap + ((cpSize - 1) * this.cpGap));
            const cpGaps = cpIndex * this.cpGap;
            val = val + dpGaps + cpGaps;
        }

        return val;
    }

    drawInnerName(ctx: CanvasRenderingContext2D | null): void {
        if (!ctx || !this.name) {
            return;
        }
        const textList = this.name.split('-');
        const lineHeight = 16; // 行高
        const totalHeight = textList.length * lineHeight;
        const diff = 2;
        const startY = this.y + ((this.height - totalHeight) / 2) + diff;
        ctx.font = `12px ${FONT_FAMILY}`;
        ctx.textAlign = 'center';
        ctx.textBaseline = 'top';
        ctx.fillStyle = this.color;
        textList.forEach((item, index) => {
            ctx.fillText(item, this.x + (this.width / 2), startY + (index * lineHeight), this.width);
        });
    }

    drawInnerIndex(ctx: CanvasRenderingContext2D | null): void {
        if (!ctx) {
            return;
        }

        ctx.font = `12px ${FONT_FAMILY}`;
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillStyle = this.color;
        ctx.fillText(this.index.toString(), this.x + (this.width / 2), this.y + (this.height / 2));
    }

    drawBottomText(ctx: CanvasRenderingContext2D | null): void {
        if (!ctx) {
            return;
        }
        ctx.font = `12px ${FONT_FAMILY}`;
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillStyle = this.color;
        ctx.fillText(this.index.toString(), this.x + (this.width / 2), this.y + this.height + (this.textHeight / 2));
    }

    draw(ctx?: CanvasRenderingContext2D | null, scrollLeft: number = 0, scrollTop: number = 0): void {
        if (!ctx) {
            return;
        }

        this.scrollLeft = scrollLeft;
        this.scrollTop = scrollTop;

        ctx.save();
        if (this.fillColor !== undefined && this.fillColor !== '') {
            ctx.fillStyle = 'white';
            ctx.fillRect(this.x, this.y, this.width, this.height);
            ctx.fillStyle = this.fillColor;
            ctx.fillRect(this.x, this.y, this.width, this.height);
        } else {
            ctx.fillStyle = this.backgroundColor;
            ctx.lineWidth = 2;
            ctx.fillRect(this.x, this.y, this.width, this.height);
        }
        ctx.restore();

        this.drawInnerIndex(ctx);

        if (this.name) {
            this.drawInnerName(ctx);
        }

        if (this.textHeight) {
            this.drawBottomText(ctx);
        }
    }

    isInside(x: number, y: number): boolean {
        return x >= this.x &&
            x <= this.x + this.width &&
            y >= this.y &&
            y <= this.y + this.height;
    }
}

interface ParallelismSize {
    tpSize: number;
    dpSize: number;
    cpSize: number;
    epSize: number;
    ppSize: number;
}

// 连线（通信域）(group)
export class Line extends Shape {
    static CLICK_TOLERANCE = 4; // 点击连线的容错范围
    type: ParallelismType;
    rectList: FrameGroupItem['list'] = [];
    lineList: LinePosition[] = [];
    parallelismSize: ParallelismSize;
    ppOffset = -9;
    epOffsetX = -3;
    epOffsetY = 6;
    dpOffsetX = 3;
    dpOffsetY = 0;
    cpOffsetX = 9;
    cpOffsetY = -8;
    scrollLeft: number = 0;
    scrollTop: number = 0;
    constructor(type: ParallelismType, list: FrameGroupItem['list'], parallelismSize: ParallelismSize) {
        super();

        this.type = type;
        this.rectList = list;
        this.parallelismSize = parallelismSize;
        this.getLineDetails();
    }

    drawLine(ctx: CanvasRenderingContext2D, color: string, [startX, startY, endX, endY]: LinePosition): void {
        ctx.beginPath();
        ctx.moveTo(startX - this.scrollLeft, startY - this.scrollTop);
        ctx.lineTo(endX - this.scrollLeft, endY - this.scrollTop);
        ctx.strokeStyle = color;
        ctx.lineWidth = 2;
        ctx.stroke();
    }

    getLineDetails(): void {
        const len = this.rectList.length;
        const horizontalLine: number[] = [];
        this.rectList.forEach(({ index: rectIndex, position, attribute }, index) => {
            const rect = new Rectangle({
                index: rectIndex,
                rowAndCol: position,
                attribute: {
                    ...attribute,
                    ...this.parallelismSize,
                },
            });
            switch (this.type) {
                case 'pp':
                    if (index === len - 1) { return; }
                    this.lineList.push([
                        rect.x + (rect.width / 2) + this.ppOffset, rect.y + rect.height,
                        rect.x + (rect.width / 2) + this.ppOffset, rect.y + rect.height + rect.textHeight + rect.yGap]);
                    break;
                case 'tp':
                    if (index === len - 1) { return; }
                    this.lineList.push([
                        rect.x + rect.width, rect.y + (rect.height / 2),
                        rect.x + rect.width + rect.gap, rect.y + (rect.height / 2),
                    ]);
                    break;
                case 'dp':
                case 'cp':
                case 'ep': {
                    const offsetX = this[`${this.type}OffsetX`];
                    const offsetY = this[`${this.type}OffsetY`];
                    this.lineList.push([
                        rect.x + (rect.width / 2) + offsetX, rect.y + rect.height,
                        rect.x + (rect.width / 2) + offsetX, rect.y + rect.height + rect.textHeight + (rect.yGap / 2) + offsetY,
                    ]);
                    if (index === 0 || index === len - 1) {
                        horizontalLine.push(rect.x + (rect.width / 2) + offsetX, rect.y + rect.height + rect.textHeight + (rect.yGap / 2) + offsetY);
                        if (index === len - 1) {
                            this.lineList.push(horizontalLine as LinePosition);
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        });
    }

    draw(ctx: CanvasRenderingContext2D | null, scrollLeft: number = 0, scrollTop: number = 0): void {
        if (ctx === null) {
            return;
        }

        this.scrollLeft = scrollLeft;
        this.scrollTop = scrollTop;

        this.lineList.forEach(lines => {
            this.drawLine(ctx, colorsMap[this.type], lines);
        });
    }

    isInside(x: number, y: number): boolean {
        for (const line of this.lineList) {
            const [x1, y1, x2, y2] = line;
            const distance = distanceToLine({
                x1: x1 - this.scrollLeft,
                y1: y1 - this.scrollTop,
                x2: x2 - this.scrollLeft,
                y2: y2 - this.scrollTop,
                x,
                y,
            });

            // 假设点击位置距离线段的距离小于 CLICK_TOLERANCE px 时认为点击了线
            if (distance < Line.CLICK_TOLERANCE) {
                return true;
            }
        }

        return false;
    };
}

// 框（并行组）(domain)
export class Frame extends Shape {
    static FRAME_TOLERANCE = 3; // 点击框的容错范围
    type: ParallelismType;
    rectList: FrameGroupItem['list'];
    parallelismSize: ParallelismSize;
    boundingBox: {x: number;y: number;width: number; height: number} | null = null;
    tpOffset = 8;
    ppOffset = 3;
    cpOffset = 10;
    dpOffset = 14;
    epOffset = 18;

    constructor(type: FrameGroupItem['type'], frameList: FrameGroupItem['list'], parallelismSize: ParallelismSize) {
        super();
        this.type = type;
        this.rectList = frameList;
        this.parallelismSize = parallelismSize;
    }

    draw(ctx: CanvasRenderingContext2D | null, scrollLeft: number = 0, scrollTop: number = 0): void {
        if (ctx === null) {
            return;
        }

        ctx.strokeStyle = colorsMap[this.type];
        ctx.lineWidth = 2;

        const { index: firstRankIndex, position: firstRankPosition, attribute: firstRankAttribute } = this.rectList[0];
        const { index: lastRankIndex, position: lastRankPosition, attribute: lastRankAttribute } = this.rectList[this.rectList.length - 1];
        const { x: firstX, y: firstY, width, height, textHeight } = new Rectangle({
            index: firstRankIndex,
            rowAndCol: firstRankPosition,
            attribute: {
                ...firstRankAttribute,
                ...this.parallelismSize,
            },
        });
        const { x: lastX, y: lastY } = new Rectangle({
            index: lastRankIndex,
            rowAndCol: lastRankPosition,
            attribute: {
                ...lastRankAttribute,
                ...this.parallelismSize,
            },
        });
        const offset = this[`${this.type}Offset`];
        const frameX = firstX - offset;
        const frameY = firstY - offset;
        const frameWidth = lastX - frameX + width + offset;
        const frameHeight = lastY - frameY + height + textHeight + offset;
        const scrolledFrameX = frameX - scrollLeft;
        const scrolledFrameY = frameY - scrollTop;

        ctx.strokeRect(scrolledFrameX, scrolledFrameY, frameWidth, frameHeight);

        this.boundingBox = {
            x: scrolledFrameX, y: scrolledFrameY, width: frameWidth, height: frameHeight,
        };
    }

    isInside(x: number, y: number): boolean {
        if (!this.boundingBox) {
            return false;
        }

        const { x: frameX, y: frameY, width, height } = this.boundingBox;

        // 上边框
        if (isInFrameBorderRange(x, frameX, frameX + width) && isInFrameBorderRange(y, frameY, frameY)) {
            return true;
        }

        // 下边框
        if (isInFrameBorderRange(x, frameX, frameX + width) && isInFrameBorderRange(y, frameY + height, frameY + height)) {
            return true;
        }

        // 左边框
        if (isInFrameBorderRange(x, frameX, frameX) && isInFrameBorderRange(y, frameY, frameY + height)) {
            return true;
        }

        // 右边框
        if (isInFrameBorderRange(x, frameX + width, frameX + width) && isInFrameBorderRange(y, frameY, frameY + height)) {
            return true;
        }

        return false;
    }
}

export class CanvasDrawer {
    static PADDING = 40;
    private readonly rectangles: Rectangle[] = [];
    private readonly lines: Line[] = [];
    private readonly frames: Frame[] = [];
    private readonly ctx: CanvasRenderingContext2D | null = null;
    private readonly canvasRef: RefObject<HTMLCanvasElement>;
    private scrollLeft: number = 0;
    private scrollTop: number = 0;

    constructor(canvasRef: RefObject<HTMLCanvasElement>) {
        const ctx = canvasRef.current?.getContext('2d');
        this.canvasRef = canvasRef;
        if (ctx !== null && ctx !== undefined) {
            this.ctx = ctx;
        }
    }

    get rectangleList(): Rectangle[] {
        return this.rectangles;
    }

    get lineList(): Line[] {
        return this.lines;
    }

    get frameList(): Frame[] {
        return this.frames;
    }

    get visibleRectangleList(): Rectangle[] {
        const viewportWidth = this.canvasRef.current?.width ?? 0;
        return this.rectangles.filter(rect =>
            rect.originalX + rect.width > this.scrollLeft &&
            rect.originalX < this.scrollLeft + viewportWidth,
        );
    }

    addRectangle(rect: Rectangle): void {
        this.rectangles.push(rect);
    }

    addLine(line: Line): void {
        this.lines.push(line);
    }

    addFrame(frame: Frame): void {
        this.frames.push(frame);
    }

    clearRectangles(): void {
        this.rectangles.length = 0;
    }

    clearLines(): void {
        this.lines.length = 0;
    }

    clearFrames(): void {
        this.frames.length = 0;
    }

    render(scrollLeft: number = 0, scrollTop: number = 0): void {
        this.scrollLeft = scrollLeft;
        this.scrollTop = scrollTop;
        this.ctx?.resetTransform();
        this.ctx?.scale(devicePixelRatio, devicePixelRatio);
        this.clearCanvas();
        this.visibleRectangleList.forEach(rect => rect.draw(this.ctx, scrollLeft, scrollTop));
        this.lines.forEach(line => line.draw(this.ctx, scrollLeft, scrollTop));
        this.frames.forEach(frame => frame.draw(this.ctx, scrollLeft, scrollTop));
    }

    clearCanvas(): void {
        const { width = 1000, height = 1000 } = this.canvasRef.current ?? {};
        this.ctx?.clearRect(0, 0, width, height);
    }

    clearShapesData(): void {
        this.clearRectangles();
        this.clearLines();
        this.clearFrames();
    }
}

const isInFrameBorderRange = (value: number, rangeStart: number, rangeEnd: number): boolean =>
    value >= rangeStart - Frame.FRAME_TOLERANCE && value <= rangeEnd + Frame.FRAME_TOLERANCE;

const distanceToLine = ({ x1, y1, x2, y2, x, y }: {x1: number; y1: number; x2: number; y2: number; x: number; y: number}): number => {
    const dx = x2 - x1;
    const dy = y2 - y1;
    const lineLengthSquared = (dx * dx) + (dy * dy);

    // 如果线段长度为 0，则返回到该点的距离
    if (lineLengthSquared === 0) {
        return Math.hypot(x - x1, y - y1);
    }

    const t = (((x - x1) * dx) + ((y - y1) * dy)) / lineLengthSquared;

    // 将 t 限制在 [0, 1] 范围内，以确保最近的点在线段上
    const clampedT = Math.max(0, Math.min(1, t));

    const closestX = x1 + (clampedT * dx);
    const closestY = y1 + (clampedT * dy);

    return Math.hypot(x - closestX, y - closestY);
};
