import React, { useEffect } from 'react';
import { runInAction } from 'mobx';
import { observer } from 'mobx-react';
import { MoreTableProps, TableViewProps } from './types';
import { useDetailUpdater, useMoreUpdater } from './hooks';
import { selectRow } from './utils';
import { AutoAdjustedTable } from './base/AutoAdjustedTable';
import { CommonStateProto, TabProto } from './base/Tabs';

export const SelectSimpleTabularDetail = observer(<T extends CommonStateProto>(
    { session, height, detail, tabState, commonState, depsList }: TableViewProps<TabProto, T>) => {
    useEffect(() => {
        runInAction(() => {
            session.selectedDetailKeys = [];
            session.selectedDetails = [];
            session.selectedMultiSlice = '';
        });
    }, [session.selectedRange]);
    const state = useDetailUpdater(session, detail, tabState, depsList);
    const unit = session.selectedUnits[0];
    return <AutoAdjustedTable {...state} height={height}
        rowSelection={{
            selectedRowKeys: session.selectedDetailKeys,
        }}
        rowClassName={'click-able'}
        expandable={{ showExpandColumn: false }}
        onRow={(row): React.HTMLAttributes<any> => ({
            onClick: async (): Promise<void> => {
                detail?.clickCallback?.({ row, session, detail, unit, commonState });
                selectRow(row, session, state);
                if (detail?.fetchMoreData !== undefined && detail.more?.field !== undefined) {
                    row[detail.more?.field] = await detail?.fetchMoreData(session, row).catch(() => []);
                }
            },
            onDoubleClick: (): void => {
                selectRow(row, session, state);
                detail?.doubleClickCallback?.({ row, session, detail, unit, commonState });
            },
            onMouseEnter: (): void => {
                detail?.mouseEnterCallback?.({ session, row });
            },
            onMouseLeave: (): void => {
                detail?.mouseLeaveCallback?.({ session, row });
            },
        })}
    />;
});

export const SelectSimpleTabularMore = observer(({ more, height, session }: MoreTableProps) => {
    const state = useMoreUpdater(session, more);
    return <AutoAdjustedTable {...state} height={height}
        expandable={{ showExpandColumn: false }}/>;
});
