/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { observer } from 'mobx-react-lite';
import React, { useEffect, useState } from 'react';
import ReactDOM from 'react-dom';
import { useTranslation } from 'react-i18next';
import * as echarts from 'echarts';
import type { PlainLegendComponentOption } from 'echarts';
import { Col, Row, Table, Empty } from 'antd';
import type { ColumnsType } from 'antd/es/table';
import type { CategoryAxisBaseOption } from 'echarts/types/src/coord/axisCommonTypes';
import { addResizeEvent, COLOR, commonEchartsOptions } from '../Common';
import i18n from 'lib/i18n';
import { cloneDeep } from 'lodash';
import { CustomConsole as console } from 'lib/CommonUtils';
import CollapsiblePanel from 'lib/CollapsiblePanel';

const BandwidthTable: React.FC<{ iterationId: string; rankId: number; operatorName: string }> = (props: any) => {
    const [data, setData] = useState([]);
    useEffect(() => {
        updateData();
    }, []);

    const updateData = async(): Promise<void> => {
        const result = await getTableData(props.iterationId, props.rankId, props.operatorName, props.stage);
        const newData = wrapData(result ?? []);
        setData(newData);
    };
    return (
        <>
            <Table
                pagination={false}
                expandable={{ defaultExpandedRowKeys: ['SDMA'] }}
                columns={useColumns()}
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
    const sio = data.find((item: any) => item.transportType === 'SIO');
    let hp = data.filter((item: any) => item.transportType === 'HCCS' || item.transportType === 'PCIE');
    if (sio !== undefined) {
        hp = data.filter((item: any) => item.transportType === 'HCCS' || item.transportType === 'PCIE' || item.transportType === 'SIO');
    }
    const rdma = data.find((item: any) => item.transportType === 'RDMA');
    return [{ ...sdma, children: hp }, rdma];
}

const BandwidthChart: React.FC<{ iterationId: string; rankId: number; operatorName: string;
    stage: string; }> = (props: any) => {
    const { t } = useTranslation('communication');
    useEffect(() => {
        InitPacketAndBandwidthCharts('HCCS', props.iterationId, props.rankId, props.operatorName, props.stage);
        InitPacketAndBandwidthCharts('PCIE', props.iterationId, props.rankId, props.operatorName, props.stage);
        InitPacketAndBandwidthCharts('RDMA', props.iterationId, props.rankId, props.operatorName, props.stage);
        InitPacketAndBandwidthCharts('SIO', props.iterationId, props.rankId, props.operatorName, props.stage);
    }, [t]);
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
                <Col span={8}>
                    <div className={'chartDiv'}>
                        <div style={{ margin: '20px', fontSize: ' 2rem' }}>SIO</div>
                        <div id={'SIO'} style={{ height: '400px', width: '100%', display: 'inline-block' }}/>
                    </div>
                </Col>
            </Row>
        </div>
    );
};

const BandwidthAnalysis = observer((props:
{ iterationId: string; rankId: number; operatorName: string; stage: string }) => {
    const { t } = useTranslation('communication');
    return (
        <div>
            <CollapsiblePanel title={t('sessionTitle.PacketDistribution')}>
                <BandwidthChart {...props}/>
            </CollapsiblePanel>
            <CollapsiblePanel title={t('sessionTitle.BandwidthAnalysis')}>
                <BandwidthTable {...props}/>
            </CollapsiblePanel>
        </div>
    );
});

async function getTableData(iterationId: number, rankId: number, operatorName: string, stage: string): Promise<any> {
    const bandwidthDetails = await window.requestData('communication/bandwidth',
        { iterationId, rankId, operatorName, stage });
    return bandwidthDetails?.items ?? [];
}

async function getChartData(domId: string, iterationId: number, rankId: number,
    operatorName: string, stage: string): Promise<any> {
    const distributions = await window.requestData('communication/distribution',
        { iterationId, rankId, operatorName, transportType: domId, stage });
    return distributions?.distributionData ?? '{}';
}

