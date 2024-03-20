/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/

import type { CompareFn, FilterConfirmProps } from 'antd/es/table/interface';
import React from 'react';
import { Button, Input } from 'antd';
import { Space } from 'antd/lib/index';
import { SearchOutlined } from '@ant-design/icons';
import type { ColumnType } from 'antd/es/table';

interface ColumData {
    title: string;
    dataIndex: string;
    key: string;
    sorter?: boolean | CompareFn<any>;
    showSorterTooltip?: boolean;
}

interface SearchData {
    dataIndex: string;
    setSearchText: (text: string) => void;
    searchText: string;
    setSearchedColumn: (text: string) => void;
    searchedColumn: string;
}

export const getDefaultColumData = (key: string): {
    sorter: boolean;
    showSorterTooltip: boolean;
    ellipsis: boolean;
    key: string; } => {
    return {
        sorter: true,
        showSorterTooltip: false,
        ellipsis: true,
        key,
    };
};

export interface VoidFunction {
    (...rest: any[]): void;
}
export function isNull(val: any): boolean {
    return val === undefined || val === null || val === '';
}

export const Label = (props: {name: string;style?: object }): JSX.Element => {
    return <span style={{ margin: '0 10px', ...(props.style ?? {}) }}>{props.name ? props.name + ' :' : ''} </span>;
};

export const pythonApiSummaryColumns: ColumData[] = [
    { title: 'Time(%)', dataIndex: 'time', ...getDefaultColumData('time') },
    { title: 'Total Time(us)', dataIndex: 'totalTime', ...getDefaultColumData('totalTime') },
    { title: 'Num Calls', dataIndex: 'numberCalls', ...getDefaultColumData('numberCalls') },
    { title: 'Avg(us)', dataIndex: 'avg', ...getDefaultColumData('avg') },
    { title: 'Min(us)', dataIndex: 'min', ...getDefaultColumData('min') },
    { title: 'Max(us)', dataIndex: 'max', ...getDefaultColumData('max') },
];

export const kernelDetails: ColumData[] = [
    { title: 'Type', dataIndex: 'type', ...getDefaultColumData('type') },
    { title: 'Accelerator Core', dataIndex: 'acceleratorCore', ...getDefaultColumData('acceleratorCore') },
    { title: 'Start Time', dataIndex: 'startTime', ...getDefaultColumData('startTime') },
    { title: 'Duration(us)', dataIndex: 'duration', ...getDefaultColumData('duration') },
    { title: 'Wait Time(us)', dataIndex: 'waitTime', ...getDefaultColumData('waitTime') },
    { title: 'Block Dim', dataIndex: 'blockDim', ...getDefaultColumData('blockDim') },
    { title: 'Input Shapes', dataIndex: 'inputShapes', ...getDefaultColumData('inputShapes') },
    { title: 'Input Data Types', dataIndex: 'inputDataTypes', ...getDefaultColumData('inputDataTypes') },
    { title: 'Input Formats', dataIndex: 'inputFormats', ...getDefaultColumData('inputFormats') },
    { title: 'Output Shapes', dataIndex: 'outputShapes', ...getDefaultColumData('outputShapes') },
    { title: 'Output Data Types', dataIndex: 'outputDataTypes', ...getDefaultColumData('outputDataTypes') },
    { title: 'Output Formats', dataIndex: 'outputFormats', ...getDefaultColumData('outputFormats') },

];

export const systemViewItems: Array<{title: string;key: string}> = [
    { title: 'Python API Summary', key: '0' },
    { title: 'CANN API Summary', key: '1' },
    { title: 'Ascend HardWare Task Summary', key: '2' },
    { title: 'HCCL Summary', key: '3' },
    { title: 'Overlap Analysis', key: '4' },
    { title: 'Kernel Details', key: '5' },
];

