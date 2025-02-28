/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/
import React, { useEffect, useState } from 'react';
import { CloseCircleOutlined } from '@ant-design/icons';
import { range } from 'lodash';
import { observer } from 'mobx-react';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import type { CacheEventType, CacheRecordItem } from './defs';
import styled from '@emotion/styled';
import { type Session } from '../../entity/session';
import { safeStr } from 'ascend-utils';
import * as echarts from 'echarts';
import { type Theme, useTheme } from '@emotion/react';
import { queryCacheRecord } from '../RequestUtils';
import { swtich2Source } from '../../connection/sendNotification';
import { CACHELINE_RECORD, CACHELINE_ID, ADDRESS_RANGE } from './defs';
import { store } from '../../store';

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

enum DataIndex {
    INDEX = 2,
    CACHELINE_ID_NUM = 3,
    EVENT_NUM = 4,
    RATIO = 5,
}

interface IOptionParams {
    yAxis: number[];
    data: any[];
    isPreview: boolean;
    t: TFunction;
    theme: Theme;
    name: string;
}

const baseSet = {
    confine: true,
    animation: false,
    grid: {
        right: '100px',
    },
    xAxis: {
        type: 'category',
        data: range(0, 128),
    },
};

const visualMapBaseSet = {
    animation: false,
    min: 0,
    max: 100,
    calculable: false,
    orient: 'vertical',
    align: 'left',
    right: '30px',
    top: '50px',
};

const heatmapBaseSet = {
    type: 'heatmap',
    animation: false,
    progressive: 0,
    emphasis: {
        itemStyle: {
            shadowBlur: 10,
            shadowColor: 'rgba(0, 0, 0, 0.5)',
        },
    },
};

const getBaseOption = (params: IOptionParams): any => {
    const { yAxis, data, isPreview, t, theme, name } = params;
    return {
        ...baseSet,
        tooltip: {
            position: (point: number[]): string => point[1] < 100 ? 'bottom' : 'top',
            formatter: (item: { [key: string]: any }): string => {
                return `${t('Cacheline Id')}：${safeStr(item.data[DataIndex.CACHELINE_ID_NUM])}<br/>
${t('EventNumber')}：${safeStr(item.data[DataIndex.EVENT_NUM])}<br/>
${t('EventRatio')}：${safeStr(item.data[DataIndex.RATIO])} %`;
            },
        },
        yAxis: {
            type: 'category',
            data: yAxis,
        },
        visualMap: {
            ...visualMapBaseSet,
            textStyle: {
                color: theme.textColorSecondary,
            },
            itemHeight: isPreview ? '870px' : '370px',
            color: ['red', 'yellow', 'lightGreen', theme.bgColorGrey],
        },
        series: {
            ...heatmapBaseSet,
            data,
            name,
        },
    };
};

let allCacheRecords: CacheRecordItem[] = [];
const getchartsData = async (): Promise<{ chartsData: Record<string, ChartDataItem> ;cacheRecords: CacheRecordItem[]} > => {
    let cacheRecords: CacheRecordItem[] = [];
    try {
        cacheRecords = (await queryCacheRecord() ?? { [CACHELINE_RECORD]: [] })?.[CACHELINE_RECORD] ?? [];
    } catch (err) {
        // 请求异常，用初始空值
    }
    const chartsData: { [key: string]: ChartDataItem } = {};
    const chartkeys: CacheEventType[] = ['Hit', 'Miss'];
    chartkeys.forEach(key => {
        chartsData[key] = {
            yAxisNum: 0,
            data: [],
        };
    });
    const yAxisNum = cacheRecords.length / 128;
    for (let i = 0; i < cacheRecords.length; i++) {
        const cachelineId = cacheRecords[i][CACHELINE_ID];
        for (let j = 0; j < chartkeys.length; j++) {
            const [eventNumber, eventRatio] = cacheRecords[i][chartkeys[j]].Value;
            const ratio = Number((eventRatio * 100).toFixed(1));
            chartsData[chartkeys[j]].yAxisNum = yAxisNum;
            chartsData[chartkeys[j]].data.push(
                [
                    i % 128,
                    Math.floor(i / 128),
                    i,
                    cachelineId,
                    eventNumber,
                    ratio,
                ],
            );
        }
    }
    return { chartsData, cacheRecords };
};

interface IParams {
    domId: string;
    dataSource: ChartDataItem;
    isPreview: boolean;
    t: TFunction;
    theme: Theme;
    name: string;
}

const initCharts = (params: IParams): void => {
    const { domId, dataSource, isPreview, t, theme, name } = params;
    const chartDom = document.getElementById(domId);
    if (chartDom !== null) {
        echarts.getInstanceByDom(chartDom)?.dispose();
        const myChart = echarts.init(chartDom);
        const { data, yAxisNum } = dataSource;
        const options = getBaseOption({ yAxis: range(0, yAxisNum), data, isPreview, t, theme, name });
        myChart.setOption(options);
        myChart.on('click', showInstructions);
    }
};

export function showInstructions(params: any): void {
    const session = store.sessionStore.activeSession;
    if (!session) {
        return;
    }
    // 如果没有source信息
    if (session.coreList.length === 0) {
        return;
    }
    const { data, seriesName } = params;
    const record = allCacheRecords[data[DataIndex.INDEX]];
    if (record?.[seriesName]?.[ADDRESS_RANGE]?.length > 0) {
        swtich2Source({ addressRange: record[seriesName][ADDRESS_RANGE], cachelineId: record[CACHELINE_ID] });
    }
}

const PreviewCacheKitChart = ({ setShowPreChart, preChartData, title }:
{ setShowPreChart: (val: boolean) => void; preChartData: ChartDataItem; title: string }): JSX.Element => {
    const { t } = useTranslation('source');
    const theme = useTheme();
    const params = {
        domId: 'previewCacheKitChart',
        dataSource: preChartData,
        isPreview: true,
        t,
        theme,
        name: title,
    };
    useEffect(() => {
        initCharts(params);
    }, []);
    useEffect(() => {
        initCharts(params);
    }, [theme]);
    return <PreviewCacheKitChartContainer>
        <CloseCircleOutlined className="closePreview" onClick={(): void => setShowPreChart(false)} />
        <div className="chartTitle">{t(title)}</div>
        <div id="previewCacheKitChart" style={{ width: '1100px', height: '1000px', margin: 'auto' }}></div>
        <div className="mapTitle">{`${t('EventRatio')}(%)`}</div>
    </PreviewCacheKitChartContainer>;
};

const CacheKitChartBase = ({ session, title, data, handleClick }:
{ session: Session; title: string; data: ChartDataItem; handleClick: (val: ChartDataItem, title: string) => void }): JSX.Element => {
    const { t } = useTranslation('source');
    const theme = useTheme();
    const domId = `cacheChart_${title}`;
    useEffect(() => {
        initCharts({ dataSource: data, domId, isPreview: false, t, theme, name: title });
    }, [data, theme]);
    return <ChartContainer>
        <div className="title">{t(title)}</div>
        <div id={domId} className="chart" onClick={(): void => handleClick(data, title) } />
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
            setChartsData(data.chartsData);
            allCacheRecords = data.cacheRecords;
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
