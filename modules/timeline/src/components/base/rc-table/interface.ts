/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import type {Key} from './types';

export interface FilterConfirmProps {
    closeDropdown: boolean;
}

export interface TableHandle<RecordType = unknown> {
    scrollTo: (node: RecordType) => void;
    appendExpandedKeys: (res: Key[]) => void;
    selectFirstRoot: () => void;
};
