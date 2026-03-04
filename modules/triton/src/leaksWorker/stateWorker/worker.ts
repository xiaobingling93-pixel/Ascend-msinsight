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

export const StateWorker = new Worker(new URL('./', import.meta.url));

// 向worker发送消息
export const workerInitCanvas = ({ canvas, width, height }: Omit<InitCanvasPayload, 'type' | 'devicePixelRatio'>): void => {
    const devicePixelRatio = window.devicePixelRatio || 1;
    const offscreenCanvas = (canvas as HTMLCanvasElement).transferControlToOffscreen();
    StateWorker.postMessage(
        { type: 'initCanvas', canvas: offscreenCanvas, devicePixelRatio, width, height },
        [offscreenCanvas],
    );
};

export const workerSetMemoryStateData = ({ data }: Omit<SetMemoryStateDataPayload, 'type'>): void => {
    StateWorker.postMessage({ type: 'setMemoryStateData', data });
};

export const workerResizeCanvas = ({ width, height }: Omit<ResizeCanvasPayload, 'type'>): void => {
    StateWorker.postMessage({ type: 'resizeCanvas', width, height });
};

export const workerTransform = ({ transform }: Omit<TransformPayload, 'type'>): void => {
    StateWorker.postMessage({ type: 'transform', transform });
};

export const workerHoverItem = ({ clientX, clientY }: Omit<HoverItemPayload, 'type'>): void => {
    StateWorker.postMessage({ type: 'hoverItem', clientX, clientY });
};

export const workerClickItem = ({ clientX, clientY }: Omit<HoverItemPayload, 'type'>): void => {
    StateWorker.postMessage({ type: 'clickItem', clientX, clientY });
};

export const workerDestroy = (): void => {
    StateWorker.postMessage({ type: 'destroy' });
};
