/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import _ from 'lodash';
import { autorun, runInAction } from 'mobx';
import * as React from 'react';
import type { DetailDescriptor, InsightUnit, MoreDescriptor, SingleDataDesc } from '../../entity/insight';
import type { SelectedParams, Session } from '../../entity/session';
import type { TabState } from '../../entity/tabDependency';
import { useTranslation } from 'react-i18next';
import { platform } from '../../platforms';
import { logger } from '../../utils/Logger';
import { EMPTY_TABLE_STATE, type TableState } from './types';
import { onExpandForChildren, parseColDef, treeAttachInfo } from './utils';
import i18n from 'ascend-i18n';

const useFilterDeps = (selectedDetailKeys: Session['selectedDetailKeys'], trigger: any[], depsList: unknown[]): boolean => {
    const [triggerHook, setTriggerHook] = React.useState(false);
    const selectedDetailKeysRef = React.useRef(selectedDetailKeys);
    const triggerRef = React.useRef(trigger);
    const depsListRef = React.useRef(depsList);

    const isChangedArray = (raw: unknown[], target: unknown[]): boolean => {
        if (raw.length !== target.length) { return true; }
        for (let i = 0; i < raw.length; i++) {
            if (raw[i] !== target[i]) { return true; }
        }
        return false;
    };

    if (isChangedArray(selectedDetailKeys, selectedDetailKeysRef.current) ||
        isChangedArray(trigger, triggerRef.current) ||
        isChangedArray(depsList, depsListRef.current)) {
        setTriggerHook((prev) => !prev);
    }

    selectedDetailKeysRef.current = selectedDetailKeys;
    triggerRef.current = trigger;
    depsListRef.current = depsList;

    return triggerHook;
};

interface HandleFetchDataParams {
    result: Array<Record<string, unknown>>;
    session: Session;
    recentUnits: React.MutableRefObject<InsightUnit[]>;
    recentRange: React.MutableRefObject<[number, number] | undefined>;
    tabState?: TabState;
    selectedUnit?: InsightUnit;
    detail: DetailDescriptor<unknown>;
    setState: React.Dispatch<React.SetStateAction<TableState>>;
    onDataLoaded?: (data: unknown[]) => void;
}

const handleFetchedData = ({
    result, session, recentUnits, recentRange, tabState, selectedUnit, detail, setState, onDataLoaded,
}: HandleFetchDataParams): void => {
    const { selectedUnits, selectedRange } = session;
    if (recentUnits.current !== selectedUnits || recentRange.current !== selectedRange) { return; }
    result.forEach(treeAttachInfo);
    if (tabState?.data !== undefined && tabState?.filter !== undefined) {
        runInAction(() => { tabState.data = result; });
    } else {
        const columns = parseColDef(detail, session, tabState ?? selectedUnit?.tabState).map(col => ({
            ...col,
            title: i18n.t(`sliceList.${col.title}`, { ns: 'timeline', defaultValue: col.title }),
        }));
        setState({
            dataSource: result,
            columns,
            rowKey: (detail.rowKey ?? undefined) as (row: object) => string,
            onExpand: onExpandForChildren(session, detail.onExpand, setState),
            loading: false,
        });
        onDataLoaded?.(result);
    }
};

