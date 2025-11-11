/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React, { useEffect, useState } from 'react';
import { observer } from 'mobx-react';
import { useTranslation } from 'react-i18next';
import { type Icondition } from './Filter';
import { queryMemoryTable } from '../../RequestUtils';
import { LimitHit } from '../../LimitSet';
import { ResizeTable } from '@insight/lib/resize';
import { firstLetterUpper, Hit } from '@insight/lib/utils';
import { type Session } from '../../../entity/session';
import { CompareData } from '../../../utils/interface';
import { type Theme, useTheme } from '@emotion/react';
import { getContextElement, renderExpandColumn } from '../../Common';

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
    dataset: Array<Record<string, string | Array<Record<string, string>>>>;
}

interface Ilimit {
    overlimit: boolean;
    maxSize: number;
    current: number;
}

function getFullCols({ headerName, tDetails, isCompared, setExpandedKeys, theme }: { headerName: string[]; tDetails: any;
    isCompared: boolean; setExpandedKeys: React.Dispatch<React.SetStateAction<string[]>>; theme: Theme; }): any[] {
    const dataColumns: any[] = headerName.map((item, index) => (
        {
            key: item,
            title: index === 0 ? item : tDetails(firstLetterUpper(item)),
            dataIndex: item,
            ellipsis: true,
            render: (text: string, record: any): JSX.Element => getContextElement(text, record, theme, tDetails),
        }
    ));
    if (isCompared) {
        dataColumns.push({
            key: 'action',
            title: tDetails('Details'),
            dataIndex: 'action',
            ellipsis: true,
            fixed: 'right',
            render: (_: any, record: any): JSX.Element => {
                return renderExpandColumn(record, setExpandedKeys, tDetails);
            },
        });
        dataColumns.splice(1, 0, {
            key: 'source',
            title: tDetails('Source'),
            dataIndex: 'source',
            ellipsis: true,
        });
    }
    return dataColumns;
}

function covertRowToRecord(row: RowDetail, headerName: string[]): Record<string, string> {
    const arr = [row.name, ...row.value];
    const obj: Record<string, string> = {};
    headerName.forEach((header, index) => {
        obj[header] = arr[index];
    });
    return obj;
}

function wrapData({ data, limit, tDetails, isCompared, setExpandedKeys, theme }: { data: ItableDetail[]; limit: Ilimit;
    tDetails: any; isCompared: boolean; setExpandedKeys: React.Dispatch<React.SetStateAction<string[]>>; theme: Theme; }):
    { tablelist: ItableConfig[] ;limit: Ilimit} {
    let count = 0;
    const tablelist = data.reduce<ItableConfig[]>((pre, tableDetail) => {
        if (count > limit.maxSize) {
            count += tableDetail?.row?.length ?? 0;
            return pre;
        }
        const { headerName = [], row = [], tableName = '' } = tableDetail ?? {};
        headerName[0] = tableName;
        const cols = getFullCols({ headerName, tDetails, isCompared, setExpandedKeys, theme });
        let dataset = row.map(item => {
            const compare: Record<string, string> = covertRowToRecord(item.compare, headerName);
            if (!isCompared) {
                return compare;
            }
            const res: Record<string, string | Array<Record<string, string>>> = covertRowToRecord(item.diff, headerName);
            res.source = tDetails('Difference');
            compare.source = tDetails('Comparison');
            const baseline: Record<string, string> = covertRowToRecord(item.baseline, headerName);
            baseline.source = tDetails('Baseline');
            res.children = [compare, baseline] as Array<Record<string, string>>;
            return res;
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

async function getMemoryData(condition: Icondition): Promise<{data: ItableDetail[];advice: string[]}> {
    if (condition.blockId === '') {
        return { data: [], advice: [] };
    }

    const res = await queryMemoryTable(condition);
    const data = (res?.memoryTable?.[0]?.tableDetail ?? []) as ItableDetail[];
    const advice = (res?.memoryTable?.[0]?.advice ?? []) as string[];
    return { data, advice };
}

const defaultLimit = { overlimit: false, maxSize: 1000, current: 0 };
const memoryTable = observer(({ condition, session }: {condition: Icondition;session: Session}): JSX.Element => {
    const [data, setData] = useState<ItableDetail[]>([]);
    const [advice, setAdvice] = useState<string[]>([]);
    const [tablelist, setTablelist] = useState<ItableConfig[]>([]);
    const [limit, setLimit] = useState<Ilimit>(defaultLimit);
    const { t: tDetails } = useTranslation('details');
    const [expandedRowKeys, setExpandedKeys] = React.useState<string[]>([]);
    const theme = useTheme();
    const updateData = async(): Promise<void> => {
        const { data: newData, advice: newAdvice } = await getMemoryData(condition);
        setData(newData);
        setAdvice(newAdvice);
    };

    useEffect(() => {
        if (!session.parseStatus) {
            setTablelist([]);
            setAdvice([]);
            setLimit(defaultLimit);
            return;
        }
        updateData();
    }, [condition, session.parseStatus]);
    useEffect(() => {
        const { tablelist: newTablelist, limit: newLimit } =
            wrapData({ data, limit, tDetails, isCompared: session.dirInfo.isCompare, setExpandedKeys, theme });
        setTablelist(newTablelist);
        setLimit(newLimit);
    }, [data, tDetails]);
    return (
        <div>
            {advice.length > 0 && (<Hit text={advice} style={{ marginBottom: '10px' }} type={'alarm'} data-testId="memoryWorkloadAdvice"/>) }
            {tablelist.length === 0 && (<div style={{ textAlign: 'center', color: 'var(--grey15) ' }}>No data</div>) }
            {limit.overlimit && <LimitHit maxSize={limit.maxSize} name={`${tDetails('Memory Workload Table Records')}(${limit.current})`}/>}
            <div data-testId={'memoryWorkloadTable'}>
                {tablelist.map((item, index) => (
                    <ResizeTable
                        key={`memoryTable${index}`}
                        size="small"
                        columns={item.cols ?? []}
                        dataSource={item.dataset.map((row, rowIndex) => { return { ...row, key: `memoryTable${index}_${rowIndex}` }; })}
                        scroll={item.dataset.length > 10 ? { y: 500 } : undefined}
                        pagination={false}
                        expandable={{
                            expandIcon: () => <></>,
                            expandedRowKeys,
                        }}
                    />
                ))}
            </div>

        </div>
    );
});

export default memoryTable;
