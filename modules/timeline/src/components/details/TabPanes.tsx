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
import * as React from 'react';
import type { DetailDescriptor } from '../../entity/insight';
import type { TabProto } from './base/Tabs';
import type { OptionType, TabState } from '../../entity/tabDependency';
export interface DetailTabs extends TabProto {
    detail: DetailDescriptor<unknown>;
    bottomPanel?: Partial<CommonBottomPanel>;
    tabState?: TabState;
};

export interface CommonBottomPanel<DetailProps = any, MoreProps = any> {
    Detail?: React.FC<DetailProps>;
    detailProps?: DetailProps;
    DetailTitle?: React.FC<Record<string, unknown>> | string;
    More?: React.FC<MoreProps>;
    moreProps?: MoreProps;
    MoreTitle?: React.FC<Record<string, unknown>> | string;
    options?: OptionType[];
};
