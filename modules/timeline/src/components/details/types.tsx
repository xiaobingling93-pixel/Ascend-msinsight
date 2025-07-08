/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import type { ColumnType } from 'antd/es/table';
import type { AutoKey } from '../../utils/dataAutoKey';
import type { Session } from '../../entity/session';
import { DetailDescriptor, MoreDescriptor, SummaryFunction } from '../../entity/insight';
import type { TabComponentProps, TabProto, CommonStateProto } from './base/Tabs';
import type { TabState } from '../../entity/tabDependency';
import React from 'react';

export interface TableViewProps<Tab extends TabProto, TabsState extends CommonStateProto> {
    session: Session;
    detail?: DetailDescriptor<unknown>;
    height: number; // The height of the drawing area for this view, which is necessary to correctly draw a scrollbar.
    isTree?: boolean;
    tabState?: TabState; // used for decide what to show in FilterContainer
    commonState?: TabComponentProps<Tab, TabsState>['commonState'];
    depsList?: unknown[];
    tabs?: TabComponentProps<Tab, TabsState>['tabs'];
    interactorProps?: TabComponentProps<Tab, TabsState>['interactorProps'];
    onDataLoaded?: (data: unknown[]) => void;
    summaryBuilder?: (state: TableState, dataSource: Array<AutoKey<object>>) => React.ReactNode;
};

export interface MoreTableProps {
    more?: MoreDescriptor;
    session: Session;
    height: number;
    isTree?: boolean;
};

export interface TableState<T extends object = Record<string, unknown>> {
    dataSource: Array<AutoKey<T>>;
    columns: Array<ColumnType<T> & { summary?: SummaryFunction<T> }>;
    rowKey?: (row: object) => string;
    onExpand?: (expanded: boolean, record: T) => void;
    loading: boolean;
};

export const EMPTY_TABLE_STATE: TableState<any> = {
    dataSource: [],
    columns: [],
    loading: false,
};
