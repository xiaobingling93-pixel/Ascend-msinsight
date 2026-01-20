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

export const getZoom = (data: RenderData, canvas: OffscreenCanvas): RenderOptions['zoom'] => {
    return {
        x: canvas.width / (data.maxTimestamp - data.minTimestamp),
        y: canvas.height / (data.maxSize - data.minSize),
        offset: data.minTimestamp,
    };
};
export const searchBlockDataByPoint = (
    data: RenderData['blocks'],
    { clientX, clientY }: Omit<HoverItemPayload, 'type'>,
    transform: RenderOptions['transform'],
    zoom: RenderOptions['zoom'],
): Block | null => {
    // 将鼠标点击位置转换为真实坐标
    const x = (clientX - transform.x) / zoom.x / transform.scale;
    const y = (clientY - transform.y) / zoom.y / transform.scale;

    for (let i = 0; i < data.length; i++) {
        const block = data[i];
        if (x < block._startTimestamp - zoom.offset) {
            return null; // 之后的所有数据都不会被选中
        }
        if (x > block._endTimestamp - zoom.offset) {
            continue;
        }
        if (block.path.length > 1) {
            for (let j = 0; j < block.path.length - 1; j++) {
                const startPt = block.path[j];
                const endPt = block.path[j + 1];
                // 坐标调零
                const lx = (startPt[0] - zoom.offset);
                const ly = startPt[1];
                const rx = (endPt[0] - zoom.offset);
                const ry = endPt[1];
                const h = block.size;
                if (x < lx) {
                    return null; // 之后的所有数据都不会被选中
                }
                if (y > ly + h) {
                    break; // 当前block中的数据都不会被选中
                }
                if (isPointInExtrudedSegment(x, y, lx, ly, rx, ry, h)) {
                    return block;
                }
            }
        }
    }
    return null; // 没有找到匹配的数据块
};

// 射线法计算点是否在四边形（矩形/平行四边形）范围内
const isPointInExtrudedSegment = (px: number, py: number, sx1: number, sy1: number, sx2: number, sy2: number, h: number): boolean => {
    const p0 = [sx1, sy1];
    const p1 = [sx1, sy1 + h];
    const p2 = [sx2, sy2 + h];
    const p3 = [sx2, sy2];
    const points = [p0, p1, p2, p3];
    let inside = false;
    for (let i = 0, j = points.length - 1; i < points.length; j = i++) {
        const xi = points[i][0];
        const yi = points[i][1];
        const xj = points[j][0];
        const yj = points[j][1];
        const intersect = ((yi > py) !== (yj > py)) && (px < (xj - xi) * (py - yi) / (yj - yi) + xi);
        if (intersect) {
            inside = !inside;
        }
    }
    return inside;
};

enum MemoryEventAction {
    Malloc = 0,
    Free = 1,
};
interface MemoryEvent {
    eventAction: MemoryEventAction;
    time: number;
    blockPtr: Block;
}

const addPathPoint = (block: Block, time: number, size: number): void => {
    if (block.path === undefined || block.path.length === 0) {
        block.path = [[time, size]];
        return;
    }
    const lastPoint = block.path[block.path.length - 1];
    // 如果新加入点点时间戳在最后一个点之前，视为无效点
    if (time < lastPoint[0]) {
        return;
    }
    // 如果目前只有一个点，直接添加
    if (block.path.length === 1) {
        block.path.push([time, size]);
        return;
    }
    // 检查是否可以压缩水平线段（三个连续点size相同)
    const secondLastPoint = block.path[block.path.length - 2];
    if (size === lastPoint[1] && size === secondLastPoint[1]) {
        // 合并：将最后一个点的timestamp更新为新的timestamp
        lastPoint[0] = time;
        return;
    }
    // 无法合并则正常添加新点
    block.path.push([time, size]);
};

export const buildBlockViewPath = (blockView: SetMemoryBlocksDataPayload['data']): RenderData => {
    if (blockView.blocks === undefined || blockView.blocks.length === 0) {
        return blockView;
    }
    // 将已基于开始时间排序的block数组，还原成基于开始、结束时间构造的事件
    const sortedEvents: MemoryEvent[] = [];
    for (const block of blockView.blocks) {
        sortedEvents.push(
            { eventAction: MemoryEventAction.Malloc, time: block._startTimestamp, blockPtr: block },
            { eventAction: MemoryEventAction.Free, time: block._endTimestamp, blockPtr: block },
        );
    }
    sortedEvents.sort((a, b) => a.time - b.time);

    const currentBlocks: Block[] = [];
    let currentTotalSize = 0;
    blockView.maxTimestamp = sortedEvents[sortedEvents.length - 1].time;
    blockView.minTimestamp = sortedEvents[0].time;
    blockView.minSize = 0;

    sortedEvents.forEach(({ eventAction, time, blockPtr }, index) => {
        // 如果是分配事件
        if (eventAction === MemoryEventAction.Malloc) {
            currentBlocks.push(blockPtr);
            addPathPoint(blockPtr, time, currentTotalSize);
            currentTotalSize += blockPtr.size;
            blockView.maxSize = currentTotalSize > blockView.maxSize ? currentTotalSize : blockView.maxSize;
            return;
        }
        // 否则为释放事件, 需要在currentBlocks中找到被释放的块，根据内存分配时的特征，使用倒序查找更合适
        let freeBlockIdx = -1;
        for (let i = currentBlocks.length - 1; i >= 0; i--) {
            const block = currentBlocks[i];
            currentTotalSize -= block.size;
            addPathPoint(block, time, currentTotalSize);
            if (block.id === blockPtr.id) {
                freeBlockIdx = i;
                break;
            }
        }
        if (freeBlockIdx < 0) {
            // 查找失败，报错返回
            return;
        }
        const freeBlock: Block = currentBlocks[freeBlockIdx];
        addPathPoint(freeBlock, time, currentTotalSize);
        currentBlocks.splice(freeBlockIdx, 1);
        const tmpTime: number = index + 1 < sortedEvents.length ? (time + sortedEvents[index + 1].time) / 2 : blockView.maxTimestamp;
        for (let i = freeBlockIdx; i <= currentBlocks.length - 1; i++) {
            const block = currentBlocks[i];
            addPathPoint(block, tmpTime, currentTotalSize);
            currentTotalSize += block.size;
        }
    });
    // 处理剩余块
    for (let i = currentBlocks.length - 1; i >= 0; i--) {
        currentTotalSize -= currentBlocks[i].size;
        addPathPoint(currentBlocks[i], blockView.maxTimestamp, currentTotalSize);
    }
    return blockView;
};
