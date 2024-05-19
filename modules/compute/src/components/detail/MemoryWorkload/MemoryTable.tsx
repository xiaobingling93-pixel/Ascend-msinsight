/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React, { useEffect, useState } from 'react';
import { type Icondition } from './Filter';
import { queryMemoryTable } from '../../RequestUtils';
import { LimitHit } from '../../LimitSet';
import ResizeTable from 'lib/ResizeTable';

interface ItableDetail {
    headerName: string[];
    row: Array<{
        name: string;
        value: string[];
    }>;
}

interface ItableConfig {
    cols: any[];
    dataset: Array<Record<string, string>>;
}

interface Ilimit {
    overlimit: boolean;
    maxSize: number;
    current: number;
}

function getFullCols(headerName: string[]): any[] {
    return headerName.map((item, index) => (
        {
            title: item,
            dataIndex: item,
            ellipsis: true,
        }
    ));
}

function wrapData(data: ItableDetail[], limit: Ilimit): { tablelist: ItableConfig[] ;limit: Ilimit} {
    let count = 0;
    const tablelist = data.reduce<ItableConfig[]>((pre, tableDetail) => {
        if (count > limit.maxSize) {
            return pre;
        }
        const { headerName, row } = tableDetail;
        const cols = getFullCols(headerName);
        let dataset = row.map(item => {
            const arr = [item.name, ...item.value];
            const obj: Record<string, string> = {};
            headerName.forEach((header, index) => {
                obj[header] = arr[index];
            });
            return obj;
        });

        if (count + data.length > limit.maxSize) {
            dataset = dataset.slice(0, count + data.length - limit.maxSize);
        }
        count += dataset.length;
        pre.push({ cols, dataset });
        return pre;
    }, []);

    return { tablelist, limit: { ...limit, current: count } };
}

function Table({ condition }: {condition: Icondition}): JSX.Element {
    const [tablelist, setTablelist] = useState<ItableConfig[]>([]);
    const [limit, setLimit] = useState<Ilimit>({ overlimit: false, maxSize: 5000, current: 0 });

    const updateData = async(): Promise<void> => {
        const res = await queryMemoryTable(condition);
        const data = (res?.memoryTable?.[0]?.tableDetail ?? []) as ItableDetail[];
        const { tablelist: newTablelist, limit: newLimit } = wrapData(data, limit);
        setTablelist(newTablelist);
        setLimit(newLimit);
    };

    useEffect(() => {
        updateData();
    }, [condition.blockId]);
    return (
        <div style={{ padding: '0 20px 20px' }}>
            {tablelist.length === 0 && (<div style={{ textAlign: 'center', color: 'var(--grey15) ' }}>No data</div>) }
            {limit.overlimit && <LimitHit maxSize={limit.maxSize} name={`All Instruction Records (${limit.current})`}/>}
            {tablelist.map((item, index) => (
                <ResizeTable
                    key={`memoryTable${index}`}
                    size="small"
                    columns={item.cols ?? []}
                    dataSource={item.dataset ?? []}
                    scroll={item.dataset.length > 10 ? { y: 500 } : false}
                    pagination={false}
                />
            ))}
        </div>
    );
};

export default Table;
