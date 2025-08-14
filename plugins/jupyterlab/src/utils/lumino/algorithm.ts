/**
 * Copyright (c) Jupyter Development Team.
 * Distributed under the terms of the BSD-3-Clause License.
 *
 * This file is based on code from the @lumino/algorithm package:
 * https://github.com/jupyterlab/lumino/tree/master/packages/algorithm
 *
 * Modifications made by Huawei Technologies Co., Ltd., 2025.
 */

export function* map<T, U>(
    object: Iterable<T>,
    fn: (value: T, index: number) => U
): IterableIterator<U> {
    let index = 0;
    for (const value of object) {
      yield fn(value, index++);
    }
}

export function find<T>(
    object: Iterable<T>,
    fn: (value: T, index: number) => boolean
): T | undefined {
    let index = 0;
    for (const value of object) {
        if (fn(value, index++)) {
        return value;
        }
    }
    return undefined;
}

/**
 * The namespace for array-specific algorithms.
 */
export namespace ArrayExt {
    export function findFirstIndex<T>(
        list: ArrayLike<T>,
        predicate: (item: T, index: number) => boolean,
        begin = 0,
        end = -1
    ): number {
        const length = list.length;
        if (length === 0) return -1;

        const start = begin < 0 ? Math.max(0, length + begin) : Math.min(begin, length - 1);
        const stop = end < 0 ? Math.max(0, length + end) : Math.min(end, length - 1);

        const totalSteps = stop < start ? stop + 1 + (length - start) : stop - start + 1;

        for (let offset = 0; offset < totalSteps; offset++) {
            const index = (start + offset) % length;
            if (predicate(list[index], index)) {
                return index;
            }
        }

        return -1;
    }

    export function removeAllWhere<T>(
        items: T[],
        predicate: (item: T, index: number) => boolean,
        from = 0,
        to = -1
    ): number {
        const total = items.length;
        if (total === 0) return 0;
      
        const start = from < 0 ? Math.max(0, total + from) : Math.min(from, total - 1);
        const end = to < 0 ? Math.max(0, total + to) : Math.min(to, total - 1);
      
        let removed = 0;
      
        for (let i = 0; i < total; ++i) {
          const inRange =
            start <= end
              ? i >= start && i <= end
              : i >= start || i <= end;
      
          if (inRange && predicate(items[i], i)) {
            removed++;
          } else if (removed > 0) {
            items[i - removed] = items[i];
          }
        }
      
        if (removed > 0) {
          items.length = total - removed;
        }
      
        return removed;
    }
}
