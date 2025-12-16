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

import type { OnExpandAction } from './utils';
import { onExpandForChildren, parseColDef, treeAttachInfo } from './utils';
describe('utils test', () => {
    it('parseColDef test', () => {
        expect(() => parseColDef({ columns: [['name', (): string => 'render', 0]] }, session)).toThrow(new Error('columnsWidth at least one of the columns should have width "max-content" or "auto"'));
        expect(JSON.stringify(parseColDef({
            columns: [
                ['name2', (): string => 'render2', 'max-content', 'scroll'],
            ],
        }, session))).toEqual(JSON.stringify([
            {
                title: 'name2',
                key: 'name2',
                colSpan: 1,
                ellipsis: {
                    showTitle: true,
                },
                render: (): void => { },
            },
        ]));
    });

    it('treeAttachInfo test', () => {
        let root;
        expect(treeAttachInfo(root)).toStrictEqual(root);
        root = {};
        treeAttachInfo(root);
        expect(Object.getOwnPropertySymbols(root).length).toBe(2);
        root = { children: [{}] };
        treeAttachInfo(root);
        expect(Object.getOwnPropertySymbols(root).length).toBe(2);
        expect(Object.getOwnPropertySymbols(root.children[0]).length).toBe(2);
    });

    it('onExpandForChildren test', async () => {
        const setTableState = jest.fn();
        const node = {};
        const onExpand = jest.fn().mockResolvedValue(node);
        expect(onExpandForChildren(session, undefined, setTableState)).toBeUndefined();
        const fn = onExpandForChildren(session, onExpand, setTableState) as OnExpandAction;
        expect(fn(false, {})).toBeUndefined();
        await fn(true, node);
        expect(onExpand).toBeCalledTimes(1);
        expect(Object.getOwnPropertySymbols(node).length).toBe(2);
    });
});
