/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { observer } from 'mobx-react';
import React, { useEffect, useMemo, useRef, useState } from 'react';
import type { TreeNode } from '../../entity/common';
import type { DetailDescriptor, TableDataAdapter } from '../../entity/insight';
import type { Session } from '../../entity/session';
import { getAutoKey } from '../../utils/dataAutoKey';
import type { TableHandle } from '../base/rc-table/interface';
import type { CommonStateProto } from './base/Tabs';
import { SimpleTabularDetail } from './SimpleDetail';
import type { DetailTabs } from './TabPanes';
import { EMPTY_TABLE_STATE, type TableState, type TableViewProps } from './types';
import { parseColDef } from './utils';
import { ResizeTable } from 'ascend-resize';

const PARENT = Symbol('parent');

interface Traceable<T> { [PARENT]?: Traceable<T>; ref: React.RefObject<TableHandle> };

export type Comparator<T> = (l: T, r: T) => boolean;

const updateHierarchyInfo = <T extends Record<string, unknown>>(parent: TreeNode<T>, ref: Traceable<T>['ref']): TreeNode<T> => {
    (parent as unknown as Traceable<T>).ref = ref;
    if (!parent.children) {
        return parent;
    }
    parent.children.forEach(it => {
        (it as unknown as Traceable<T>)[PARENT] = parent as unknown as Traceable<T>;
    });
    // maybe update tree path as well
    return parent;
};

export const CallTree = observer(<T extends CommonStateProto>(props: TableViewProps<DetailTabs, T> & { detail: DetailDescriptor<unknown> }): JSX.Element => {
    // Used to link More and Details
    const ref = useRef<TableHandle>(null);
    let detail = props.detail;
    const onExpand = detail.onExpand;
    const expandWithUpdate = useMemo(() => {
        return (session: Session, r: TreeNode<Record<string, unknown>>): Promise<TreeNode<Record<string, unknown>> | undefined> => {
            const data = onExpand?.(session, r);
            if (data) {
                return data.then(it => it && updateHierarchyInfo(it, ref)).then(() => { return undefined; });
            }
            updateHierarchyInfo(r, ref);
            return Promise.resolve(undefined);
        };
    }, [onExpand]);
    // Should refactor to parent component
    const clickCallback = detail.clickCallback;
    const clickCallbackWithUpdate = useMemo(() => {
        return (args: Parameters<Exclude<DetailDescriptor<unknown>['clickCallback'], undefined>>[0]): void => {
            clickCallback?.(args);
            const { row } = args;
            updateHierarchyInfo(row, ref);
        };
    }, [clickCallback]);
    detail = useMemo(() => {
        return { ...detail, onExpand: expandWithUpdate, clickCallback: clickCallbackWithUpdate };
    }, [detail, expandWithUpdate, clickCallbackWithUpdate]);
    const onDataLoaded = (data: unknown[]): void => {
        if (ref.current === null || data.length === 0) {
            return;
        }
        setTimeout(() => ref.current?.selectFirstRoot(), 0);
    };
    return <SimpleTabularDetail {...props} detail={detail} onDataLoaded={onDataLoaded} ref={ref}/>;
});

const computeStack = <T extends Record<string, unknown>>(compare: Comparator<T>) => {
    // precondition: the path from current node to the root can be found by tracing the <parent> field
    return (node: TreeNode<T>): Array<TreeNode<T>> => {
        let stack: Array<TreeNode<T>> = [];

        // trace backward to the root
        let current: TreeNode<T> | undefined = node;
        do {
            stack.push(current);
            current = (current as unknown as Traceable<T>)[PARENT] as (TreeNode<T> | undefined);
        } while (current);
        stack = stack.reverse();

        // trace forward to the leaves
        let child = node;
        const maxDepth = 1000000;
        let i = 0;
        while (child.children && child.children.length > 0 && i < maxDepth) {
            i++;
            const target = child.children.reduce((prev, cur) => compare(prev, cur) ? prev : cur, child.children[0]);
            // add ref to every child
            (target as unknown as Traceable<T>).ref = (stack[0] as unknown as Traceable<T>).ref;
            stack.push(target);
            child = target;
        }
        return stack;
    };
};

const useStackUpdater = <T extends Record<string, unknown>>(session: Session, compare: Comparator<T>, adapter: TableDataAdapter<T>): TableState => {
    const [state, setState] = useState(EMPTY_TABLE_STATE);
    const { selectedUnits, selectedDetailKeys, selectedDetails } = session;
    const compute = computeStack(compare);
    useEffect(() => {
        const selectedDetail = selectedDetails.length > 0 ? selectedDetails[0] : undefined;
        setState({ ...EMPTY_TABLE_STATE, loading: true });
        if (selectedDetail) {
            setState({
                dataSource: compute(selectedDetail as TreeNode<T>),
                columns: parseColDef(adapter, session),
                loading: false,
            });
        } else {
            setState(EMPTY_TABLE_STATE);
        }
    }, [selectedUnits, selectedDetailKeys]);
    return state;
};

export type CallStackViewProps<T extends Record<string, unknown>> = {
    session: Session;
    height: number;
    compare: Comparator<T>; // used for extracting heaviest stack
    onNavigate?: ({ row }: { row: Record<string, unknown> }) => void;
} & TableDataAdapter<T>;

export const CallStackView = observer(<T extends Record<string, unknown>>(
    { session, height, compare, columns, onNavigate }: CallStackViewProps<T>): JSX.Element => {
    const state = useStackUpdater(session, compare, { columns });
    return <ResizeTable {...state} expandable={{ showExpandColumn: false }}
        onRow={(row): {onClick: () => void; onDoubleClick: () => void } => {
            const onDoubleClick = (): void => onNavigate?.({ row });
            const onClick = (): void => {
                session.selectedDetailKeys = [getAutoKey(row)];
                // need combine two functions
                ((state.dataSource[0] as unknown as Traceable<T>).ref)?.current?.appendExpandedKeys(state.dataSource.map(getAutoKey));
                setTimeout(() => {
                    ((state.dataSource[0] as unknown as Traceable<T>).ref)?.current?.scrollTo(row);
                }, 0);
            };
            return { onClick, onDoubleClick };
        }}
        rowSelection={{
            selectedRowKeys: session.selectedDetailKeys,
        }}/>;
});
