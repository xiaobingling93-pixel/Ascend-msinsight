/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React, { useEffect, useState } from 'react';
import { observer } from 'mobx-react';
import { useTranslation } from 'react-i18next';
import { type Icondition } from './Filter';
import { queryMemoryTable } from '../../RequestUtils';
import { LimitHit } from '../../LimitSet';
import { ResizeTable } from 'ascend-resize';
import { firstLetterUpper, Hit } from 'ascend-utils';
import { type Session } from '../../../entity/session';
import { CompareData } from '../../../utils/interface';

interface ItableDetail {
    tableName: string;
    headerName: string[];
    row: Array<CompareData<RowDetail>>;
}

interface RowDetail {
    name: string;
    value: string[];
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

function getFullCols(headerName: string[], tDetails: any): any[] {
    return headerName.map((item, index) => (
        {
            title: index === 0 ? item : tDetails(firstLetterUpper(item)),
            dataIndex: item,
            ellipsis: true,
        }
    ));
}

function wrapData(data: ItableDetail[], limit: Ilimit, tDetails: any): { tablelist: ItableConfig[] ;limit: Ilimit} {
    let count = 0;
    const tablelist = data.reduce<ItableConfig[]>((pre, tableDetail) => {
        if (count > limit.maxSize) {
            count += tableDetail?.row?.length ?? 0;
            return pre;
        }
        const { headerName = [], row = [], tableName = '' } = tableDetail ?? {};
        headerName[0] = tableName;
        const cols = getFullCols(headerName, tDetails);
        let dataset = row.map(item => {
            const arr = [item.compare.name, ...item.compare.value];
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

    return { tablelist, limit: { ...limit, current: count, overlimit: count > limit.maxSize } };
}

const defaultLimit = { overlimit: false, maxSize: 1000, current: 0 };
const memoryTable = observer(({ condition, session }: {condition: Icondition;session: Session}): JSX.Element => {
    const [data, setData] = useState<ItableDetail[]>([]);
    const [advice, setAdvice] = useState<string[]>([]);
    const [tablelist, setTablelist] = useState<ItableConfig[]>([]);
    const [limit, setLimit] = useState<Ilimit>(defaultLimit);
    const { t: tDetails } = useTranslation('details');

    const updateData = async(): Promise<void> => {
        const res = await queryMemoryTable(condition);
        const newData = (res?.memoryTable?.[0]?.tableDetail ?? []) as ItableDetail[];
        const newAdvice = (res?.memoryTable?.[0]?.advice ?? []) as string[];
        setData(newData);
        setAdvice(newAdvice);
    };

    useEffect(() => {
        if (!session.parseStatus) {
            setTimeout(() => {
                if (!session.parseStatus) {
                    setTablelist([]);
                    setLimit(defaultLimit);
                }
            }, 200);
            return;
        }
        updateData();
    }, [condition.blockId, session.parseStatus]);
    useEffect(() => {
        const { tablelist: newTablelist, limit: newLimit } = wrapData(data, limit, tDetails);
        setTablelist(newTablelist);
        setLimit(newLimit);
    }, [data, tDetails]);
    return (
        <div>
            {tablelist.length === 0 && (<div style={{ textAlign: 'center', color: 'var(--grey15) ' }}>No data</div>) }
            {limit.overlimit && <LimitHit maxSize={limit.maxSize} name={`${tDetails('Memory Workload Table Records')}(${limit.current})`}/>}
            {tablelist.map((item, index) => (
                <ResizeTable
                    key={`memoryTable${index}`}
                    size="small"
                    columns={item.cols ?? []}
                    dataSource={item.dataset ?? []}
                    scroll={item.dataset.length > 10 ? { y: 500 } : undefined}
                    pagination={false}
                />
            ))}
            {advice.length > 0 && (<Hit text={advice} style={{ marginTop: '10px' }}/>) }
        </div>
    );
});

export default memoryTable;
