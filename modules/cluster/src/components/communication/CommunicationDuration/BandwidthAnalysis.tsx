/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { observer } from 'mobx-react-lite';
import React, { useEffect, useState } from 'react';
import { createRoot } from 'react-dom/client';
import { useTranslation } from 'react-i18next';
import type { PlainLegendComponentOption } from 'echarts';
import * as echarts from 'echarts';
import { CollapsiblePanel, Empty } from '@insight/lib/components';
import { ResizeTable } from '@insight/lib/resize';
import type { ColumnsType } from 'antd/es/table';
import type { CategoryAxisBaseOption } from 'echarts/types/src/coord/axisCommonTypes.js';
import { COLOR, commonEchartsOptions } from '../../Common';
import i18n from '@insight/lib/i18n';
import { cloneDeep, merge } from 'lodash';
import {
    chartColors,
    customConsole as console,
    disposeAdaptiveEchart,
    getAdaptiveEchart,
    getDefaultChartOptions,
    safeJSONParse,
} from '@insight/lib/utils';
import styled from '@emotion/styled';
import { useTheme } from '@emotion/react';
import { queryCommunicationBandwidth, queryCommunicationDistribution } from '../../../utils/RequestUtils';
import { PacketAndBandwidthChartsParams, WrapBandwidthDataParams } from '../../../utils/interface';

const ChartsContainer = styled.div`
  display: flex;
  align-items: center;
  gap: 24px;

  .chart-item{
    flex: 1;
    padding: 16px 24px;
    border: 1px solid ${(props): string => props.theme.borderColor};

      .chart-title{
          margin-bottom: 10px;
          font-size: 16px;
      }

      .chart{
          display: flex;
          height: 400px;
      }
  }
`;

const chartDomIds = ['HCCS', 'PCIE', 'RDMA', 'SIO'];

type BandwidthElementProps = Omit<WrapBandwidthDataParams, 'isDark' | 'domId'>;

