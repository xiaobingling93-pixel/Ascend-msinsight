/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { throttle } from 'lodash';
import { customConsole as console } from 'ascend-utils';

type RenderID = number;
interface Task {
    action: () => void;
    status: 'pending' | 'fullfilled';
    type: 'once' | 'always';
}
const RENDER_FREQUENCY = 100;
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
            };
        });
    }
};

export const renderEngine = new RenderEngine();
