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
import { makeAutoObservable } from 'mobx';
import { type MenuItemModel } from '@/components/ContextMenu';
interface ContextMenu {
    visible: boolean;
    xPos: number;
    yPos: number;
}

const LEAKS_WORKER_INFO_DEFAULT = {
    sizeInfo: {
        maxTimestamp: 0,
        minTimestamp: 0,
        maxSize: 0,
        minSize: 0,
    },
    renderOptions: {
        transform: {
            x: 0,
            y: 0,
            scale: 1,
        },
        viewport: {
            width: 0,
            height: 0,
        },
        zoom: {
            x: 1,
            y: 1,
            offset: 0,
        },
    },
    mousePosition: {
        x: -1,
        y: -1,
    },
    hoverItem: null,
    clickItem: null,
};

const MARK_LINE_POSITION_DEFAULT = {
    block: { x: -1, y: -1 },
    stack: { x: -1, y: -1 },
    currentTimestamp: -1,
    clickTimestamp: -1,
};

const STATE_WORKER_INFO_DEFAULT = {
    renderOptions: {
        transform: { x: 0, y: 0, scale: 1 },
        viewport: { width: 0, height: 0 },
        zoom: { x: 1, y: 1, offset: 0 },
    },
    hoverItem: null,
    clickItem: null,
};

export class Session {
    language: 'zhCN' | 'enUS' = 'enUS';
    tritonParsed: boolean = false;
    renderId: number = 1;
    maxTime: number = 0; // x轴最大值
    minTime: number = 0; // x轴最小值
    contextMenu: ContextMenu = { visible: false, xPos: 0, yPos: 0 };
    menuItems: MenuItemModel[] = [];

    leaksWorkerInfo: {
        sizeInfo: Omit<RenderData, 'blocks'>;
        renderOptions: RenderOptions;
        hoverItem: Block | null;
        clickItem: Block | null;
    } = LEAKS_WORKER_INFO_DEFAULT;

    markLineInfo: {
        block: { x: number; y: number };
        stack: { x: number; y: number };
        currentTimestamp: number;
        clickTimestamp: number;
    } = MARK_LINE_POSITION_DEFAULT;

    stateWorkerInfo: {
        renderOptions: RenderOptions;
        hoverItem: StateDataHoverResult | null;
        clickItem: StateDataHoverResult | null;
    } = STATE_WORKER_INFO_DEFAULT;

    constructor() {
        makeAutoObservable(this);
    }
}
