/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
*/
import React, { useEffect, useState } from 'react';
import { CloseCircleOutlined } from '@ant-design/icons';
import { range } from 'lodash';
import { observer } from 'mobx-react';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import { queryCacheRecord, type CacheRecordItem } from '../RequestUtils';
import styled from '@emotion/styled';
import { type Session } from '../../entity/session';
import { safeStr } from 'ascend-utils';
import * as echarts from 'echarts';
import { type Theme, useTheme } from '@emotion/react';

const ChartContainer = styled.div`
    width: 620px;
    height: 600px;
    padding: 50px;
    position: relative;
    display: inline-block;
    .title {
        text-align: center;
        font-weight: bolder;
    }
    .mapTitle {
        position: relative;
        width: 520px;
        bottom: 50px;
        text-align: end;
    }
    .chart {
        width: 520px;
        height: 500px;
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
        z-index: 10;
    }
    .chartTitle {
        text-align: center;
        padding: 20px;
        font-size: 20px;
        font-weight: bolder;
        position: absolute;
        width: 100%;
    }
    .mapTitle {
        position: relative;
        width: 1100px;
        bottom: 50px;
        margin: auto;
        text-align: end;
    }
`;

const CacheKitChartsContainer = styled.div`
    display: grid;
    grid-template-columns: repeat(auto-fill, 620px);
    width: 100%;
    justify-content: center;
`;

interface ChartDataItem {
    yAxisNum: number;
    data: any[];
};

const baseOption = (yAxis: number[], data: any[], isPreview: boolean, t: TFunction, theme: Theme): any => {
    return {
        animation: false,
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
            animation: false,
            min: 0,
            max: 100,
            calculable: false,
            orient: 'vertical',
            align: 'left',
            right: '30px',
            top: '50px',
            textStyle: {
                color: theme.textColorSecondary,
            },
            itemHeight: isPreview ? '870px' : '370px',
            color: ['red', 'yellow', 'lightGreen', theme.bgColorGrey],
        },
        series: {
            type: 'heatmap',
            animation: false,
            progressive: 0,
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

const initCharts = (dataSource: ChartDataItem, domId: string, isPreview: boolean, t: TFunction, theme: Theme): void => {
    const chartDom = document.getElementById(domId);
    if (chartDom !== null) {
        echarts.getInstanceByDom(chartDom)?.dispose();
        const myChart = echarts.init(chartDom);
        const { data, yAxisNum } = dataSource;
        const options = baseOption(range(0, yAxisNum), data, isPreview, t, theme);
        myChart.setOption(options);
    }
};

const PreviewCacheKitChart = ({ setShowPreChart, preChartData, title }:
{ setShowPreChart: (val: boolean) => void; preChartData: ChartDataItem; title: string }): JSX.Element => {
    const { t } = useTranslation('source');
    const theme = useTheme();
    useEffect(() => {
        initCharts(preChartData, 'previewCacheKitChart', true, t, theme);
    }, []);
    useEffect(() => {
        initCharts(preChartData, 'previewCacheKitChart', true, t, theme);
    }, [theme]);
    return <PreviewCacheKitChartContainer>
        <CloseCircleOutlined className="closePreview" onClick={(): void => setShowPreChart(false)} />
        <div className="chartTitle">{title}</div>
        <div id="previewCacheKitChart" style={{ width: '1100px', height: '1000px', margin: 'auto' }}></div>
        <div className="mapTitle">{`${t('EventRatio')}(%)`}</div>
    </PreviewCacheKitChartContainer>;
};

const CacheKitChartBase = ({ session, title, data, handleClick }:
{ session: Session; title: string; data: ChartDataItem; handleClick: (val: ChartDataItem, title: string) => void }): JSX.Element => {
    const { t } = useTranslation('source');
    const theme = useTheme();
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
        initCharts(data, domId, false, t, theme);
    }, [data, theme]);
    return <ChartContainer>
        <div className="title">{titleMap[title]}</div>
        <div id={domId} className="chart" onClick={(): void => handleClick(data, titleMap[title])} />
        <div className="mapTitle">{`${t('EventRatio')}(%)`}</div>
    </ChartContainer>;
};

const CacheKitChart = observer(({ session }: { session: Session }): JSX.Element => {
    const [chartsData, setChartsData] = useState({} as { [key: string]: ChartDataItem });
    const [showPreChart, setShowPreChart] = useState(false);
    const [preChartData, setPreChartData] = useState({} as ChartDataItem);
    const [preChartDataTitle, setPreChartDataTitle] = useState('');
    const handleClick = (chartData: ChartDataItem, chartTitle: string): void => {
        setPreChartData(chartData);
        setPreChartDataTitle(chartTitle);
        setShowPreChart(true);
    };
    useEffect(() => {
        getchartsData().then((data) => {
            setShowPreChart(false);
            setChartsData(data);
        });
    }, [session.updateId]);
    return <>
        <CacheKitChartsContainer style={{ display: showPreChart ? 'none' : 'grid' }}>
            {
                Object.keys(chartsData).map(key => (
                    <CacheKitChartBase key={key} title={key} session={session} data={chartsData[key]} handleClick={handleClick} />
                ))
            }
        </CacheKitChartsContainer>
        {showPreChart && <PreviewCacheKitChart preChartData={preChartData} setShowPreChart={setShowPreChart} title={preChartDataTitle} />}
    </>;
});

export default CacheKitChart;
