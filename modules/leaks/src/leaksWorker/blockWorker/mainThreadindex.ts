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

import { buildBlockViewPath, getZoom, searchBlockDataByPoint } from '../tools/dataProcess';
import { debounce } from 'lodash';
import { NativeRenderer } from './nativeCanvas/NativeRenderer';
import { store } from '@/store';
import { Session } from '@/entity/session';
import { runInAction } from 'mobx';
export class MainThreadRender {
    canvas: HTMLCanvasElement = document.createElement('canvas');
    memoryBlockData: RenderData = { maxTimestamp: 0, minTimestamp: 0, maxSize: 0, minSize: 0, blocks: [] };
    transform: RenderOptions['transform'] = { x: 0, y: 0, scale: 1 };
    viewport: RenderOptions['viewport'] = { width: 0, height: 0 };
    zoom: RenderOptions['zoom'] = { x: 1, y: 1, offset: 0 };
    renderer: NativeRenderer | null = null;
    session: Session;

    constructor() {
        const { sessionStore } = store;
        this.session = sessionStore.activeSession as Session;
    }

    async initCanvasHandler(payload: Omit<InitCanvasPayload, 'type'>): Promise<void> {
        this.canvas = payload.canvas as HTMLCanvasElement;
        this.renderer = new NativeRenderer(this.canvas, devicePixelRatio);
        this.viewport = { width: payload.width, height: payload.height };
        await this.renderer.initialize();
    };

    setMemoryBlockDataHandler(payload: Omit<SetMemoryBlocksDataPayload, 'type'>): void {
        this.memoryBlockData = buildBlockViewPath(payload.data);
        const { maxTimestamp, minTimestamp, maxSize, minSize } = this.memoryBlockData;
        this.zoom = getZoom(this.memoryBlockData, this.canvas);
        runInAction(() => {
            this.session.leaksWorkerInfo.sizeInfo = {
                maxTimestamp,
                minTimestamp,
                maxSize,
                minSize,
            };
            this.session.leaksWorkerInfo.renderOptions.zoom = this.zoom;
        });
        this.renderer?.setZoom(this.zoom).setData(this.memoryBlockData.blocks);
        this.renderer?.updateCanvasSize(this.viewport);
    };

    resizeCanvasHandler(payload: Omit<ResizeCanvasPayload, 'type'>): void {
        this.viewport = { width: payload.width, height: payload.height };
        this.renderer?.updateCanvasSize(this.viewport);
        if (this.memoryBlockData === undefined) {
            return;
        }
        this.zoom = getZoom(this.memoryBlockData, this.canvas);
        this.renderer?.setZoom(this.zoom);
    };

    transformHandler(payload: Omit<TransformPayload, 'type'>): void {
        this.transform = payload.transform;
        this.renderer?.setTransform(this.transform);
    };

    debouncedSearchBlockData = debounce((payload: Omit<HoverItemPayload, 'type'>): void => {
        if (this.memoryBlockData?.blocks?.length > 0) {
            const result = searchBlockDataByPoint(this.memoryBlockData.blocks, payload, this.transform, this.zoom);
            this.renderer?.setHighlightData(result === null ? [] : [result]);
            runInAction(() => {
                this.session.leaksWorkerInfo.hoverItem = result;
            });
        }
    }, 10);

    clickItemHandler(payload: Omit<HoverItemPayload, 'type'>): void {
        const result = searchBlockDataByPoint(this.memoryBlockData.blocks, payload, this.transform, this.zoom);
        runInAction(() => {
            this.session.leaksWorkerInfo.clickItem = result;
        });
    };

    hoverItemHandler(payload: Omit<HoverItemPayload, 'type'>): void {
        this.debouncedSearchBlockData(payload);
    };

    destroyHandler(): void {
        this.memoryBlockData = { maxTimestamp: 0, minTimestamp: 0, maxSize: 0, minSize: 0, blocks: [] };
        this.transform = { x: 0, y: 0, scale: 1 };
        this.zoom = { x: 1, y: 1, offset: 0 };
        runInAction(() => {
            this.session.leaksWorkerInfo.sizeInfo = {
                maxTimestamp: 0,
                minTimestamp: 0,
                maxSize: 0,
                minSize: 0,
            };
            this.session.leaksWorkerInfo.renderOptions.zoom = this.zoom;
            this.session.leaksWorkerInfo.clickItem = null;
        });
        this.renderer?.setData([]).setTransform(this.transform).setZoom(this.zoom);
    };
}
