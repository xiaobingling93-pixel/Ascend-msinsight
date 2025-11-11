/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { getShortcutKey } from '@insight/lib/utils';
import type { ActionName } from './types';

export type ShortcutName = Extract<
| ActionName,
| 'undoZoom'
| 'resetZoom'
| 'zoomIntoSelection'
>;

const shortcutMap: Record<ShortcutName, string[]> = {
    undoZoom: ['Backspace'],
    resetZoom: [getShortcutKey('CtrlOrCmd+0')],
    zoomIntoSelection: ['Shift+Z'],
};

export const getShortcutFromShortcutName = (name: ShortcutName, idx = 0): string => {
    const shortcuts = shortcutMap[name];
    return shortcuts !== undefined && shortcuts.length > 0
        ? shortcuts[idx] || shortcuts[0]
        : '';
};
