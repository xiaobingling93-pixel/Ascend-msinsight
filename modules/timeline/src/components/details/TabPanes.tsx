/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
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
