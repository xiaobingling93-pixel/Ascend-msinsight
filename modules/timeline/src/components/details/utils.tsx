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
import type { TreeNode } from '../../entity/common';
import { SorterDef, SummaryFunction, TableDataAdapter } from '../../entity/insight';
import type { Session } from '../../entity/session';
import type { TabState } from '../../entity/tabDependency';
import { getAutoKey } from '../../utils/dataAutoKey';
import { getOrigin } from '../../utils/traceOrigin';
import type { ColumnType } from 'antd/es/table';
import type { FixedType } from '../base/rc-table/types';
import { Abbreviature } from './base/Abbreviature';
import { parseFilterDef } from './tableFilter';
import type { TableState } from './types';

const parseSorterDef = <T extends Record<string, unknown>>(sorterDef?: SorterDef<T>): Record<string, unknown> => {
    if (sorterDef?.sorter) {
        return { sorter: sorterDef.sorter };
    }
    return {};
};

export const parseColDef = <T extends Record<string, unknown> = Record<string, unknown>>(
    def: TableDataAdapter<T>,
    session: Session,
    tabState?: TabState | undefined,
): Array<ColumnType<T> & { summary?: SummaryFunction<T> }> => {
    if (def.columns.every(col => typeof (col[2]) === 'number')) {
        throw new Error('columnsWidth at least one of the columns should have width "max-content" or "auto"');
    }
    const cols = [] as Array<ColumnType<T> & { summary?: SummaryFunction<T> }>;
    def.columns.forEach((col, index) => {
        if (col[4]?.(session)) {
            return;
        }
        const actionDef = def.actions?.[index];
        const partialCol = {
            title: col[0],
            key: col[0],
            colSpan: 1,
            ellipsis: { showTitle: true },
            ...parseFilterDef(actionDef?.filterKey),
            ...parseSorterDef(actionDef),
            summary: def.summaries?.get(col[0]),
        };
        if (col[3] !== 'scroll') {
            const render = col[1];
            cols.push({
                ...partialCol,
                width: col[2],
                fixed: col[3] as (FixedType | undefined),
                render: (_, item: object) => <Abbreviature content={render(item as T, session, tabState)} />,
            });
        } else {
            cols.push({
                ...partialCol,
                render: (_, item: object) => col[1](item as T, session, tabState),
            });
        }
    });
    return cols;
};

export type OnExpandAction = (expanded: boolean, r: Record<string, unknown>) => void;

type OnExpandConfig<T> = ((session: Session, data: TreeNode<Record<string, T>>) => Promise<TreeNode<Record<string, T>> | undefined>);

export function treeAttachInfo<T>(root?: TreeNode<T>): TreeNode<T> | undefined {
    if (!root) { return root; }
    getAutoKey(root);
    getOrigin(root);
    root.children?.forEach(treeAttachInfo);
    return root;
};

export const onExpandForChildren = (session: Session, onExpand: OnExpandConfig<any> | undefined,
    setTableState: React.Dispatch<React.SetStateAction<TableState>>): OnExpandAction | undefined => {
    if (!onExpand) {
        return undefined;
    }
    return (expanded: boolean, r: TreeNode<Record<string, unknown>>): void => {
        if (!expanded || onExpand === undefined) {
            return;
        }
        onExpand(session, getOrigin(r)).then(treeAttachInfo).then(tree => {
            // do nothing if data is not updated
            if (!tree) { return; }
            // data is updated in place, which won't trigger ui refreshing, so force refresh the ui
            setTableState(state => ({ ...state, data: [...state.dataSource] }));
        });
    };
};

interface RenderRadiusBorderParams {
    topLeft: number;
    topRight: number;
    bottomRight: number;
    bottomLeft: number;
    depth: number;
    ctx: CanvasRenderingContext2D;
}

export const renderRadiusBorder = ({ topLeft, topRight, bottomRight, bottomLeft, depth, ctx }: RenderRadiusBorderParams): void => {
    const halfLine = 1;
    ctx.lineWidth = halfLine * 3;
    const height = Math.floor(bottomLeft - halfLine - 1);
    const beginX = topLeft;
    const beginY = topRight + (((halfLine * 2) + height) * depth);
    ctx.strokeRect(beginX, beginY, bottomRight, Math.floor(bottomLeft - halfLine - 1));
};