export const GetPageData = (page: { current: number; pageSize: number; total: number }, setPage: VoidFunction): object => {
    return {
        ...page,
        showSizeChanger: page.total > 10,
        pageSizeOptions: [10, 20, 50, 100],
        showTotal: (total: number) => (<div style={{ marginRight: '10px' }}>Total {total} items</div>),
        hideOnSinglePage: false,
        onChange: (current: number, pageSize: number) => { setPage({ ...page, current, pageSize }); },
        showQuickJumper: page.total / (page.pageSize === 0 ? 1 : page.pageSize) > 5,
    };
};

export const querySystemViewDetails = async(param: {
    isQueryTotal: boolean; rankId: string; pageSize: number; current: number; orderBy: string; order: string;
    layer: string; searchName: string;
}): Promise<any> => {
    return window.requestData('unit/systemView', param, 'timeline');
};

export const queryKernelDetails = async(param: {
    rankId: string; pageSize: number; current: number; orderBy: string; order: string;
    coreType: string; searchName: string;
}): Promise<any> => {
    return window.requestData('unit/kernelDetails', param, 'timeline');
};

export const queryOneKernel = async(param: {
    rankId: string; name: string; timestamp: number; duration: number;
}): Promise<any> => {
    return window.requestData('unit/one/kernelDetail', param, 'timeline');
};

export const Loading = ({ size = 20, style = {} }: {size?: number;style?: object}): JSX.Element => {
    return (<div className={'loading'}
        style={{ width: `${size}px`, height: `${size}px`, ...style }}></div>);
};

// eslint-disable-next-line max-lines-per-function
export const getColumnSearchProps = ({ dataIndex, setSearchText, searchText, setSearchedColumn, searchedColumn }: SearchData): ColumnType<any> => {
    const handleSearch = (
        selectedKeys: string[],
        confirm: (param?: FilterConfirmProps) => void,
        index: string,
    ): void => {
        confirm();
        setSearchText(selectedKeys[0]);
        setSearchedColumn(index);
    };

    const handleReset = (clearFilters: (() => void) | undefined,
        confirm: (param?: FilterConfirmProps) => void,
        index: string,
    ): void => {
        setSearchText('');
        clearFilters?.();
        setSearchedColumn(index);
        confirm();
    };
    return ({
        filterDropdown: ({
            setSelectedKeys,
            selectedKeys,
            confirm,
            clearFilters,
        }) => (
            <div style={{ padding: 8 }} onKeyDown={(e: React.KeyboardEvent<HTMLDivElement>): void => { e.stopPropagation(); }}>
                <Input
                    placeholder="Search by Name"
                    value={selectedKeys[0] !== null && selectedKeys[0] !== undefined ? selectedKeys[0] : ''}
                    onChange={(e: React.ChangeEvent<HTMLInputElement>): void =>
                        setSelectedKeys((e.target.value !== null && e.target.value !== undefined) ? [e.target.value] : [])}
                    onPressEnter={(): void => handleSearch(selectedKeys as string[], confirm, dataIndex)}
                    style={{ width: 168, marginBottom: 8, display: 'block' }}
                />
                <Space>
                    <Button
                        type="primary"
                        onClick={(): void => handleSearch(selectedKeys as string[], confirm, dataIndex)}
                        icon={<SearchOutlined/>}
                        size="small"
                        style={{ width: 80 }}
                    >
                        Search
                    </Button>
                    <Button
                        onClick={(): void => { if (clearFilters) { handleReset(clearFilters, confirm, dataIndex); } }}
                        size="small"
                        style={{ width: 80 }}
                    >
                        Reset
                    </Button>
                </Space>
            </div>
        ),
        filterIcon: (filtered: boolean) => (
            <SearchOutlined style={{ color: filtered ? '#1890ff' : undefined }}/>
        ),
        onFilter: (value, record) =>
            record[dataIndex]
                .toString()
                .toLowerCase()
                .includes((value as string).toLowerCase()),
        render: (text: string) => (text),
    });
};
