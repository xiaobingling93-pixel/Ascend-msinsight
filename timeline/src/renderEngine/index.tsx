import { throttle } from 'lodash';

type RenderID = number;
const RENDER_FREQUENCY = 100;
export class RenderEngine {
    private readonly _renderTasks: Map<RenderID, Function> = new Map();
    private _status: 'idle' | 'running' = 'running';
    private _curTaskID: number = 0;

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
            if (typeof task !== 'function') {
                console.warn('render callback is not a function, please check your parameter of draw()');
                return;
            }
            task();
        });
    }

    start(): void {
        this._status = 'running';
    }

    stop(): void {
        this._status = 'idle';
    }

    addTask(task: Function): RenderID {
        const renderID = this._curTaskID;
        this._renderTasks.set(renderID, task);
        this._curTaskID++;
        return renderID;
    }

    deleteTask(renderID: RenderID): void {
        this._renderTasks.delete(renderID);
    }
};

export const renderEngine = new RenderEngine();