export const useDetailUpdater = (session: Session, detail: DetailDescriptor<unknown> | undefined, tabState: TabState | undefined,
    dep: unknown = [], onDataLoaded?: (data: unknown[]) => void): TableState => {
    const [state, setState] = React.useState<TableState>(EMPTY_TABLE_STATE);
    const { selectedUnits, selectedRange, selectedDetailKeys } = session;
    const selectedUnit = selectedUnits.length > 0 ? selectedUnits[0] : undefined;
    const fetchData = session.phase === 'error' ? undefined : detail?.fetchData;
    const trigger = tabState?.trigger !== undefined ? Object.values(tabState.trigger) : [];
    const depsList = Array.isArray(dep) ? dep : [dep];
    const hookTrigger = useFilterDeps(selectedDetailKeys, trigger, depsList);

    const onDataFetched = React.useMemo(() => ([selectedUnits, selectedRange].filter(_.isEmpty).length === 0
        ? fetchData?.(session, selectedUnit?.metadata)
        : undefined), [selectedUnits, selectedRange, hookTrigger, detail]);

    const recentUnits = React.useRef(selectedUnits);
    const recentRange = React.useRef(selectedRange);
    recentUnits.current = selectedUnits;
    recentRange.current = selectedRange;

    const loadData = (): void => {
        if (detail && onDataFetched && selectedRange !== undefined) {
            logger('DetailPanel', `[DetailPanel] calling ${selectedUnit?.name ?? ''}'s fetchData`);
            setState({ ...EMPTY_TABLE_STATE, loading: true });
            onDataFetched?.then(result => {
                handleFetchedData({ result, session, recentUnits, recentRange, tabState, selectedUnit, detail, setState, onDataLoaded });
            }).catch(() => {
                setState(EMPTY_TABLE_STATE);
                platform.dialog(i18n.t('error:4004'));
            });
        } else {
            setState(EMPTY_TABLE_STATE);
        }
    };

    React.useEffect(() => {
        if (session.selectedData?.showSelectedData !== true) {
            loadData();
        }
    }, [selectedUnits, selectedRange, detail, ...trigger, ...depsList, session.language]);
    // 需要进行多选过滤的才会执行下面的代码
    React.useEffect(() =>
        autorun(() => {
            if (detail === undefined || tabState?.filter === undefined) { return; }
            setState({
                dataSource: tabState.getFilterData(),
                columns: parseColDef(detail, session, tabState),
                rowKey: (detail.rowKey ?? undefined) as (row: object) => string,
                onExpand: onExpandForChildren(session, detail.onExpand, setState),
                loading: false,
            });
        }), []);
    return state;
};

export const useMoreUpdater = (session: Session, more: MoreDescriptor | undefined): TableState => {
    const [state, setState] = React.useState<TableState>(EMPTY_TABLE_STATE);
    const { selectedUnits, selectedDetailKeys, selectedDetails } = session;

    React.useEffect(() => {
        const selectedDetail = selectedDetails.length > 0 ? selectedDetails[0] : undefined;
        setState({ ...EMPTY_TABLE_STATE, loading: true });
        if (more && selectedDetail) {
            setState({
                dataSource: selectedDetail[more.field] as object[],
                columns: parseColDef(more, session),
                rowKey: (more.rowKey ?? undefined) as (row: object) => string,
                onExpand: onExpandForChildren(session, more.onExpand, setState),
                loading: false,
            });
        } else {
            setState(EMPTY_TABLE_STATE);
        }
    }, [selectedUnits, selectedDetailKeys, more]);
    return state;
};

export const useSelectedParamsDetailUpdater = (session: Session, detail: DetailDescriptor<unknown> | undefined):
TableState => {
    const [state, setState] = React.useState<TableState>(EMPTY_TABLE_STATE);
    const { selectedUnits } = session;
    const dependenciesKeys = Object.keys(session.selectedParams) as unknown as Array<keyof SelectedParams>;
    const hasDependencies = _.isEmpty(dependenciesKeys.filter((dependenciesKey) => session.selectedParams[dependenciesKey] === undefined));
    const dependencies = dependenciesKeys.map((dependencyKey) => session.selectedParams[dependencyKey]);
    const { t } = useTranslation();
    const loadData = (): void => {
        const selectedUnit = selectedUnits.length > 0 ? selectedUnits[0] : undefined;
        const fetchData = detail?.fetchData;
        if (fetchData && hasDependencies && session.phase === 'download') {
            logger('DetailPanel', `[DetailPanel] calling ${selectedUnit?.name ?? ''}'s fetchData`);
            platform.trace('useComparison', {});
            setState({ ...EMPTY_TABLE_STATE, loading: true });
            fetchData(session, selectedUnit?.metadata).then(result => {
                result.forEach(treeAttachInfo);
                setState({
                    dataSource: result,
                    columns: parseColDef(detail, session),
                    rowKey: (detail.rowKey ?? undefined) as (row: object) => string,
                    onExpand: onExpandForChildren(session, detail.onExpand, setState),
                    loading: false,
                });
            }).catch(() => {
                setState(EMPTY_TABLE_STATE);
                platform.dialog(t('error:4004'));
            });
        } else {
            setState(EMPTY_TABLE_STATE);
        }
    };
    React.useEffect(loadData, [...dependencies, detail]);
    return state;
};

