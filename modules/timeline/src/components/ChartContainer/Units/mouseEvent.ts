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

enum MouseButton {
    LEFT = 0,
    MIDDLE = 1,
    RIGHT = 2,
}

type ModifierKey = 'shift' | 'ctrlOrCommand' | 'none';

type MouseEventHandler = (event: React.MouseEvent<HTMLElement, MouseEvent>) => void;

interface ComplexMouseEventHandlerMap {
    shiftLeft?: MouseEventHandler;
    shiftRight?: MouseEventHandler;
    ctrlLeft?: MouseEventHandler;
    ctrlRight?: MouseEventHandler;
    left: MouseEventHandler;
    right: MouseEventHandler;
    stopPropagation?: boolean;
}

const DEFAULT_HANDLER: MouseEventHandler = (): void => {};

export function useComplexMouseEvent(eventMap: ComplexMouseEventHandlerMap):
(e: React.MouseEvent<HTMLElement, MouseEvent>) => void {
    const handleMouseEvent = React.useCallback((event: React.MouseEvent<HTMLElement, MouseEvent>,
        button: MouseButton, modifier: ModifierKey) => {
        const modifierMapping: {
            [key in ModifierKey]: MouseEventHandler[];
        } = {
            shift: [eventMap.shiftLeft ?? eventMap.left, DEFAULT_HANDLER, eventMap.shiftRight ?? eventMap.right],
            ctrlOrCommand: [eventMap.ctrlLeft ?? eventMap.left, DEFAULT_HANDLER, eventMap.ctrlRight ?? eventMap.right],
            none: [eventMap.left, DEFAULT_HANDLER, eventMap.right],
        };
        const handler = modifierMapping[modifier][button];
        if (typeof handler === 'function') {
            handler(event);
        }
    }, [eventMap]);

    return React.useCallback((e: React.MouseEvent<HTMLElement, MouseEvent>): void => {
        if (Object.is(eventMap.stopPropagation, true)) {
            e.stopPropagation();
        }
        const button = e.button;
        const isOnlyShift = e.shiftKey && !e.altKey && !e.ctrlKey && !e.metaKey;
        const isOnlyCtrlOrCommand = !e.shiftKey && !e.altKey && (e.ctrlKey || e.metaKey);
        if (isOnlyShift) {
            // shift + 鼠标按键
            handleMouseEvent(e, button, 'shift');
        } else if (isOnlyCtrlOrCommand) {
            // ctrl + 鼠标按键
            handleMouseEvent(e, button, 'ctrlOrCommand');
        } else {
            // 普通鼠标按键
            handleMouseEvent(e, button, 'none');
        }
    }, [eventMap, handleMouseEvent]);
}
