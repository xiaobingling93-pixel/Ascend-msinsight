/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import type { ColumnType } from './types';

export interface ColGroupProps<RecordType> {
    colWidths: ReadonlyArray<number | string | undefined>;
    columns?: ReadonlyArray<ColumnType<RecordType>>;
    columCount?: number;
}

export function ColGroup<RecordType>({ colWidths, columns, columCount }: ColGroupProps<RecordType>): JSX.Element {
    const cols: React.ReactElement[] = [];
    const len = columCount ?? columns?.length ?? 0;

    let shouldInsert = false;
    for (let i = len - 1; i >= 0; i--) {
        const width = columns?.[i]?.width ?? colWidths[i];

        if (width || shouldInsert) {
            cols.unshift(<col key={i} width={width} />);
            shouldInsert = true;
        }
    }

    return <colgroup>{cols}</colgroup>;
}
