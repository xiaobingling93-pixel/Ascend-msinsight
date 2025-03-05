/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import { isMac } from './Common';

export const KEYS = {
    ARROW_DOWN: 'ArrowDown',
    ARROW_LEFT: 'ArrowLeft',
    ARROW_RIGHT: 'ArrowRight',
    ARROW_UP: 'ArrowUp',
    PAGE_UP: 'PageUp',
    PAGE_DOWN: 'PageDown',
    BACKSPACE: 'Backspace',
    ALT: 'Alt',
    CTRL_OR_CMD: isMac ? 'metaKey' : 'ctrlKey',
    DELETE: 'Delete',
    ENTER: 'Enter',
    ESCAPE: 'Escape',
    QUESTION_MARK: '?',
    SPACE: ' ',
    TAB: 'Tab',
    CHEVRON_LEFT: '<',
    CHEVRON_RIGHT: '>',
    PERIOD: '.',
    COMMA: ',',
    SUBTRACT: '-',
    SLASH: '/',

    A: 'a',
    D: 'd',
    G: 'g',
    L: 'l',
    M: 'M',
    Q: 'q',
    R: 'r',
    S: 's',
    W: 'w',

    0: '0',
    1: '1',
    2: '2',
    3: '3',
    4: '4',
    5: '5',
    6: '6',
    7: '7',
    8: '8',
    9: '9',
} as const;

export const getShortcutKey = (shortcut: string): string => {
    const key = shortcut
        .replace(/\bAlt\b/i, 'Alt')
        .replace(/\bShift\b/i, 'Shift')
        .replace(/\b(?<key>Enter|Return)\b/i, 'Enter');
    if (isMac) {
        return key
            .replace(/\bCtrlOrCmd\b/gi, 'Cmd')
            .replace(/\bAlt\b/i, 'Option');
    }
    return key.replace(/\bCtrlOrCmd\b/gi, 'Ctrl');
};
