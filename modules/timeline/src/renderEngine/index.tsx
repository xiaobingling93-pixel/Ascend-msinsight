/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
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

import { throttle } from 'lodash';
import { customConsole as console } from '@insight/lib/utils';

type RenderID = number;
interface Task {
    action: () => void;
    status: 'pending' | 'fullfilled';
    type: 'once' | 'always';
}
const RENDER_FREQUENCY = 16;
export class RenderEngine {
    private readonly _renderTasks: Map<RenderID, Task> = new Map();
    private _curTaskID: number = 0;
    private _status: 'running' | 'waiting' = 'running';

    private readonly run = throttle((): void => {
        requestAnimationFrame(() => {
            this.flush();
            if (this._status === 'running') {
                this.run();
            }
        });
    }, RENDER_FREQUENCY);

    constructor() {
        this.run();
    }

    start(): void {
        this._status = 'running';
    }

    stop(): void {
        this._status = 'waiting';
    }

    addTask(action: () => void, type: 'once' | 'always' = 'always'): RenderID {
        const renderID = this._curTaskID;
        this._renderTasks.set(renderID, { action, status: 'pending', type });
        this._curTaskID++;
        return renderID;
    }

    deleteTask(renderID: RenderID): void {
        this._renderTasks.delete(renderID);
    }

    private flush(): void {
        this._renderTasks.forEach((task) => {
            if (typeof task.action !== 'function') {
                console.warn('render callback is not a function, please check your parameter of draw()');
                return;
            }
            if (task.status === 'pending') {
                task.action();
                task.type === 'once' && (task.status = 'fullfilled');
            }
        });
    }
}

export const renderEngine = new RenderEngine();
