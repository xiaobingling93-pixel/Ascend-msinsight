/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { observer } from 'mobx-react-lite';
import React, { useEffect, useState } from 'react';
import { Session } from '../../entity/session';
import * as echarts from 'echarts';
import { Col, Layout, Row, Table } from 'antd';
import type { ColumnsType } from 'antd/es/table';
// eslint-disable-next-line import/no-unresolved
import { CategoryAxisBaseOption } from 'echarts/types/src/coord/axisCommonTypes';
import { Container } from './Common';

const BandwidthTable: React.FC = (props: any) => {
    const [ data, setData ] = useState([]);
    useEffect(() => {
        updateData();
    }, []);

    const updateData = async(): Promise<void> => {
        const result = await getTableData();
        const data = wrapData(result);
        setData(data);
    };
    return (
        <>
            <Table
                pagination={false}
                expandable={{ defaultExpandedRowKeys: ['SDMA'] }}
                columns={columns}
                dataSource={data}
                rowKey={'transport_type'}
            />
        </>
    );
};

function wrapData(data: any): any {
    const sdma = data.find((item: any) => item.transport_type === 'SDMA');
    const hp = data.filter((item: any) => item.transport_type === 'HCCS' || item.transport_type === 'PCIe');
    const rdma = data.find((item: any) => item.transport_type === 'RDMA');
    return [ { ...sdma, children: hp }, rdma ];
}

const BandwidthChart: React.FC = (props: any) => {
    useEffect(() => {
        InitPacketAndBandwidthCharts('HCCS');
        InitPacketAndBandwidthCharts('PCIe');
        InitPacketAndBandwidthCharts('RDMA');
    }, []);
    return (
        <div className={'bandwidthChart'}>
            <Row wrap={false}>
                <Col span={8}>
                    <div>
                        <div style={{ margin: '20px', fontSize: ' 2rem' }}>HCCS</div>
                        <div id={'HCCS'} style={{ height: '400px', width: '100%', display: 'inline-block' }}/>
                    </div>
                </Col>
                <Col span={8}>
                    <div>
                        <div style={{ margin: '20px', fontSize: ' 2rem' }}>PCIe</div>
                        <div id={'PCIe'} style={{ height: '400px', width: '100%', display: 'inline-block' }}/>
                    </div>
                </Col>
                <Col span={8}>
                    <div>
                        <div style={{ margin: '20px', fontSize: ' 2rem' }}>RDMA</div>
                        <div id={'RDMA'} style={{ height: '400px', width: '100%', display: 'inline-block' }}/>
                    </div>
                </Col>
            </Row>
        </div>
    );
};

const BandwidthAnalysis = observer(function ({ session, rankId, operatorName }:
{ session: Session;rankId: number;operatorName: string }) {
    return (
        <Layout>
            <Container
                title={'Packet Distribution'}
                content={ <BandwidthChart/>}
            />
            <Container
                title={'Bandwidth Analysis'}
                content={ <BandwidthTable/> }
            />
        </Layout>
    );
});

async function getTableData (): Promise<any> {
    const bandwidthDetails = await window.request('communication/duration/bandwidth',
        { iterationId: 1, rankId: 1, operatorName: 'hcom_allReduce__5' });
    return bandwidthDetails.bandwidthData;
}

async function getChartData (): Promise<any> {
    const distributions = await window.request('communication/duration/distribution',
        { iterationId: 1, rankId: 1, operatorName: 'hcom_allReduce__5', transportType: 'HCCS' });
    return distributions.distributionData[0].size_distribution;
}

async function InitPacketAndBandwidthCharts(domId: string): Promise<void> {
    const chartDom = document.getElementById(domId);
    if (chartDom !== null) {
        const myChart = echarts.init(chartDom);
        myChart.setOption(await wrapBandwidthData(domId));
    }
}

async function wrapBandwidthData(domId: string): Promise<echarts.EChartsOption> {
    const distributionData = await getChartData();
    const packetSizeData: number[] = [];
    const packetNumberData: number[] = [];
    const packetBandwidthData: number[] = [];
    if (distributionData.length === 0) {
        return {};
    }
    const distributionDataJson: Distribution = JSON.parse(distributionData);
    for (const [ packetSize, values ] of Object.entries(distributionDataJson)) {
        if (values.length !== 2) {
            console.error('The format of distribution data is error');
            return {};
        }
        packetSizeData.push(Number(Number(packetSize).toFixed(4)));
        packetNumberData.push(values[0]);
        packetBandwidthData.push(Number(values[1].toFixed(4)));
    }
    (bandwidthOption.xAxis as CategoryAxisBaseOption).data = packetSizeData;
    (bandwidthOption.series as echarts.SeriesOption[])[0].data = packetNumberData;
    (bandwidthOption.series as echarts.SeriesOption[])[1].data = packetBandwidthData;
    return bandwidthOption;
}

export interface Distribution {
    [packetSize: string]: number [];
}

const bandwidthOption: echarts.EChartsOption = {
    tooltip: {
        trigger: 'axis',
        axisPointer: {
            type: 'cross',
            crossStyle: {
                color: '#999',
            },
        },
    },
    toolbox: {
        feature: {
            dataView: { show: true, readOnly: false },
            magicType: { show: true, type: [ 'line', 'bar' ] },
            restore: { show: true },
            saveAsImage: { show: true },
        },
    },
    legend: {
        data: [ 'Packet Number', 'Bandwidth(GB/s)' ],
    },
    xAxis: {
        type: 'category',
        name: 'Packet Size(MB)',
        nameLocation: 'middle',
        nameGap: 40,
        data: [],
        axisPointer: {
            type: 'shadow',
        },
    },
    yAxis: [
        {
            type: 'value',
            minInterval: 1,
            name: 'Packet Number',
            axisLabel: {
                formatter: '{value}',
            },
        },
        {
            type: 'value',
            name: 'Bandwidth(GB/s)',
            axisLabel: {
                formatter: '{value} GB/s',
            },
        },
    ],
    series: [
        {
            name: 'Packet Number',
            type: 'bar',
            tooltip: {
                valueFormatter: function (value) {
                    return `${value}`;
                },
            },
            data: [],
        },
        {
            name: 'Bandwidth(GB/s)',
            type: 'line',
            yAxisIndex: 1,
            tooltip: {
                valueFormatter: function (value) {
                    return `${value}` + ' GB/s';
                },
            },
            data: [],
        },
    ],
};

interface DataType {
    key: React.ReactNode;
    TransportType: string;
    TransitSize: number;
    TransitTime: number;
    Bandwidth: number;
    BandwidthUtilization: number;
    LargePacketRatio: number;
    children?: DataType[];
}

const columns: ColumnsType<DataType> = [
    {
        title: 'Transport Type',
        dataIndex: 'transport_type',
        key: 'TransportType',
    },
    {
        title: 'Transit Size(MB)',
        dataIndex: 'transit_size',
        key: 'TransitSize',
    },
    {
        title: 'Transit Time(ms)',
        dataIndex: 'transit_time',
        key: 'TransitTime',
    },
    {
        title: 'Bandwidth(GB/s) ',
        dataIndex: 'bandwidth_size',
        key: 'Bandwidth',
    },
    {
        title: 'Bandwidth(Utilization)',
        dataIndex: 'bandwidth_utilization',
        key: 'BandwidthUtilization',
    },
    {
        title: 'Large Packet Ratio',
        dataIndex: 'large_package_ratio',
        key: 'LargePacketRatio',
    },
];
export default BandwidthAnalysis;
