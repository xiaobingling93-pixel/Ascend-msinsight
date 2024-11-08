/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { useState, useEffect, useCallback } from 'react';
import { debounce } from 'lodash';

/**
 * 界面尺寸改变
 * @returns {readonly [number]}
 */
export function useResizeEventDependency(): readonly [number] {
    const [version, setVersion] = useState(0);

    const increaseSize = useCallback(
        debounce(() => {
            setVersion((prev) => prev + 1);
        }, 100),
        [],
    );

    useEffect(() => {
        window.addEventListener('resize', increaseSize);

        return () => {
            window.removeEventListener('resize', increaseSize);
        };
    }, []);

    return [version] as const;
};

/**
 * 二分搜索
 * @param arr 搜索的数组
 * @param key 搜索的key值
 * @param compareFun 比较函数
 * @returns {number} 要搜索值的索引
 */
export function binarySearch(arr: any[][], key: number, compareFun: (key: number, mid: Array<number | string>) => number): number {
    let low = 0;
    let high = arr.length - 1;
    while (low <= high) {
        const mid = Math.round((high + low) / 2);
        const cmp = compareFun(key, arr[mid]);
        if (cmp > 0) {
            low = mid + 1;
        } else if (cmp < 0) {
            high = mid - 1;
        } else {
            // 可能存在多个arr[mid][0]等于key，只有arr[mid][1]（Allocated项）不为null时才是真正要找的项
            if (arr[mid][1] !== null) {
                return mid;
            }

            let step = 1;
            while (true) {
                const isLowerOk = mid - step >= 0 && Number(arr[mid - step][0]) === key;
                const isHigherOk = mid + step <= arr.length - 1 && Number(arr[mid + step][0]) === key;
                if (isLowerOk && arr[mid - step][1] !== null) {
                    return mid - step;
                }
                if (isHigherOk && arr[mid + step][1] !== null) {
                    return mid + step;
                }
                if (!isLowerOk && !isHigherOk) {
                    break;
                }
                step += 1;
            }
            return -1;
        }
    }
    return -1;
}
