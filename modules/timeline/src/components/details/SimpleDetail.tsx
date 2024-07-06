import { observer } from 'mobx-react';
import React, { ForwardedRef } from 'react';
import { TableHandle } from '../base/rc-table/interface';
import { AutoAdjustedTable } from './base/AutoAdjustedTable';
import { CommonStateProto } from './base/Tabs';
import { useDetailUpdater, useMoreUpdater } from './hooks';
import { DetailTabs } from './TabPanes';
import { MoreTableProps, TableViewProps } from './types';
import { selectRow } from './utils';

function Support<T extends CommonStateProto>(
    { session, height, detail, isTree = true, tabState, commonState, onDataLoaded }: TableViewProps<DetailTabs, T>,
    ref: ForwardedRef<TableHandle>): JSX.Element {
    const state = useDetailUpdater(session, detail, tabState, [], onDataLoaded);
    const unit = session.selectedUnits[0];
    return <AutoAdjustedTable
        {...state}
        height={height}
        ref={ref}
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
    return <AutoAdjustedTable {...state} height={height}
        expandable={{ onExpand: state.onExpand, showExpandColumn: isTree }}/>;
});
