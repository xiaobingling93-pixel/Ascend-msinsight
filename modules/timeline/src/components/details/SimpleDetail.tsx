/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { observer } from 'mobx-react';
import React from 'react';
import type { ForwardedRef } from 'react';
import type { TableHandle } from '../base/rc-table/interface';
import type { CommonStateProto } from './base/Tabs';
import { useDetailUpdater, useMoreUpdater } from './hooks';
import type { DetailTabs } from './TabPanes';
import type { MoreTableProps, TableViewProps } from './types';
import { selectRow } from './utils';
import { ResizeTable } from 'ascend-resize';

function Support<T extends CommonStateProto>(
    { session, height, detail, isTree = true, tabState, commonState, onDataLoaded }: TableViewProps<DetailTabs, T>,
    ref: ForwardedRef<TableHandle>): JSX.Element {
    const state = useDetailUpdater(session, detail, tabState, [], onDataLoaded);
    const unit = session.selectedUnits[0];
    return <ResizeTable
        {...state}
        rowSelection={{
            selectedRowKeys: session.selectedDetailKeys,
        }}
        expandable={{ onExpand: state.onExpand, showExpandColumn: isTree }}
        onRow={(row): {onClick: () => void; onDoubleClick: () => void} => ({
            onClick: (): void => {
                selectRow(row, session, state);
                detail?.clickCallback?.({ row, session, detail, unit, commonState });
            },
            onDoubleClick: (): void => {
                selectRow(row, session, state);
                detail?.doubleClickCallback?.({ row, session, detail, unit, commonState });
            },
        })}
    />;
}
export const SimpleTabularDetail = React.forwardRef(Support) as
    <T extends CommonStateProto>(props: React.PropsWithChildren<TableViewProps<DetailTabs, T>> & { ref?: React.Ref<TableHandle> }) => JSX.Element;

export const SimpleTabularMore = observer(({ more, session, height, isTree = true }: MoreTableProps) => {
    const state = useMoreUpdater(session, more);
    return <ResizeTable {...state}
        expandable={{ onExpand: state.onExpand, showExpandColumn: isTree }}/>;
});
