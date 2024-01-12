type Callback<T extends unknown[], K> = (...args: T) => Promise<K>;
export const customDebounce = function<T extends unknown[], K>(callback: Callback<T, K>): Callback<T, K> {
    let waitingTask: ReturnType<Callback<T, K>> | null = null;
    let curTask: Callback<T, K> | null = null;
    const taskQueue: Array<(value: K) => void> = [];
    return (...args) => {
        if (waitingTask === null) {
            waitingTask = callback(...args);
            return waitingTask;
        }
        return new Promise((resolve, reject) => {
            curTask = callback;
            taskQueue.push(resolve);
            waitingTask?.then((res) => {
                if (resolve !== taskQueue[taskQueue.length - 1]) {
                    resolve(res);
                    return;
                }
                waitingTask = curTask?.(...args) ?? null;
                waitingTask?.then(resolve);
            })
                .catch(e => reject(e));
        });
    };
};
