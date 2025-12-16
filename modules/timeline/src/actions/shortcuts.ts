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
