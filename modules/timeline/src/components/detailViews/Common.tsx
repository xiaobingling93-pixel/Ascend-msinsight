/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/

import type { CompareFn } from 'antd/es/table/interface';
import React from 'react';

interface ColumData {
    title: string;
    dataIndex: string;
    key: string;
    sorter?: boolean | CompareFn<any>;
    showSorterTooltip?: boolean;
}

const getDefaultColumData = (key: string): {} => {
    return {
        sorter: true,
        showSorterTooltip: false,
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
    { title: 'Name', dataIndex: 'name', key: 'name', ...getDefaultColumData('name') },
    { title: 'Time(%)', dataIndex: 'time', key: 'time', ...getDefaultColumData('time') },
    { title: 'Total Time(us)', dataIndex: 'totalTime', key: 'totalTime', ...getDefaultColumData('totalTime') },
    { title: 'Num Calls', dataIndex: 'numberCalls', key: 'numberCalls', ...getDefaultColumData('numberCalls') },
    { title: 'Avg(us)', dataIndex: 'avg', key: 'avg', ...getDefaultColumData('avg') },
    { title: 'Min(us)', dataIndex: 'min', key: 'min', ...getDefaultColumData('min') },
    { title: 'Max(us)', dataIndex: 'max', key: 'max', ...getDefaultColumData('max') },
];

export const kernelDetails: ColumData[] = [
    { title: 'Name', dataIndex: 'name', key: 'name', ...getDefaultColumData('name') },
    { title: 'Type', dataIndex: 'type', key: 'type', ...getDefaultColumData('type') },
    { title: 'Accelerator Core', dataIndex: 'acceleratorCore', key: 'acceleratorCore', ...getDefaultColumData('acceleratorCore') },
    { title: 'Start Time', dataIndex: 'startTime', key: 'startTime', ...getDefaultColumData('startTime') },
    { title: 'Duration(us)', dataIndex: 'duration', key: 'duration', ...getDefaultColumData('duration') },
    { title: 'Wait Time(us)', dataIndex: 'waitTime', key: 'waitTime', ...getDefaultColumData('waitTime') },
    { title: 'Block Dim', dataIndex: 'blockDim', key: 'blockDim', ...getDefaultColumData('blockDim') },
    { title: 'Input Shapes', dataIndex: 'inputShapes', key: 'inputShapes', ...getDefaultColumData('inputShapes') },
    { title: 'Input Data Types', dataIndex: 'inputDataTypes', key: 'inputDataTypes', ...getDefaultColumData('inputDataTypes') },
    { title: 'Input Formats', dataIndex: 'inputFormats', key: 'inputFormats', ...getDefaultColumData('inputFormats') },
    { title: 'Output Shapes', dataIndex: 'outputShapes', key: 'outputShapes', ...getDefaultColumData('outputShapes') },
    { title: 'Output Data Types', dataIndex: 'outputDataTypes', key: 'outputDataTypes', ...getDefaultColumData('outputDataTypes') },
    { title: 'Output Formats', dataIndex: 'outputFormats', key: 'outputFormats', ...getDefaultColumData('outputFormats') },

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
        pageSizeOptions: [ 10, 20, 50, 100 ],
        showTotal: (total: number) => (<div style={{ marginRight: '10px' }}>Total {total} items</div>),
        hideOnSinglePage: false,
        onChange: (current: number, pageSize: number) => { setPage({ ...page, current, pageSize }); },
        showQuickJumper: page.total / page.pageSize > 5,
    };
};

export const querySystemViewDetails = async(param: {
    isQueryTotal: boolean; rankId: string; pageSize: number; current: number; orderBy: string; order: string;
    layer: string;
}): Promise<any> => {
    return window.requestData('unit/systemView', param, 'timeline');
};

export const queryKernelDetails = async(param: {
    rankId: string; pageSize: number; current: number; orderBy: string; order: string;
    coreType: string;
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
