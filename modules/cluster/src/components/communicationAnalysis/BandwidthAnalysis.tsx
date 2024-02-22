/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { observer } from 'mobx-react-lite';
import React, { useEffect, useState } from 'react';
import ReactDOM from 'react-dom';
import * as echarts from 'echarts';
import { Col, Layout, Row, Table, Empty } from 'antd';
import type { ColumnsType } from 'antd/es/table';
import type { CategoryAxisBaseOption } from 'echarts/types/src/coord/axisCommonTypes';
import { Container, addResizeEvent, COLOR } from '../Common';

const BandwidthTable: React.FC<{ iterationId: string; rankId: number; operatorName: string }> = (props: any) => {
    const [data, setData] = useState([]);
    useEffect(() => {
        updateData();
    }, []);

    const updateData = async(): Promise<void> => {
        const result = await getTableData(props.iterationId, props.rankId, props.operatorName, props.stage);
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
                rowKey={'transportType'}
            />
        </>
    );
};

function wrapData(data: any): any {
    data.forEach((item: any) => {
        if (item.largePacketRatio === null || item.largePacketRatio === undefined) {
            item.largePacketRatio = '/';
        }
    });
    const sdma = data.find((item: any) => item.transportType === 'SDMA');
    sdma.largePacketRatio = '/';
    const hp = data.filter((item: any) => item.transportType === 'HCCS' || item.transportType === 'PCIE');
    const rdma = data.find((item: any) => item.transportType === 'RDMA');
    return [{ ...sdma, children: hp }, rdma];
}

const BandwidthChart: React.FC<{ iterationId: string; rankId: number; operatorName: string;
    stage: string; }> = (props: any) => {
    useEffect(() => {
        InitPacketAndBandwidthCharts('HCCS', props.iterationId, props.rankId, props.operatorName, props.stage);
        InitPacketAndBandwidthCharts('PCIE', props.iterationId, props.rankId, props.operatorName, props.stage);
        InitPacketAndBandwidthCharts('RDMA', props.iterationId, props.rankId, props.operatorName, props.stage);
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
{ iterationId: string; rankId: number; operatorName: string; stage: string }) {
    return (
        <Layout>
            <Container
                style={{ minWidth: '1000px' }}
                title={'Packet Distribution'}
                content={ <BandwidthChart {...props}/>}
            />
            <Container
                style={{ minWidth: '1000px' }}
                title={'Bandwidth Analysis'}
                content={ <BandwidthTable {...props}/> }
            />
        </Layout>
    );
});

async function getTableData (iterationId: number, rankId: number, operatorName: string, stage: string): Promise<any> {
    const bandwidthDetails = await window.requestData('communication/bandwidth',
        { iterationId, rankId, operatorName, stage });
    return bandwidthDetails.items;
}

async function getChartData (domId: string, iterationId: number, rankId: number,
    operatorName: string, stage: string): Promise<any> {
    const distributions = await window.requestData('communication/distribution',
        { iterationId, rankId, operatorName, transportType: domId, stage });
    if (distributions.distributionData === undefined) {
        return '{}';
    }
    return distributions.distributionData;
}

async function InitPacketAndBandwidthCharts(domId: string, iterationId: number,
    rankId: number, operatorName: string, stage: string): Promise<void> {
    const chartDom = document.getElementById(domId);
    if (chartDom !== null) {
        const res = await wrapBandwidthData(domId, iterationId, rankId, operatorName, stage);
        if (res === null) {
            ReactDOM.render((<Empty image={Empty.PRESENTED_IMAGE_SIMPLE}/>), chartDom);
        } else {
            const myChart = echarts.init(chartDom);
            myChart.setOption(res);
            addResizeEvent(myChart);
        }
    }
}

async function wrapBandwidthData(domId: string, iterationId: number,
    rankId: number, operatorName: string, stage: string): Promise<echarts.EChartsOption | null> {
    const distributionData = await getChartData(domId, iterationId, rankId, operatorName, stage);
    const packetSizeData: number[] = [];
    const packetNumberData: number[] = [];
    const packetBandwidthData: number[] = [];
    if (distributionData === '{}' || distributionData === null ||
        distributionData === undefined || distributionData === 'null') {
        return null;
    }
    const distributionDataJson: Distribution = JSON.parse(distributionData);
    for (const [packetSize, values] of Object.entries(distributionDataJson)
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
                color: COLOR.BrightBlue,
                type: 'solid',
            },
        },
        confine: true,
    },
    toolbox: {
        feature: {
            dataView: { show: true, readOnly: false },
            magicType: { show: true, type: ['line', 'bar'] },
            restore: { show: true },
        },
        top: 15,
    },
    legend: {
        data: [
            { name: 'Packet Number', textStyle: { color: COLOR.Grey50 } },
            { name: 'Bandwidth(GB/s)', textStyle: { color: COLOR.Grey50 } },
        ],
        tooltip: {
            show: true,
            formatter: function () {
                const div = document.createElement('div');
                div.className = 'legend-tooltip';
                div.append('Click to Switch Chart Display and Hide');
                return div;
            },
        },
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
        axisLabel: {
            color: COLOR.Grey40,
        },
    },
    yAxis: [
        {
            type: 'value',
            minInterval: 1,
            name: 'Packet Number',
            axisLabel: {
                formatter: '{value}',
                color: COLOR.Grey40,
                width: 60,
                overflow: 'break',
            },
        },
        {
            type: 'value',
            name: 'Bandwidth(GB/s)',
            axisLabel: {
                formatter: '{value} GB/s',
                color: COLOR.Grey40,
                width: 85,
                overflow: 'break',
            },
            splitLine: {
                lineStyle: {
                    color: COLOR.Grey20,
                    type: 'dashed',
                },
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
    grid: {
        left: 65,
        right: 90,
        top: 80,
    },
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
        dataIndex: 'transportType',
        key: 'TransportType',
        align: 'center',
        ellipsis: true,
    },
    {
        title: 'Transit Size(MB)',
        dataIndex: 'transitSize',
        key: 'TransitSize',
        align: 'center',
        ellipsis: true,
    },
    {
        title: 'Transit Time(ms)',
        dataIndex: 'transitTime',
        key: 'TransitTime',
        align: 'center',
        ellipsis: true,
    },
    {
        title: 'Bandwidth(GB/s) ',
        dataIndex: 'bandwidth',
        key: 'Bandwidth',
        align: 'center',
        ellipsis: true,
    },
    {
        title: 'Large Packet Ratio',
        dataIndex: 'largePacketRatio',
        key: 'LargePacketRatio',
        align: 'center',
        ellipsis: true,
    },
];
export default BandwidthAnalysis;
