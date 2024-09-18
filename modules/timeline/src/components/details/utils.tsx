/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { runInAction } from 'mobx';
import * as React from 'react';
import type { TreeNode } from '../../entity/common';
import type { SorterDef, TableDataAdapter } from '../../entity/insight';
import type { Session } from '../../entity/session';
import type { TabState } from '../../entity/tabDependency';
import { type AutoKey, getAutoKey } from '../../utils/dataAutoKey';
import { getOrigin } from '../../utils/traceOrigin';
import type { ColumnsType } from 'antd/es/table';
import type { FixedType } from '../base/rc-table/types';
import { Abbreviature } from './base/Abbreviature';
import { parseFilterDef } from './tableFilter';
import type { TableState } from './types';
import { drawRoundedRect } from '../charts/common';

const parseSorterDef = <T extends Record<string, unknown>>(sorterDef?: SorterDef<T>): Record<string, unknown> => {
    if (sorterDef?.sorter) {
        return { sorter: sorterDef.sorter };
    }
    return {};
};

export const parseColDef = <T extends Record<string, unknown>>(
    def: TableDataAdapter<T>,
    session: Session,
    tabState?: TabState | undefined,
): ColumnsType<Record<string, unknown>> => {
    if (def.columns.every(col => typeof (col[2]) === 'number')) {
        throw new Error('columnsWidth at least one of the columns should have width "max-content" or "auto"');
    }
    const cols = [] as ColumnsType<Record<string, unknown>>;
    def.columns.forEach((col, index) => {
        const actionDef = def.actions?.[index];
        const partialCol = {
            title: col[0],
            key: col[0],
            colSpan: 1,
            ellipsis: { showTitle: true },
            ...parseFilterDef(actionDef?.filterKey),
            ...parseSorterDef(actionDef),
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

export const selectRow = (row: AutoKey<Record<string, unknown>>, session: Session, state: TableState): void => runInAction(() => {
    session.selectedDetailKeys = [state.rowKey?.(row) ?? getAutoKey(row)];
    session.selectedDetails = [row];
});

export type OnExpandAction = (expanded: boolean, r: Record<string, unknown>) => void;

type OnExpandConfig = ((session: Session, data: TreeNode<Record<string, unknown>>) => Promise<TreeNode<Record<string, unknown>> | undefined>);

export function treeAttachInfo<T>(root?: TreeNode<T>): TreeNode<T> | undefined {
    if (!root) { return root; }
    getAutoKey(root);
    getOrigin(root);
    root.children?.forEach(treeAttachInfo);
    return root;
};

export const onExpandForChildren = (session: Session, onExpand: OnExpandConfig | undefined,
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
            // force refresh more panel in case data is updated in place
            if (session.selectedDetailKeys.length > 0) {
                runInAction(() => { session.selectedDetailKeys = [...session.selectedDetailKeys]; });
            }
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
    ctx.lineWidth = halfLine * 2;
    let width = bottomRight;
    width = Math.max(1, Math.floor(width));
    const height = Math.floor(bottomLeft - halfLine - 1);
    const beginX = topLeft;
    const beginY = topRight + (((halfLine * 2) + height) * depth);
    const radius = width >= 2 ? 1 : width / 2;
    if (radius < 1) {
        ctx.strokeRect(beginX, beginY, bottomRight, Math.floor(bottomLeft - halfLine - 1));
    } else {
        drawRoundedRect([Math.floor(beginX), Math.floor(beginY), Math.floor(bottomRight), Math.floor(bottomLeft - halfLine - 1)], ctx, radius);
        ctx.stroke();
    }
};
