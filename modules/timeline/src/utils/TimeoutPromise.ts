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
export class TimeoutPromise {
    private readonly delay: number;
    private readonly promise: Promise<unknown>;

    constructor(promise: Promise<unknown>, delay: number) {
        this.delay = delay;
        this.promise = promise;
    }

    run(msg?: string): Promise<unknown> {
        return this.timeoutPromise(this.promise, this.delay, msg);
    }

    runAbort(msg?: string): AbortPromise {
        return this.abortPromise(this.promise, this.delay, msg);
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
}

export interface AbortPromise {
    abort: (reason?: any) => void;
    promise: Promise<unknown>;
}
