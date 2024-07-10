import * as React from 'react';
import { observer } from 'mobx-react';
import { runInAction } from 'mobx';
import { Row, Col, Select } from 'antd';
import { RowProps } from 'antd/lib/row';
import styled from '@emotion/styled';
import _ from 'lodash';
import { useTranslation } from 'react-i18next';
import { TableViewProps, TableState } from './types';
import { Session, SelectedParams } from '../../entity/session';
import { selectRow } from './utils';
import { AutoAdjustedTable } from './base/AutoAdjustedTable';
import { useSelectedParamsDetailUpdater, useExtraDataUpdater } from './hooks';
import { StyledSelect } from '../base/StyledSelect';
import { ReactComponent as Arrow } from '../../assets/images/comparisonArrow.svg';
import { CommonStateProto } from './base/Tabs';
import { DetailTabs } from './TabPanes';

const CONTROLLERBAR_HEIGHT = 48;

const ControllerBar = styled((props: RowProps) => <Row {...props}/>)`
    height: ${CONTROLLERBAR_HEIGHT}px;
    background-color: ${(props): string => props.theme.controllerBarBackgroundColor};
    color: ${(props): string => props.theme.thumbIconBackgroundColor};
`;

const ArrowIcon = styled((props: { width: number; height: number }) => <Arrow {...props}/>)`
    fill: ${(props): string => props.theme.thumbIconBackgroundColor};
`;

const StaticTable = function<T extends CommonStateProto>({ session, height, detail, state, commonState }: TableViewProps<DetailTabs, T> & { state: TableState }): JSX.Element {
    const unit = session.selectedUnits[0];
    return <AutoAdjustedTable
        height={height - CONTROLLERBAR_HEIGHT}
        {...state}
        rowSelection={{
            selectedRowKeys: session.selectedDetailKeys,
        }}
        expandable={{ onExpand: state.onExpand }}
        onRow={(row): React.HTMLAttributes<any> => ({
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
};

const useComparisonUpdater = <T extends CommonStateProto>(session: Session, detail: TableViewProps<DetailTabs, T>['detail']): TableState & { extraData: Record<string, unknown> } => {
    const state = useSelectedParamsDetailUpdater(session, detail);
    const extraData = useExtraDataUpdater(session, detail) ?? {};

    return { ...state, extraData };
};

const getHandleFuncs = (session: Session, selectedParams: Array<keyof SelectedParams>): Array<(rawId: string) => void> => {
    return selectedParams.map((curSelectedParam) => (rawId: string) => {
        _.debounce((rawId) => {
            runInAction(() => {
                session.selectedDetailKeys = [];
                session.selectedDetails = [];
                selectedParams.filter((selectedParam) => selectedParam !== curSelectedParam && session.selectedParams[selectedParam] === Number(rawId))
                    .forEach((selectedParam) => { session.selectedParams[selectedParam] = undefined; });
                session.selectedParams[curSelectedParam] = Number(rawId);
            });
        }, 300)(rawId);
    });
};

const getDisplayValue = (session: Session, comparisonInfos: unknown[], callbackFunc: CallbackFunc['processDisplayValue']): [ string | undefined, string | undefined, number | undefined, number | undefined ] => {
    if (!Array.isArray(comparisonInfos)) {
        return [undefined, undefined, undefined, undefined];
    }
    const [baseValue, baseId] = callbackFunc(session, comparisonInfos, true);
    const [curValue, curId] = callbackFunc(session, comparisonInfos, false);

    return [baseValue, curValue, baseId, curId];
};

const useFilterListValues = (comparisonInfos: unknown[], callbackFunc: CallbackFunc['filterListValues'], baseId?: number, curId?: number): [ unknown[], unknown[] ] => {
    const baseListValues = React.useMemo(callbackFunc(comparisonInfos, baseId), [comparisonInfos, baseId]);
    const curListValues = React.useMemo(callbackFunc(comparisonInfos, curId), [comparisonInfos, curId]);
    return [baseListValues, curListValues];
};

type CallbackFunc = {
    processDisplayValue: (session: Session, info: unknown[], isBase: boolean) => [(string | undefined), (number | undefined)];
    filterListValues: (info: unknown[], id?: number) => () => unknown[];
    createSelectOption: (info: unknown) => [ number, string ];
};

type ComparisonDetailProps<T extends CommonStateProto> = TableViewProps<DetailTabs, T> & CallbackFunc;

export const ComparisonDetail = observer(<T extends CommonStateProto>(
    { session, height, detail, processDisplayValue, filterListValues, createSelectOption }: ComparisonDetailProps<T>): JSX.Element => {
    const { t } = useTranslation();
    const state = useComparisonUpdater(session, detail);
    const [handleBase, handleCur] = getHandleFuncs(session, Object.keys(session.selectedParams) as unknown as Array<keyof SelectedParams>);
    const [baseValue, curValue, baseId, curId] = getDisplayValue(session, state.extraData.result as unknown[], processDisplayValue);
    const [baseListValues, curListValues] = useFilterListValues(state.extraData.result as unknown[], filterListValues, baseId, curId);
    return (
        <>
            <ControllerBar gutter={7} align="middle">
                <Col style={{ marginLeft: 10 }}>{t('Comparison Base')}</Col>
                <Col><StyledSelect
                    width={130}
                    height={32}
                    onChange={handleBase}
                    value={baseValue}
                >
                    {baseListValues?.map((item) => {
                        const [rawId, selectStr] = createSelectOption(item);
                        return <Select.Option key={rawId}>{selectStr}</Select.Option>;
                    })}
                </StyledSelect></Col>
                <Col><ArrowIcon width={13} height={13}/></Col>
                <Col>{t('Comparison Target')}</Col>
                <Col><StyledSelect
                    width={130}
                    height={32}
                    onChange={handleCur}
                    value={curValue}
                >
                    {curListValues?.map((item) => {
                        const [rawId, selectStr] = createSelectOption(item);
                        return <Select.Option key={rawId}>{selectStr}</Select.Option>;
                    })}
                </StyledSelect></Col>
            </ControllerBar>
            <Row><StaticTable
                height={height}
                session={session}
                detail={detail}
                state={state}
            /></Row>
        </>
    );
});
