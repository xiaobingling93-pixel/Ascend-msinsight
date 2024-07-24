/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import type React from 'react';
import { useCallback, useEffect, useMemo, useRef, useState } from 'react';

const getSize = ({ visibleHeight, itemHeight, count }:
{visibleHeight: number;itemHeight: number;count: number}):
{
    visibleCount: number;
    totalHeight: number;
} => {
    if (itemHeight <= 0) {
        return { visibleCount: 0, totalHeight: visibleHeight };
    }
    const visibleCount = Math.ceil(visibleHeight / itemHeight) + 2;
    const totalHeight = itemHeight * count;
    return { visibleCount, totalHeight };
};
const setPostion = (e: any, { itemHeight, visibleCount, targetElement, setRange }:
{
    itemHeight: number;
    visibleCount: number;
    targetElement: HTMLElement | null;
    setRange: React.Dispatch<React.SetStateAction<[number, number]>>;
},
): void => {
    if (itemHeight <= 0 || targetElement === null || targetElement === undefined) {
        return;
    }
    const start = Math.floor(e.target.scrollTop / itemHeight);
    const end = start + visibleCount;
    setRange([start, end]);
    const offset = start * itemHeight;
    targetElement.style.top = `${offset}px`;
};

const initVirtual = ({ boxElement, targetElement, scrollEvent, totalHeight }:
{
    scrollEvent: (e: Event) => void;
    totalHeight: number;
    boxElement: HTMLElement | null;
    targetElement: HTMLElement | null;
},
) : (() => void) => {
    const isNullElement = boxElement === null || boxElement === undefined || targetElement === null || targetElement === undefined;
    if (isNullElement) {
        return (): void => {};
    }
    // 占位div
    const placeholderWrapper = document.createElement('div');
    placeholderWrapper.style.height = `${totalHeight}px`;
    boxElement.appendChild(placeholderWrapper);
    boxElement.style.position = 'relative';
    if (totalHeight === 0) {
        Object.assign(targetElement.style, { position: 'relative' });
    } else {
        Object.assign(targetElement.style, { position: 'absolute', top: '0', left: '0' });
    }
    // 添加滚动事件
    boxElement.addEventListener('scroll', scrollEvent);
    boxElement.scrollTo({ top: 0 });
    return (): void => {
        // 清理占位div
        boxElement.removeChild(placeholderWrapper);
        boxElement.removeEventListener('scroll', scrollEvent);
    };
};

export const useWatchVirtualRender = <T>({ visibleHeight, itemHeight, dataSource }: {
    visibleHeight: number;
    itemHeight: number;
    dataSource: T[];
},
): { data: T[];boxRef: React.MutableRefObject<null>;targetRef: React.MutableRefObject<null>} => {
    const boxRef = useRef(null);
    const targetRef = useRef(null);

    const [range, setRange] = useState<[number, number]>([0, 0]);
    const { visibleCount, totalHeight } = useMemo(() => getSize({ visibleHeight, itemHeight, count: dataSource.length }),
        [visibleHeight, itemHeight, dataSource.length]);
    const scrollEvent = useCallback((e: Event) => {
        setPostion(e, { itemHeight, visibleCount, setRange, targetElement: targetRef.current });
    }, [visibleCount, totalHeight, targetRef.current]);
    useEffect(() => {
        setRange([0, visibleCount]);
        return initVirtual({ scrollEvent, totalHeight, targetElement: targetRef.current, boxElement: boxRef.current });
    }, [scrollEvent, totalHeight]);
    const renderData = useMemo(() => dataSource.slice(...range), [dataSource, range]);
    return { data: renderData, boxRef, targetRef };
};
