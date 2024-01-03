import { throttle } from 'lodash';

type RenderID = number;
interface Task {
    action: Function;
    status: 'pending' | 'fullfilled';
    type: 'once' | 'always';
}
const RENDER_FREQUENCY = 100;
export class RenderEngine {
    private readonly _renderTasks: Map<RenderID, Task> = new Map();
    private _curTaskID: number = 0;
    private _status: 'running' | 'waiting' = 'running';

    constructor() {
        this.run();
    }

    private readonly run = throttle((): void => {
        requestAnimationFrame(() => {
            this.flush();
            if (this._status === 'running') {
                this.run();
            }
        });
    }, RENDER_FREQUENCY);

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

    start(): void {
        this._status = 'running';
    }

    stop(): void {
        this._status = 'waiting';
    }

    addTask(action: Function, type: 'once' | 'always' = 'always'): RenderID {
        const renderID = this._curTaskID;
        this._renderTasks.set(renderID, { action, status: 'pending', type });
        this._curTaskID++;
        return renderID;
    }

    deleteTask(renderID: RenderID): void {
        this._renderTasks.delete(renderID);
    }
};

export const renderEngine = new RenderEngine();
