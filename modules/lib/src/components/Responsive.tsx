/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { useState, useEffect, useRef, ReactNode } from 'react';

interface Size {
    width: number; height: number;
}
interface ResponsiveProps {
    children: (size: Size) => ReactNode;
    onChange?: (size: Size) => void;
}

export const Responsive: React.FC<ResponsiveProps> = ({ children, onChange }) => {
    const [ResizeObserverImpl, setResizeObserverImpl] = useState<typeof ResizeObserver | null>(null);
    const [size, setSize] = useState({ width: 0, height: 0 });
    const ref = useRef<HTMLDivElement | null>(null);

    // resize-observer-polyfill
    useEffect(() => {
        async function loadPolyfill(): Promise<void> {
            if (typeof window !== 'undefined' && typeof window.ResizeObserver === 'undefined') {
                // 动态导入 polyfill
                const module = await import('resize-observer-polyfill');
                setResizeObserverImpl(() => module.default);
            } else {
                setResizeObserverImpl(() => window.ResizeObserver);
            }
        }
        loadPolyfill();
    }, []);

    useEffect(() => {
        if (!ResizeObserverImpl) {
            return () => {};
        }

        const resizeObserver = new ResizeObserverImpl(([entry]) => {
            if (entry.target instanceof HTMLElement) {
                const newSize = {
                    width: entry.contentRect.width,
                    height: entry.contentRect.height,
                };
                setSize(newSize);

                if (onChange) {
                    onChange(newSize);
                }
            }
        });

        const currentElement = ref.current;
        if (currentElement) {
            resizeObserver.observe(currentElement);
        }

        return (): void => {
            if (currentElement) {
                resizeObserver.disconnect();
            }
        };
    }, [ResizeObserverImpl]);

    return (
        <div ref={ref}>
            {children(size)}
        </div>
    );
};

export default Responsive;
