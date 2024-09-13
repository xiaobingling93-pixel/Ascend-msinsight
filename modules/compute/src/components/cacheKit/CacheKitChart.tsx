/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
*/
import React, { useEffect, useState } from 'react';
import { CloseCircleOutlined } from '@ant-design/icons';
import * as echarts from 'echarts';
import { range } from 'lodash';
import { observer } from 'mobx-react';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import { addResizeEvent } from 'ascend-utils/EchartUtils';
import { queryCacheRecord, type CacheRecordItem } from '../RequestUtils';
import styled from '@emotion/styled';
import { type Session } from '../../entity/session';
import { safeStr } from 'ascend-utils';

const ChartContainer = styled.div`
    width: 620px;
    height: 600px;
    padding: 50px;
    position: relative;
    .title {
        text-align: center;
    }
    .mapTitle {
        position: relative;
        display: inline-block;
        bottom: 50px;
        left: 400px;
    }
    .chart {
        width: 100%;
        height: 100%;
        position: relative;
        poimter-events: none;
        &::before {
            content: "";
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            poimter-events: auto;
            z-index: 10;
            cursor: pointer;
        }
    }
`;

const PreviewCacheKitChartContainer = styled.div`
    background: ${(props): string => props.theme.bgColor};
    width: 100%;
    height: 100%;
    position: absolute;
    top: 0;
    left: 0;
    .closePreview {
        position: absolute;
        top: 10px;
        right: 10px;
    }
    .mapTitle {
        position: relative;
        display: inline-block;
        bottom: 50px;
        left: 1400px;
    }
`;

interface ChartDataItem {
    yAxisNum: number;
    data: any[];
};

const baseOption = (yAxis: number[], data: any[], isPreview: boolean, t: TFunction): any => {
    return {
        tooltip: {
            position: 'top',
            formatter: (item: { [key: string]: any }): string => {
                return `${t('EventNumber')}：${safeStr(item.data[2])}<br/>${t('EventRatio')}：${safeStr(item.data[3])} %`;
            },
        },
        grid: {
            right: '100px',
        },
        xAxis: {
            type: 'category',
            data: range(0, 128),
        },
        yAxis: {
            type: 'category',
            data: yAxis,
        },
        visualMap: {
            min: 0,
            max: 100,
            calculable: true,
            orient: 'vertical',
            align: 'left',
            right: '20px',
            top: '50px',
            itemHeight: isPreview ? '870px' : '370px',
            color: ['red', 'yellow', 'lightGreen', 'lightBlue', 'blue'],
        },
        series: {
            type: 'heatmap',
            data,
            emphasis: {
                itemStyle: {
                    shadowBlur: 10,
                    shadowColor: 'rgba(0, 0, 0, 0.5)',
                },
            },
        },
    };
};

const getchartsData = async (): Promise<{ [key: string]: ChartDataItem } > => {
    let cacheRecords: CacheRecordItem[] = [];
    try {
        cacheRecords = (await queryCacheRecord() ?? { cacheRecords: [] }).cacheRecords;
    } catch (err) {
        // 请求异常，用初始空值
    }
    const chartsData: { [key: string]: ChartDataItem } = {};
    const chartkeys = ['loadCount', 'storeCount', 'hit', 'miss', 'allocate', 'evictAndWrite', 'evictWithoutWrite'];
    chartkeys.forEach(key => {
        chartsData[key] = {
            yAxisNum: 0,
            data: [],
        };
    });
    const yAxisNum = cacheRecords.length / 128;
    for (let i = 0; i < cacheRecords.length; i++) {
        for (let j = 0; j < chartkeys.length; j++) {
            const [eventNumber, eventRatio] = cacheRecords[i][chartkeys[j]];
            const ratio = Number((eventRatio * 100).toFixed(1));
            chartsData[chartkeys[j]].yAxisNum = yAxisNum;
            chartsData[chartkeys[j]].data.push([
                i % 128,
                Math.floor(i / 128),
                eventNumber,
                ratio,
            ]);
        }
    }
    return chartsData;
};

const initCharts = (dataSource: ChartDataItem, domId: string, isPreview: boolean, t: TFunction): void => {
    const chartDom = document.getElementById(domId);
    if (chartDom !== null) {
        echarts.init(chartDom).dispose();
        const myChart = echarts.init(chartDom);
        const { data, yAxisNum } = dataSource;
        const options = baseOption(range(0, yAxisNum), data, isPreview, t);
        myChart.setOption(options);
        addResizeEvent(myChart);
    }
};

const PreviewCacheKitChart = ({ setShowPreChart, preChartData }:
{ setShowPreChart: (val: boolean) => void; preChartData: ChartDataItem }): JSX.Element => {
    const { t } = useTranslation('source');
    useEffect(() => {
        initCharts(preChartData, 'previewCacheKitChart', true, t);
    }, []);
    return <PreviewCacheKitChartContainer>
        <CloseCircleOutlined className="closePreview" onClick={(): void => setShowPreChart(false)}/>
        <div id="previewCacheKitChart" style={{ width: '1100px', height: '1000px', margin: 'auto' }}></div>
        <div className="mapTitle">{`${t('EventRatio')}(%)`}</div>
    </PreviewCacheKitChartContainer>;
};

const CacheKitChartBase = ({ session, title, data, handleClick }:
{ session: Session; title: string; data: ChartDataItem; handleClick: (val: ChartDataItem) => void }): JSX.Element => {
    const { t } = useTranslation('source');
    const titleMap: { [key: string]: string } = {
        loadCount: 'Load Count',
        storeCount: 'Store Count',
        hit: 'Hit',
        miss: 'Miss',
        allocate: 'Allocate',
        evictAndWrite: 'Evict And Write',
        evictWithoutWrite: 'Evict Without Write',
    };
    const domId = `cacheChart_${title}`;
    useEffect(() => {
        initCharts(data, domId, false, t);
    }, [data]);
    return <ChartContainer>
        <div className="title">{titleMap[title]}</div>
        <div id={domId} className="chart" onClick={(): void => handleClick(data)}/>
        <div className="mapTitle">{`${t('EventRatio')}(%)`}</div>
    </ChartContainer>;
};

const CacheKitChart = observer(({ session }: { session: Session }): JSX.Element => {
    const [chartsData, setChartsData] = useState({} as { [key: string]: ChartDataItem });
    const [showPreChart, setShowPreChart] = useState(false);
    const [preChartData, setPreChartData] = useState({} as ChartDataItem);
    const handleClick = (chartData: ChartDataItem): void => {
        setPreChartData(chartData);
        setShowPreChart(true);
    };
    useEffect(() => {
        getchartsData().then((data) => {
            setShowPreChart(false);
            setChartsData(data);
        });
    }, [session.updateId]);
    return <>
        <div style={{ display: showPreChart ? 'none' : 'flex', flexWrap: 'wrap', width: '1860px' }}>
            {
                Object.keys(chartsData).map(key => (
                    <CacheKitChartBase key={key} title={key} session={session} data={chartsData[key]} handleClick={handleClick}/>
                ))
            }
        </div>
        {showPreChart && <PreviewCacheKitChart preChartData={preChartData} setShowPreChart={setShowPreChart}/>}
    </>;
});

export default CacheKitChart;
