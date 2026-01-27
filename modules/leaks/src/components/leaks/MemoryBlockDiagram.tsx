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

import React, { useEffect, useRef } from 'react';
import {
    workerInitCanvas,
    workerResizeCanvas,
    workerTransform,
    workerHoverItem,
    workerClickItem,
} from '@/leaksWorker/blockWorker/worker';
import { Session } from '@/entity/session';
import { runInAction } from 'mobx';
import { Axis, HoverItem, MarkLineBlock } from './tools';

export const MemoryBlockDiagram = ({ session }: { session: Session }): JSX.Element => {
    const containerRef = useRef<HTMLDivElement>(null);
    const ref = useRef<HTMLCanvasElement>(null);
    const isDragging = useRef(false);
    const isClick = useRef(false);
    const dragStartPoint = useRef({ x: 0, y: 0 });

    const handleResize = (): void => {
        if (ref.current === null || containerRef.current === null) {
            return;
        }
        const containerRect = containerRef.current.getBoundingClientRect();
        const width = containerRect.width - 100;
        const height = containerRect.height - 50;
        runInAction(() => {
            session.leaksWorkerInfo.renderOptions.viewport = { width, height };
        });
        workerResizeCanvas({ width, height });
    };

    const handleWheel = (ev: WheelEvent): void => {
        ev.preventDefault();

        if (ref.current === null) {
            return;
        }

        const rect = ref.current.getBoundingClientRect();

        // 计算鼠标相对于画布的坐标
        const mouseX = ev.clientX - rect.left;
        const mouseY = rect.height - (ev.clientY - rect.top);

        // 获取当前变换参数
        const currentTransform = session.leaksWorkerInfo.renderOptions.transform;

        // 计算缩放前鼠标在实际内容中的相对位置（相对于画布原点）
        const originalContentMouseX = (mouseX - currentTransform.x) / currentTransform.scale;
        const originalContentMouseY = (mouseY - currentTransform.y) / currentTransform.scale;

        // 计算新的缩放值
        const deltaScale = ev.deltaY > 0 ? -0.1 : 0.1;
        const newScale = Math.max(0.1, currentTransform.scale + deltaScale);

        const maxRangeX = rect.width;
        const minRangeX = -rect.width * newScale;
        const maxRangeY = rect.height;
        const minRangeY = -rect.height * newScale;
        // 计算缩放后的新偏移，使鼠标下的内容位置不变
        // 原始偏移距离 + (内容相对位置 * (新缩放 - 旧缩放))
        const newX = Math.min(Math.max(mouseX - originalContentMouseX * newScale, minRangeX), maxRangeX);
        const newY = Math.min(Math.max(mouseY - originalContentMouseY * newScale, minRangeY), maxRangeY);

        // 更新变换参数
        const transform = { x: newX, y: newY, scale: newScale };
        runInAction(() => {
            session.leaksWorkerInfo.renderOptions.transform = transform;
        });
        workerTransform({ transform });
    };

    const handleMouseDown = (ev: MouseEvent): void => {
        if (ev.button !== 0 || ref.current === null) {
            return;
        }
        isClick.current = true;
        const rect = ref.current.getBoundingClientRect();
        dragStartPoint.current = {
            x: ev.clientX - rect.left,
            y: ev.clientY - rect.top,
        };
    };

    const handleMouseUp = (): void => {
        isDragging.current = false;
    };

    const handleMouseLeave = (): void => {
        isDragging.current = false;
        isClick.current = false;
        runInAction(() => {
            session.markLineInfo.block = { x: -1, y: -1 };
            session.markLineInfo.stack = { x: -1, y: -1 };
        });
    };

    const handleMouseMove = (ev: MouseEvent): void => {
        if (ref.current === null) {
            return;
        }
        if (isClick.current) {
            isClick.current = false;
            isDragging.current = true;
        }
        const rect = ref.current.getBoundingClientRect();
        const currentX = ev.clientX - rect.left;
        const currentY = ev.clientY - rect.top;
        if (!isDragging.current) {
            workerHoverItem({ clientX: currentX, clientY: rect.height - currentY });
            runInAction(() => {
                session.markLineInfo.block = { x: currentX, y: currentY };
            });
            return;
        }
        runInAction(() => {
            session.markLineInfo.block = { x: -1, y: -1 };
        });

        const currentTransform = session.leaksWorkerInfo.renderOptions.transform;

        const deltaX = currentX - dragStartPoint.current.x;
        const deltaY = currentY - dragStartPoint.current.y;
        const maxRangeX = rect.width;
        const minRangeX = -rect.width * currentTransform.scale;
        const maxRangeY = rect.height;
        const minRangeY = -rect.height * currentTransform.scale;

        const transform = {
            ...currentTransform,
            x: Math.min(Math.max(currentTransform.x + deltaX, minRangeX), maxRangeX),
            y: Math.min(Math.max(currentTransform.y - deltaY, minRangeY), maxRangeY),
        };
        runInAction(() => {
            session.leaksWorkerInfo.renderOptions.transform = transform;
        });

        workerTransform({ transform });

        dragStartPoint.current = { x: currentX, y: currentY };
    };

    const handleClick = (ev: MouseEvent): void => {
        if (ref.current === null) {
            return;
        }
        if (isClick.current) {
            isClick.current = false;
            const rect = ref.current.getBoundingClientRect();
            workerClickItem({ clientX: ev.clientX - rect.left, clientY: rect.height - (ev.clientY - rect.top) });
            if (session.markLineInfo.block.x > -1) {
                runInAction(() => {
                    session.memoryStamp = Math.round(session.markLineInfo.currentTimestamp);
                });
            }
        }
    };

    useEffect(() => {
        if (ref.current === null || containerRef.current === null) {
            return;
        }
        const canvas = ref.current;
        try {
            const containerRect = containerRef.current.getBoundingClientRect();
            const width = containerRect.width - 100;
            const height = containerRect.height - 50;

            const offscreenCanvas = canvas.transferControlToOffscreen();
            runInAction(() => {
                session.leaksWorkerInfo.renderOptions.viewport = { width, height };
            });
            workerInitCanvas({ offscreenCanvas, width, height });
        } catch (_e) {
            // 进入这里，说明画布已经离屏代码，不需要做额外处理
        }
        handleResize();
    }, []);

    useEffect(() => {
        if (ref.current === null || containerRef.current === null) {
            return;
        }
        const canvas = ref.current;

        window.addEventListener('resize', handleResize);

        canvas.addEventListener('wheel', handleWheel, { passive: false, capture: true });
        canvas.addEventListener('mousedown', handleMouseDown);
        canvas.addEventListener('mousemove', handleMouseMove);
        canvas.addEventListener('mouseup', handleMouseUp);
        canvas.addEventListener('mouseleave', handleMouseLeave);
        canvas.addEventListener('click', handleClick);

        return () => {
            window.removeEventListener('resize', handleResize);

            canvas.removeEventListener('wheel', handleWheel, { capture: true });
            canvas.removeEventListener('mousedown', handleMouseDown);
            canvas.removeEventListener('mousemove', handleMouseMove);
            canvas.removeEventListener('mouseup', handleMouseUp);
            canvas.removeEventListener('mouseleave', handleMouseLeave);
            canvas.removeEventListener('click', handleClick);
        };
    }, []);

    return <div ref={containerRef} style={{ width: 'calc(100% - 100px)', height: 500, paddingLeft: 100, paddingTop: 20 }}>
        <div style={{ position: 'relative' }}>
            <Axis session={session} />
            <canvas
                ref={ref}
                style={{ imageRendering: 'pixelated', touchAction: 'none' }}
            />
            <MarkLineBlock session={session} />
            <HoverItem session={session} />
        </div>
    </div>;
};
