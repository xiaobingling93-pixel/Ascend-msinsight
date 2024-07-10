export class TimeoutPromise {
    private readonly delay: number;
    private readonly promise: Promise<unknown>;

    constructor(promise: Promise<unknown>, delay: number) {
        this.delay = delay;
        this.promise = promise;
    }

    private delayPromise(ms: number): Promise<never> {
        return new Promise(resolve => {
            setTimeout(resolve, ms);
        });
    }

    private timeoutPromise(promise: Promise<unknown>, delay: number, msg?: string): Promise<unknown> {
        const timeout = this.delayPromise(delay).then((): void => {
            throw new Error(msg ?? 'Operation timed out');
        });
        return Promise.race([promise, timeout]);
    }

    run(msg?: string): Promise<unknown> {
        return this.timeoutPromise(this.promise, this.delay, msg);
    }

    private abortPromise(promise: Promise<unknown>, ms: number, msg?: string): AbortPromise {
        const timeout = this.delayPromise(ms).then((): void => {
            throw new Error(msg ?? 'Operation timed out');
        });
        const abortP = {} as AbortPromise;
        const abortPromise = new Promise((resolve, reject) => {
            abortP.abort = reject;
        });
        abortP.promise = Promise.race([promise, abortPromise, timeout]);
        return abortP;
    }

    runAbort(msg?: string): AbortPromise {
        return this.abortPromise(this.promise, this.delay, msg);
    }
}

export interface AbortPromise {
    abort: (reason?: any) => void;
    promise: Promise<unknown>;
}
