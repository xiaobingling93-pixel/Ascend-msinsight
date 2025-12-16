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
export const getOperatingSystem = function (): string {
    const userAgent = navigator.userAgent.toLowerCase();

    if (userAgent.includes('windows')) {
        return 'Windows';
    } else if (userAgent.includes('macintosh') || userAgent.includes('mac os')) {
        return 'Mac OS';
    } else if (userAgent.includes('linux')) {
        return 'Linux';
    } else {
        return 'Unknown';
    }
};

export const isMac = getOperatingSystem() === 'Mac OS';

export const KEYS = {
    ARROW_DOWN: 'ArrowDown',
    ARROW_LEFT: 'ArrowLeft',
    ARROW_RIGHT: 'ArrowRight',
    ARROW_UP: 'ArrowUp',
    PAGE_UP: 'PageUp',
    PAGE_DOWN: 'PageDown',
    BACKSPACE: 'Backspace',
    SHIFT: 'Shift',
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
    F: 'f',
    G: 'g',
    K: 'k',
    L: 'l',
    M: 'm',
    Q: 'q',
    R: 'r',
    S: 's',
    W: 'w',
    Z: 'z',

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