const BandwidthTable: React.FC<BandwidthElementProps> = (props: BandwidthElementProps) => {
    const [data, setData] = useState([]);
    useEffect(() => {
        updateData();
    }, []);

    const updateData = async(): Promise<void> => {
        const result = await getTableData(props);
        const newData = wrapData(result ?? []);
        setData(newData);
    };
    return (
        <>
            <ResizeTable
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
    if (sdma) { sdma.largePacketRatio = '/'; };
    const sio = data.find((item: any) => item.transportType === 'SIO');
    let hp = data.filter((item: any) => item.transportType === 'HCCS' || item.transportType === 'PCIE');
    if (sio !== undefined) {
        hp = data.filter((item: any) => item.transportType === 'HCCS' || item.transportType === 'PCIE' || item.transportType === 'SIO');
    }
    const rdma = data.find((item: any) => item.transportType === 'RDMA');
    return rdma !== undefined ? [{ ...sdma, children: hp }, rdma] : [{ ...sdma, children: hp }];
}

const BandwidthChart: React.FC<BandwidthElementProps> = (props: BandwidthElementProps) => {
    const { iterationId, rankId, dbPath, operatorName, stage, pgName, groupIdHash } = props;
    const { t } = useTranslation('communication');
    const theme = useTheme();
    const isDark = theme.mode === 'dark';
    const locale = i18n.language?.slice(0, 2);

    useEffect(() => {
        const params = { iterationId, rankId, dbPath, operatorName, stage, isDark, locale, pgName, groupIdHash };
        InitPacketAndBandwidthCharts({ ...params, domId: 'HCCS' });
        InitPacketAndBandwidthCharts({ ...params, domId: 'PCIE' });
        InitPacketAndBandwidthCharts({ ...params, domId: 'RDMA' });
        InitPacketAndBandwidthCharts({ ...params, domId: 'SIO' });
    }, [t, theme]);
    return (
        <ChartsContainer>
            <div className={'chart-item'}>
                <div className={'chart-title'}>HCCS</div>
                <div id={'HCCS'} className={'chart'} />
            </div>

            <div className={'chart-item'}>
                <div className={'chart-title'}>PCIE</div>
                <div id={'PCIE'} className={'chart'} />
            </div>

            <div className={'chart-item'}>
                <div className={'chart-title'}>RDMA</div>
                <div id={'RDMA'} className={'chart'} />
            </div>

            <div className={'chart-item'}>
                <div className={'chart-title'}>SIO</div>
                <div id={'SIO'} className={'chart'} />
            </div>
        </ChartsContainer>
    );
};

const BandwidthAnalysis = observer((props:
{ iterationId: string; rankId: number; dbPath: string; operatorName: string; stage: string; pgName: string; groupIdHash: string }) => {
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

async function getTableData(params: BandwidthElementProps): Promise<any> {
    return await queryCommunicationBandwidth(params);
}

async function getChartData(params: PacketAndBandwidthChartsParams): Promise<any> {
    return await queryCommunicationDistribution(params);
}

async function InitPacketAndBandwidthCharts({
    domId,
    iterationId,
    rankId,
    dbPath,
    operatorName,
    stage,
    isDark,
    locale,
    pgName,
    groupIdHash,
}: PacketAndBandwidthChartsParams): Promise<void> {
    const chartDom = document.getElementById(domId);
    if (chartDom !== null) {
        const res = await wrapBandwidthData({ domId, iterationId, rankId, dbPath, operatorName, stage, isDark, pgName, groupIdHash });
        if (res === null || res === undefined) {
            const root = createRoot(chartDom);
            root.render(<Empty style={{ margin: 'auto' }} image={Empty.PRESENTED_IMAGE_SIMPLE}/>);
        } else {
            disposeAdaptiveEchart(chartDom);
            const myChart = getAdaptiveEchart(chartDom, null, { locale });
            myChart.setOption(res);
        }
    }
}

const translateList = <T extends Record<string, any>>(list: T[]): T[] => {
    return list.map((item: any) => ({
        ...item,
        name: i18n.t(item.name, { ns: 'communication' }),
    }));
};

async function wrapBandwidthData({ domId, iterationId, rankId, dbPath, operatorName, stage, isDark, pgName, groupIdHash }: WrapBandwidthDataParams):
Promise<echarts.EChartsOption | null> {
    const distributionData = await getChartData({ domId, iterationId, rankId, dbPath, operatorName, stage, pgName, groupIdHash } as PacketAndBandwidthChartsParams);
    const packetSizeData: number[] = [];
    const packetNumberData: number[] = [];
    const packetBandwidthData: number[] = [];
    const bandwidthOptionClone = cloneDeep(bandwidthOption);
    // 如果返回内容为空字符串，说明数据库中不存在数据，此时，对应的带宽图做隐藏处理
    if (distributionData === '') {
        const chartDom = document.getElementById(domId);
        const chartParentDom = chartDom?.parentElement;
        if (!chartParentDom) { return null; }
        (chartParentDom as HTMLElement).style.display = 'none';
        chartDomIds.forEach(id => {
            const dom = document.getElementById(id);
            if (dom && id !== domId) {
                echarts.getInstanceByDom(dom)?.resize();
            }
        });
    }

    if (['{}', null, undefined, 'null', ''].includes(distributionData)) {
        return null;
    }
    const distributionDataJson: Distribution = safeJSONParse(distributionData, {});
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
            packetBandwidthData.push(Number((packetSizeNumber * packetNumber / 1000 / (durationTime / 1000)).toFixed(4)));
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
    merge(bandwidthOptionClone.toolbox, getDefaultChartOptions(isDark).toolbox);

    return bandwidthOptionClone;
}

export interface Distribution {
    [packetSize: string]: number [];
}

const bandwidthOption: echarts.EChartsOption = {
    textStyle: getDefaultChartOptions().textStyle,
    color: chartColors,
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
            { name: 'Packet Number', textStyle: { color: COLOR.GREY_50 } },
            { name: 'Bandwidth(GB/s)', textStyle: { color: COLOR.GREY_50 } },
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
            color: COLOR.GREY_40,
        },
    },
    yAxis: [
        {
            type: 'value',
            minInterval: 1,
            name: 'Packet Number',
            axisLabel: {
                formatter: '{value}',
                color: COLOR.GREY_40,
                width: 60,
                overflow: 'break',
            },
        },
        {
            type: 'value',
            name: 'Bandwidth(GB/s)',
            axisLabel: {
                formatter: '{value} GB/s',
                color: COLOR.GREY_40,
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
