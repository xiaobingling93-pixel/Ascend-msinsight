/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { observer } from 'mobx-react-lite';
import React, { useEffect, useState } from 'react';
import ReactDOM from 'react-dom';
import * as echarts from 'echarts';
import { Col, Layout, Row, Table, Empty } from 'antd';
import type { ColumnsType } from 'antd/es/table';
// eslint-disable-next-line import/no-unresolved
import { CategoryAxisBaseOption } from 'echarts/types/src/coord/axisCommonTypes';
import { Container } from '../Common';

const BandwidthTable: React.FC<{ iterationId: string; rankId: number; operatorName: string }> = (props: any) => {
    const [ data, setData ] = useState([]);
    useEffect(() => {
        updateData();
    }, []);

    const updateData = async(): Promise<void> => {
        const result = await getTableData(props.iterationId, props.rankId, props.operatorName);
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
    sdma.large_package_ratio = '/';
    const hp = data.filter((item: any) => item.transport_type === 'HCCS' || item.transport_type === 'PCIE');
    const rdma = data.find((item: any) => item.transport_type === 'RDMA');
    return [ { ...sdma, children: hp }, rdma ];
}

const BandwidthChart: React.FC<{ iterationId: string; rankId: number; operatorName: string }> = (props: any) => {
    useEffect(() => {
        InitPacketAndBandwidthCharts('HCCS', props.iterationId, props.rankId, props.operatorName);
        InitPacketAndBandwidthCharts('PCIE', props.iterationId, props.rankId, props.operatorName);
        InitPacketAndBandwidthCharts('RDMA', props.iterationId, props.rankId, props.operatorName);
    }, []);
    return (
        <div className={'bandwidthChart'}>
            <Row wrap={false}>
                <Col span={8}>
                    <div className={'chartDiv'}>
                        <div style={{ margin: '20px', fontSize: ' 2rem' }}>HCCS</div>
                        <div id={'HCCS'} style={{ height: '400px', width: '100%', display: 'inline-block' }}/>
                    </div>
                </Col>
                <Col span={8}>
                    <div className={'chartDiv'}>
                        <div style={{ margin: '20px', fontSize: ' 2rem' }}>PCIE</div>
                        <div id={'PCIE'} style={{ height: '400px', width: '100%', display: 'inline-block' }}/>
                    </div>
                </Col>
                <Col span={8}>
                    <div className={'chartDiv'}>
                        <div style={{ margin: '20px', fontSize: ' 2rem' }}>RDMA</div>
                        <div id={'RDMA'} style={{ height: '400px', width: '100%', display: 'inline-block' }}/>
                    </div>
                </Col>
            </Row>
        </div>
    );
};

const BandwidthAnalysis = observer(function (props:
{ iterationId: string; rankId: number; operatorName: string }) {
    return (
        <Layout>
            <Container
                title={'Packet Distribution'}
                content={ <BandwidthChart {...props}/>}
            />
            <Container
                title={'Bandwidth Analysis'}
                content={ <BandwidthTable {...props}/> }
            />
        </Layout>
    );
});

async function getTableData (iterationId: number, rankId: number, operatorName: string): Promise<any> {
    const bandwidthDetails = await window.request('communication/duration/bandwidth',
        { iterationId, rankId, operatorName });
    return bandwidthDetails.bandwidthData;
}

async function getChartData (domId: string, iterationId: number, rankId: number, operatorName: string): Promise<any> {
    const distributions = await window.request('communication/duration/distribution',
        { iterationId, rankId, operatorName, transportType: domId });
    return distributions.distributionData[0].size_distribution;
}

async function InitPacketAndBandwidthCharts(domId: string, iterationId: number,
    rankId: number, operatorName: string): Promise<void> {
    const chartDom = document.getElementById(domId);
    if (chartDom !== null) {
        const res = await wrapBandwidthData(domId, iterationId, rankId, operatorName);
        if (res === null) {
            ReactDOM.render((<Empty image={Empty.PRESENTED_IMAGE_SIMPLE}/>), chartDom);
        } else {
            const myChart = echarts.init(chartDom);
            myChart.setOption(res);
        }
    }
}

async function wrapBandwidthData(domId: string, iterationId: number,
    rankId: number, operatorName: string): Promise<echarts.EChartsOption | null> {
    const distributionData = await getChartData(domId, iterationId, rankId, operatorName);
    const packetSizeData: number[] = [];
    const packetNumberData: number[] = [];
    const packetBandwidthData: number[] = [];
    if (distributionData === '{}' || distributionData === null || distributionData === undefined) {
        return null;
    }
    const distributionDataJson: Distribution = JSON.parse(distributionData);
    for (const [ packetSize, values ] of Object.entries(distributionDataJson)
        .sort((a, b) => parseFloat(a[0]) - parseFloat(b[0]))) {
        if (values.length !== 2) {
            console.error('The format of distribution data is error');
            return null;
        }
        const packetSizeNumber = Number(packetSize);
        const packetNumber = values[0];
        const durationTime = values[1];
        if (durationTime === 0.0) {
            packetBandwidthData.push(0);
        } else {
            packetBandwidthData.push(Number((packetSizeNumber * packetNumber / 1000 / (durationTime / 1000))
                .toFixed(4)));
        }
        packetSizeData.push(Number(packetSizeNumber.toFixed(4)));
        packetNumberData.push(packetNumber);
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
        data: [
            { name: 'Packet Number', textStyle: { color: 'rgb(123,122,122)' } },
            { name: 'Bandwidth(GB/s)', textStyle: { color: 'rgb(123,122,122)' } },
        ],
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
            barMaxWidth: 80,
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
        title: 'Large Packet Ratio',
        dataIndex: 'large_package_ratio',
        key: 'LargePacketRatio',
    },
];
export default BandwidthAnalysis;
