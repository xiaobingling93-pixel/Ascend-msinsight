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
import { DownOutlined } from '@ant-design/icons';
import { Button } from 'ascend-components';

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

const renderExpandColomn = (record: any, setExpandedKeys: React.Dispatch<React.SetStateAction<string[]>>): JSX.Element => {
    return record.source === 'Difference'
        ? (<Button type="link"
            onClick={(): void => {
                setExpandedKeys((pre: any) => {
                    const list = [...pre];
                    const keyIndex = list.indexOf(record.key);
                    if (keyIndex === -1) {
                        list.push(record.key);
                    } else {
                        list.splice(keyIndex, 1);
                    }
                    return list;
                });
            }}>see more<DownOutlined/></Button>)
        : <></>;
};

function getFullCols(headerName: string[], tDetails: any, isCompared: boolean, setExpandedKeys: React.Dispatch<React.SetStateAction<string[]>>): any[] {
    const dataColumns: any[] = headerName.map((item, index) => (
        {
            key: item,
            title: index === 0 ? item : tDetails(firstLetterUpper(item)),
            dataIndex: item,
            ellipsis: true,
        }
    ));
    if (isCompared) {
        dataColumns.push({
            key: 'action',
            title: 'Details',
            dataIndex: 'action',
            ellipsis: true,
            fixed: 'right',
            render: (_: any, record: any): JSX.Element => {
                return renderExpandColomn(record, setExpandedKeys);
            },
        });
        dataColumns.splice(1, 0, {
            key: 'source',
            title: 'Source',
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

function wrapData(data: ItableDetail[], limit: Ilimit, tDetails: any, isCompared: boolean,
    setExpandedKeys: React.Dispatch<React.SetStateAction<string[]>>): { tablelist: ItableConfig[] ;limit: Ilimit} {
    let count = 0;
    const tablelist = data.reduce<ItableConfig[]>((pre, tableDetail) => {
        if (count > limit.maxSize) {
            count += tableDetail?.row?.length ?? 0;
            return pre;
        }
        const { headerName = [], row = [], tableName = '' } = tableDetail ?? {};
        headerName[0] = tableName;
        const cols = getFullCols(headerName, tDetails, isCompared, setExpandedKeys);
        let dataset = row.map(item => {
            const compare: Record<string, string> = covertRowToRecord(item.compare, headerName);
            if (!isCompared) {
                return compare;
            }
            const res: Record<string, string | Array<Record<string, string>>> = covertRowToRecord(item.diff, headerName);
            res.source = 'Difference';
            compare.source = 'Compare';
            const baseline: Record<string, string> = covertRowToRecord(item.baseline, headerName);
            baseline.source = 'Baseline';
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

const defaultLimit = { overlimit: false, maxSize: 1000, current: 0 };
const memoryTable = observer(({ condition, session }: {condition: Icondition;session: Session}): JSX.Element => {
    const [data, setData] = useState<ItableDetail[]>([]);
    const [advice, setAdvice] = useState<string[]>([]);
    const [tablelist, setTablelist] = useState<ItableConfig[]>([]);
    const [limit, setLimit] = useState<Ilimit>(defaultLimit);
    const { t: tDetails } = useTranslation('details');
    const [expandedRowKeys, setExpandedKeys] = React.useState<string[]>([]);

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
    }, [condition, session.parseStatus]);
    useEffect(() => {
        const { tablelist: newTablelist, limit: newLimit } = wrapData(data, limit, tDetails, session.dirInfo.isCompare, setExpandedKeys);
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
                    dataSource={item.dataset.map((row, rowIndex) => { return { ...row, key: `memoryTable${index}_${rowIndex}` }; })}
                    scroll={item.dataset.length > 10 ? { y: 500 } : undefined}
                    pagination={false}
                    expandable={{
                        expandIcon: () => <></>,
                        expandedRowKeys,
                    }}
                />
            ))}
            {advice.length > 0 && (<Hit text={advice} style={{ marginTop: '10px' }}/>) }
        </div>
    );
});

export default memoryTable;
