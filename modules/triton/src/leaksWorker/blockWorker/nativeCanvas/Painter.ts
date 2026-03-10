/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

import { getColorStringByAddr } from '@/leaksWorker/tools/color';

export class Painter {
    readonly canvas: HTMLCanvasElement;
    readonly devicePixelRatio: number;
    private context: CanvasRenderingContext2D | null = null;
    private data: RenderData['blocks'] = [];
    private highlightData: RenderData['blocks'] = [];

    constructor(canvas: HTMLCanvasElement, devicePixelRatio: number) {
        this.canvas = canvas;
        this.devicePixelRatio = devicePixelRatio;
    }

    async initialize(): Promise<void> {
        this.context = this.canvas.getContext('2d');
    }

    processData(data: RenderData['blocks'] = []): void {
        this.data = data;
    }

    processHighlightData(highlightData: RenderData['blocks'] = []): void {
        this.highlightData = highlightData;
    }

    render(options: RenderOptions): void {
        if (this.context === null) {
            return;
        }
        const { transform, viewport } = options;
        this.context.resetTransform();
        this.context.clearRect(0, 0, viewport.width, viewport.height);
        this.context.translate(transform.x, viewport.height - transform.y);
        this.context.scale(transform.scale, -transform.scale);
        this.context.save();
        this.renderData(this.data, options);
        this.renderData(this.highlightData, options, true);
    }

    renderData(data: RenderData['blocks'], options: RenderOptions, isHighlight: boolean = false): void {
        for (let i = 0; i < data.length; i++) {
            const { path, size, addr } = data[i];
            for (let j = 0; j < path.length - 1; j++) {
                this.drawShape(path[j], path[j + 1], size, addr, options, isHighlight);
            }
        }
    }

    drawShape(p0: [number, number], p1: [number, number], size: number, addr: string, options: RenderOptions, isHighlight: boolean): void {
        if (this.context === null) {
            return;
        }
        const { zoom } = options;

        const lx = (p0[0] - zoom.offset) * zoom.x;
        const ly = p0[1] * zoom.y;
        const rx = (p1[0] - zoom.offset) * zoom.x;
        const ry = p1[1] * zoom.y;
        const h = size * zoom.y;

        // 绘制四边形
        this.context.beginPath();
        this.context.moveTo(lx, ly);
        this.context.lineTo(lx, ly + h);
        this.context.lineTo(rx, ry + h);
        this.context.lineTo(rx, ry);
        this.context.closePath();

        this.context.fillStyle = getColorStringByAddr(addr, isHighlight); // 填充颜色
        this.context.fill(); // 填充图形
    }
}
