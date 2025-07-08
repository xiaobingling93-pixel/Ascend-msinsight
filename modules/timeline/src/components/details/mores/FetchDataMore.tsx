/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import * as React from 'react';
import { observer } from 'mobx-react';
import type { Session } from '../../../entity/session';
import type { InsightUnit, TableDataAdapter } from '../../../entity/insight';
import { parseColDef } from '../utils';
import { type TableState, EMPTY_TABLE_STATE } from '../types';
import { ResizeTable } from 'ascend-resize';

/**
 * This Component is independent fetch data More Component,
 * but its params are from the selected detail row.
 */
const useMoreUpdater = function<T extends Record<string, unknown>>(session: Session, fetchData: FetchDataMoreProps<T>['fetchData'], def: TableDataAdapter<T>): TableState<T> {
    const [state, setState] = React.useState<TableState<T>>(EMPTY_TABLE_STATE);
    const { selectedUnits, selectedDetailKeys, selectedDetails, selectedRange } = session;
    const recentUnits = React.useRef(selectedUnits);
    const recentRange = React.useRef(selectedRange);
    recentUnits.current = selectedUnits;
    recentRange.current = selectedRange;

    React.useEffect(() => {
        const selectedDetail = selectedDetails.length > 0 ? selectedDetails[0] : undefined;
        if (selectedDetail !== undefined && selectedRange !== undefined && session.phase === 'download') {
            setState({ ...EMPTY_TABLE_STATE, loading: true });
            fetchData(session, selectedDetail).then(result => {
                if (recentUnits.current !== selectedUnits || recentRange.current !== selectedRange) {
                    return;
                }
                setState({
                    dataSource: result,
                    columns: parseColDef<T>(def, session),
                    loading: false,
                });
            }).catch(() => {
                setState({ ...EMPTY_TABLE_STATE, loading: false });
            });
        } else {
            setState({ ...EMPTY_TABLE_STATE, loading: false });
        }
    }, [selectedUnits, selectedDetailKeys]);
    return state;
};

export type FetchDataMoreProps<T extends Record<string, unknown>> = {
    session: Session;
    isTree?: boolean;
    fetchData: (...args: unknown[]) => Promise<T[]>;
    onExpand: (...args: unknown[]) => void;
    clickCallback?: ({ row, session, unit }: { row: Record<string, unknown>; session: Session; unit?: InsightUnit }) => void;
    doubleClickCallback?: ({ row, session, unit }: { row: Record<string, unknown>; session: Session; unit?: InsightUnit }) => void;
} & TableDataAdapter<T>;
export const FetchDataMore = observer(<T extends Record<string, unknown>>({
    session,
    isTree = false,
    fetchData,
    columns,
    actions,
    clickCallback,
    doubleClickCallback,
}: FetchDataMoreProps<T>): JSX.Element => {
    const state = useMoreUpdater(session, fetchData, { columns, actions });
    return <ResizeTable {...state}
        expandable={{ onExpand: state.onExpand, showExpandColumn: isTree }}
        onRow={(row): {onClick: () => void; onDoubleClick: () => void} => ({
            onClick: (): void => {
                clickCallback?.({ row, session, unit: session.selectedUnits[0] });
            },
            onDoubleClick: (): void => {
                doubleClickCallback?.({ row, session, unit: session.selectedUnits[0] });
            },
        })}
    />;
});
