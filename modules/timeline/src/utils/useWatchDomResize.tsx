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
import * as React from 'react';
import ResizeObserver from 'resize-observer-polyfill';

export function useWatchDomResize<T extends Element>(): [
    DOMRectReadOnly | null,
    React.RefObject<T>,
] {
    const [rect, setRect] = React.useState<DOMRectReadOnly | null>(null);
    const ref = React.useRef<T>(null);
    React.useEffect(() => {
        const observer = new ResizeObserver(([entry]) => {
            window.requestAnimationFrame(() => {
                setRect(entry.contentRect);
            });
        });
        if (ref.current) {
            observer.observe(ref.current);
        }
        return () => {
            observer.disconnect();
        };
    }, []);
    return [rect, ref];
};

export function useWatchResize<T extends Element>(param: 'height' | 'width'): [ number, React.RefObject<T> ] {
    const [rect, ref] = useWatchDomResize<T>();
    const [size, setSize] = React.useState(0);
    React.useEffect(() => {
        if (rect?.[param] !== undefined) {
            setSize(rect[param]);
        }
    }, [rect]);
    return [size, ref];
};
