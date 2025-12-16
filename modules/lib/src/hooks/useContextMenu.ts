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
