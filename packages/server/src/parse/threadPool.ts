import { Worker, isMainThread, parentPort } from 'worker_threads';
import { parseWorkerOnMessage } from './parser_worker';

if (!isMainThread) {
    parentPort?.on('message', async (msg: any) => await parseWorkerOnMessage(msg));
}

export enum WorkMessageType {
    PARSE = 0,
    EXIT,
}

export enum WorkStatus {
    RUNNING = 0,
    END,
}

export class ThreadPool {
    private readonly taskCount = 4;
    private readonly runningWorkers = new Array<Worker>();
    private readonly awaitWorkers = new Array<Worker>();
    private readonly taskList = new Array<any>();
    private taskFinishCallback: Function | undefined;
    private allTaskFinishCallback: Function | undefined;

    constructor(taskFinishCallback?: Function, allTaskFinishCallback?: Function) {
        for (let i = 0; i < this.taskCount; ++i) {
            this.awaitWorkers.push(this.createWork(i));
        }
        this.taskFinishCallback = taskFinishCallback;
        this.allTaskFinishCallback = allTaskFinishCallback;
    }

    createWork(n: number): Worker {
        const work = new Worker(__filename);
        // const work: Worker = new Worker(workPath);
        // On worker online
        work.on('online', () => {
            console.log(`[ThreadPool] worker ${n} is online and executing code!`);
        });
        work.on('message', (msg) => {
            console.log(`[ThreadPool] worker ${n} is done! task count:${this.taskList.length}`);
            if (this.taskFinishCallback) {
                this.taskFinishCallback(msg.data);
            }
            if (this.taskList.length > 0) {
                const data = this.taskList.shift();
                work.postMessage({ command: WorkMessageType.PARSE, data });
            } else {
                const i = this.runningWorkers.indexOf(work);
                this.runningWorkers.splice(i, 1);
                this.awaitWorkers.push(work);
                console.log(`[ThreadPool] tasks. running:${this.runningWorkers.length}, await:${this.awaitWorkers.length}`);
                if (this.runningWorkers.length === 0 && this.allTaskFinishCallback) {
                    this.allTaskFinishCallback();
                }
            }
        });
        // On worker exit
        work.on('exit', (code) => {
            console.log(`[ThreadPool] Worker ${n} execution is over with code: ${code}`);
        });
        // On worker error
        work.on('error', (error) => {
            // Sometime stack is undefined, we can stringify error instead
            console.log(`[ThreadPool] Worker ${n} catch an error: ${error.stack ?? JSON.stringify(error)}`);
        });
        return work;
    }

    public addTask(data: any): void {
        if (this.awaitWorkers.length > 0) {
            const task = this.awaitWorkers.pop() as Worker;
            task.postMessage({ command: WorkMessageType.PARSE, data });
            this.runningWorkers.push(task);
        } else {
            this.taskList.push(data);
        }
    }

    public setTaskFinishCallback(callback?: Function): void {
        this.taskFinishCallback = callback;
    }

    public setAllTaskFinishCallback(callback?: Function): void {
        this.allTaskFinishCallback = callback;
    }
}
