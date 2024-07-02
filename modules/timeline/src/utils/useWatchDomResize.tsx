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
