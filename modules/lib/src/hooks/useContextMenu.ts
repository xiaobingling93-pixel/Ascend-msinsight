/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { useEffect, useState } from 'react';

export function useContextMenu(container: HTMLElement | null): {
    visible: boolean;
    coords: {x: number; y: number};
    coordsOffset: {x: number; y: number};
    setVisible: (visible: boolean) => void;
} {
    const [visible, setVisible] = useState(false);
    const [coords, setCoords] = useState({ x: 0, y: 0 });
    const [coordsOffset, setCoordsOffset] = useState({ x: 0, y: 0 });

    useEffect(() => {
        if (!container) return;

        const open = (e: MouseEvent): void => {
            e.preventDefault();
            e.stopPropagation();
            setCoords({ x: e.clientX, y: e.clientY });
            setCoordsOffset({ x: e.offsetX, y: e.offsetY });
            setVisible(true);
        };

        const close = (): void => {
            setVisible(false);
        };

        container.addEventListener('contextmenu', open);
        window.addEventListener('mousedown', close);
        window.addEventListener('wheel', close);
        window.addEventListener('blur', close);

        return () => {
            container.removeEventListener('contextmenu', open);
            window.removeEventListener('mousedown', close);
            window.removeEventListener('wheel', close);
            window.removeEventListener('blur', close);
        };
    }, [container]);

    return { visible, coords, coordsOffset, setVisible };
}
