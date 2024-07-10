import * as React from 'react';
import { observer } from 'mobx-react';
import { AutoAdjustedTable } from '../base/AutoAdjustedTable';
import { Session } from '../../../entity/session';
import { InsightUnit, TableDataAdapter } from '../../../entity/insight';
import { parseColDef } from '../utils';
import { TableState, EMPTY_TABLE_STATE } from '../types';

/**
 * This Component is independent fetch data More Component,
 * but its params are from the selected detail row.
 */
const useMoreUpdater = function<T extends Record<string, unknown>>(session: Session, fetchData: FetchDataMoreProps<T>['fetchData'], def: TableDataAdapter<T>): TableState {
    const [state, setState] = React.useState<TableState>(EMPTY_TABLE_STATE);
    const { selectedUnits, selectedDetailKeys, selectedDetails, selectedRange } = session;
    const recentUnits = React.useRef(selectedUnits);
    const recentRange = React.useRef(selectedRange);
    recentUnits.current = selectedUnits;
    recentRange.current = selectedRange;

    React.useEffect(() => {
        const selectedDetail = selectedDetails.length > 0 ? selectedDetails[0] : undefined;
        if (selectedDetail !== undefined && selectedRange !== undefined && session.phase === 'download') {
            setState({ ...EMPTY_TABLE_STATE, isLoading: true });
            fetchData(session, selectedDetail).then(result => {
                if (recentUnits.current !== selectedUnits || recentRange.current !== selectedRange) {
                    return;
                }
                setState({
                    data: result,
                    columns: parseColDef(def, session),
                    isLoading: false,
                });
            }).catch(() => {
                setState({ ...EMPTY_TABLE_STATE, isLoading: false });
            });
        } else {
            setState({ ...EMPTY_TABLE_STATE, isLoading: false });
        }
    }, [selectedUnits, selectedDetailKeys]);
    return state;
};

export type FetchDataMoreProps<T extends Record<string, unknown>> = {
    session: Session;
    height: number;
    isTree?: boolean;
    fetchData: (...args: unknown[]) => Promise<T[]>;
    onExpand: (...args: unknown[]) => void;
    clickCallback?: ({ row, session, unit }: { row: T; session: Session; unit?: InsightUnit }) => void;
    doubleClickCallback?: ({ row, session, unit }: { row: T; session: Session; unit?: InsightUnit }) => void;
} & TableDataAdapter<T>;
export const FetchDataMore = observer(<T extends Record<string, unknown>>({
    session,
    height,
    isTree = false,
    fetchData,
    columns,
    actions,
    clickCallback,
    doubleClickCallback,
}: FetchDataMoreProps<T>): JSX.Element => {
    const state = useMoreUpdater(session, fetchData, { columns, actions });
    return <AutoAdjustedTable {...state} height={height}
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