async function InitPacketAndBandwidthCharts(domId: string, iterationId: number,
    rankId: number, operatorName: string, stage: string): Promise<void> {
    const chartDom = document.getElementById(domId);
    if (chartDom !== null) {
        const res = await wrapBandwidthData(domId, iterationId, rankId, operatorName, stage);
        if (res === null || res === undefined) {
            ReactDOM.render((<Empty image={Empty.PRESENTED_IMAGE_SIMPLE}/>), chartDom);
        } else {
            const myChart = echarts.init(chartDom);
            myChart.setOption(res);
            addResizeEvent(myChart);
        }
    }
}

const translateList = <T extends Record<string, any>>(list: T[]): T[] => {
    return list.map((item: any) => ({
        ...item,
        name: i18n.t(item.name, { ns: 'communication' }),
    }));
};

async function wrapBandwidthData(domId: string, iterationId: number,
    rankId: number, operatorName: string, stage: string): Promise<echarts.EChartsOption | null> {
    const distributionData = await getChartData(domId, iterationId, rankId, operatorName, stage);
    const packetSizeData: number[] = [];
    const packetNumberData: number[] = [];
    const packetBandwidthData: number[] = [];
    const bandwidthOptionClone = cloneDeep(bandwidthOption);
    // 如果返回内容为空字符串，说明数据库中不存在数据，此时，对应的带宽图做隐藏处理
    if (distributionData === '') {
        const chartDom = document.getElementById(domId);
        const chartParentDom = chartDom?.parentElement?.parentElement;
        if (chartParentDom) {
            (chartParentDom as HTMLElement).style.display = 'none';
        }
    }

    if (['{}', null, undefined, 'null', ''].includes(distributionData)) {
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

    (bandwidthOptionClone.legend as PlainLegendComponentOption).data = translateList((bandwidthOptionClone.legend as PlainLegendComponentOption).data as []);
    bandwidthOptionClone.series = translateList(bandwidthOptionClone.series as []);
    bandwidthOptionClone.yAxis = translateList(bandwidthOptionClone.yAxis as []);
    (bandwidthOptionClone.xAxis as CategoryAxisBaseOption).name = i18n.t((bandwidthOptionClone.xAxis as CategoryAxisBaseOption).name ?? '', { ns: 'communication' });
    (bandwidthOptionClone.xAxis as CategoryAxisBaseOption).data = packetSizeData;
    (bandwidthOptionClone.series as echarts.SeriesOption[])[0].data = packetNumberData;
    (bandwidthOptionClone.series as echarts.SeriesOption[])[1].data = packetBandwidthData;
    return bandwidthOptionClone;
}

export interface Distribution {
    [packetSize: string]: number [];
}

const bandwidthOption: echarts.EChartsOption = {
    tooltip: {
        ...commonEchartsOptions.tooltip,
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
                div.append(i18n.t('chart:switchTooltip'));
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
            splitLine: commonEchartsOptions.splitLineY,
        },
    ],
    series: [
        {
            name: 'Packet Number',
            type: 'bar',
            tooltip: {
                valueFormatter: function (value): string {
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
                valueFormatter: function (value): string {
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
    transportType: string;
    transitSize: number;
    transitTime: number;
    bandwidth: number;
    bandwidthUtilization: number;
    largePacketRatio: number;
    children?: DataType[];
}

const useColumns = (): ColumnsType<DataType> => {
    const { t } = useTranslation('communication');
    return [
        {
            title: t('searchCriteria.TransportType'),
            dataIndex: 'transportType',
            key: 'TransportType',
            align: 'center',
            ellipsis: true,
        },
        {
            title: `${t('searchCriteria.TransitSize')}(MB)`,
            dataIndex: 'transitSize',
            key: 'TransitSize',
            align: 'center',
            ellipsis: true,
        },
        {
            title: `${t('searchCriteria.TransitTime')}(ms)`,
            dataIndex: 'transitTime',
            key: 'TransitTime',
            align: 'center',
            ellipsis: true,
        },
        {
            title: `${t('searchCriteria.Bandwidth')}(GB/s)`,
            dataIndex: 'bandwidth',
            key: 'Bandwidth',
            align: 'center',
            ellipsis: true,
        },
        {
            title: t('tableHead.Large Packet Ratio'),
            dataIndex: 'largePacketRatio',
            key: 'LargePacketRatio',
            align: 'center',
            ellipsis: true,
        },
    ];
};
export default BandwidthAnalysis;
