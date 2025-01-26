/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
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
