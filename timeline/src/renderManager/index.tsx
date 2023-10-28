export class RenderManager {
    private readonly _renderQueue: Function[] = [];
    private readonly _frequency: number;
    private _timer: NodeJS.Timer | undefined;

    constructor(fps: number) {
        this._frequency = 1e3 / fps;
        this.run();
    }

    private run(): void {
        this._timer = setInterval(() => {
            this.flush();
        }, this._frequency);
    }

    private flush(): void {
        while (this._renderQueue.length !== 0) {
            const callback = this._renderQueue.shift();
            if (typeof callback !== 'function') {
                console.warn('render callback is not a function, please check your parameter of draw()');
                continue;
            }
            callback();
        }
    }

    stop(): void {
        if (this._timer) {
            this._timer = undefined;
        }
    }

    addTask(callback: Function): void {
        this._renderQueue.push(callback);
    }
};

export const renderManager = new RenderManager(40);
