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
        const el = ref.current;

        if (!ResizeObserverImpl || !el) {
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

        resizeObserver.observe(el);
        return (): void => resizeObserver.disconnect();
    }, [ResizeObserverImpl]);

    return (
        <div ref={ref}>
            {children(size)}
        </div>
    );
};

export default Responsive;
