/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { Table } from 'antd';
import React from 'react';
import { getPageConfigWithAllData } from '../Common';

const columns = [
    {
        title: 'Rank ID',
        dataIndex: 'rankId',
    },
    {
        title: 'Computing',
        dataIndex: 'computingTime',
    },
    {
        title: 'Communication(Not Overlapped)',
        dataIndex: 'communicationNotOverLappedTime',
    },
    {
        title: 'Communication(Overlapped)',
        dataIndex: 'communicationOverLappedTime',
    },
    {
        title: 'Free',
        dataIndex: 'freeTime',
    },
    {
        title: 'Computing Time Ratio',
        dataIndex: 'ComputingTimeRatio',
    },
    {
        title: 'Communication Time Ratio',
        dataIndex: 'CommunicationTimeRatio',
    },
];
const SummaryTable = (props: any): JSX.Element => {
    const { dataSource = [], style = {} } = props;
    return (<Table
        style={style}
        dataSource={dataSource}
        columns={columns}
        pagination={getPageConfigWithAllData(dataSource.length)}
        size="small"
    />);
};
export default SummaryTable;