export const useExtraDataUpdater = <T extends DetailDescriptor<unknown>>(
    session: Session, detail: T | undefined,
): Record<string, unknown> => {
    const [state, setState] = React.useState({});
    const { selectedUnits, endTimeAll } = session;
    const selectedUnit = selectedUnits.length > 0 ? selectedUnits[0] : undefined;
    const loadData = (): void => {
        const fetchExtraData = detail?.fetchExtraData;
        if (detail && fetchExtraData && session.phase === 'download') {
            logger('DetailPanel', `[DetailPanel] calling ${selectedUnit?.name ?? ''}'s fetchData`);
            fetchExtraData(session, selectedUnit?.metadata).then(result => {
                if (result !== undefined) {
                    setState({ result });
                }
            }).catch(() => {
                setState({});
                logger('useExtraDataUpdater', 'fetchExtraData occurred an exception.');
            });
        } else {
            setState({});
        }
    };
    React.useEffect(loadData, [endTimeAll, detail]);
    return { ...state };
};

export const useSelectedDataDetailUpdater = (session: Session, detail: SingleDataDesc<Record<string, unknown>, unknown>, selectedData: unknown):
{ renderFields?: Array<[string, string | JSX.Element]>; data: Record<string, unknown> } => {
    const [renderFields, setRenderFields] = React.useState<Array<[string, string | JSX.Element]>>();
    const [state, setState] = React.useState<Record<string, unknown>>({});
    const { selectedUnits } = session;
    const selectedUnit = selectedUnits.length > 0 ? selectedUnits[0] : undefined;
    const fetchData = session.phase === 'error' ? undefined : detail?.fetchData;
    const onDataFetched = React.useMemo(() => ([selectedUnits, selectedData].filter(_.isEmpty).length === 0
        ? fetchData?.(session, selectedUnit?.metadata)
        : undefined), [selectedUnits, selectedData, detail]);

    const recentUnits = React.useRef(selectedUnits);
    const recentData = React.useRef(selectedData);
    const { t } = useTranslation();
    recentUnits.current = selectedUnits; recentData.current = selectedData;

    const loadData = (): void => {
        if (onDataFetched !== undefined && selectedData !== undefined && session.phase === 'download') {
            logger('DetailPanel', `[DetailPanel] calling ${selectedUnit?.name ?? ''}'s fetchData`);
            onDataFetched?.then(result => {
                if (recentUnits.current !== selectedUnits || recentData.current !== selectedData) { return; }
                setState(result);
                const renderField: Array<[string, string | JSX.Element]> = [];
                detail.renderFields.forEach(item => {
                    const render = item[1];
                    if (item[2] !== undefined) {
                        const isHiden = item[2];
                        if (!isHiden(result)) {
                            renderField.push([item[0], render(result, session)]);
                        }
                    } else {
                        renderField.push([item[0], render(result, session)]);
                    }
                });
                setRenderFields(renderField);
            }).catch(() => {
                platform.dialog(t('error:4004'));
            });
        }
    };

    React.useEffect(loadData, [selectedUnits, selectedData, detail]);
    return { renderFields, data: state };
};
