/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
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
