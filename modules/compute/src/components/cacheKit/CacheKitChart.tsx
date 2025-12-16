/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */
import React, { useEffect, useState, useMemo } from 'react';
import { CloseCircleOutlined } from '@ant-design/icons';
import { range } from 'lodash';
import { observer } from 'mobx-react';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import type { CacheEventType, CacheRecordItem } from './defs';
import styled from '@emotion/styled';
import type { CacheUnit, Session } from '../../entity/session';
import { isArray, safeStr } from '@insight/lib/utils';
import * as echarts from 'echarts';
import { type Theme, useTheme } from '@emotion/react';
import { queryCacheRecord } from '../RequestUtils';
import { swtich2Source } from '../../connection/sendNotification';
import { CACHELINE_RECORD, CACHELINE_ID, ADDRESS_RANGE, HIT, MISS } from './defs';
import { Dropdown } from '@insight/lib/components';
import type { MenuProps } from 'antd';
import { observable, runInAction } from 'mobx';

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
        splitArea: {
            show: true,
        },
        axisLabel: {
            showMaxLabel: true,
        },
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
            axisLabel: {
                showMaxLabel: true,
            },
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
const getchartsData = async (parsed: boolean = true): Promise<{ chartsData: Record<string, ChartDataItem> ;cacheRecords: CacheRecordItem[]} > => {
    let cacheRecords: CacheRecordItem[] = [];
    const chartsData: { [key: string]: ChartDataItem } = {};
    const chartkeys: CacheEventType[] = [HIT, MISS];
    chartkeys.forEach(key => {
        chartsData[key] = {
            yAxisNum: 0,
            data: [],
        };
    });
    if (!parsed) {
        return { chartsData, cacheRecords };
    }

    try {
        cacheRecords = (await queryCacheRecord() ?? { [CACHELINE_RECORD]: [] })?.[CACHELINE_RECORD] ?? [];
    } catch (err) {
        // 请求异常，用初始空值
        return { chartsData, cacheRecords };
    }
    const yAxisNum = cacheRecords.length / 128;
    for (let i = 0; i < cacheRecords.length; i++) {
        const cachelineId = cacheRecords[i]?.[CACHELINE_ID];
        for (let j = 0; j < chartkeys.length; j++) {
            const value = cacheRecords[i]?.[chartkeys[j]]?.Value;
            if (!isArray(value)) {
                continue;
            }
            const [eventNumber, eventRatio] = value;
            if (typeof eventNumber !== 'number' || typeof eventRatio !== 'number') {
                continue;
            }
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
    setDropDownVisible?: (val: boolean) => void;
}

const initCharts = (params: IParams): void => {
    const { domId, dataSource, isPreview, t, theme, name, setDropDownVisible } = params;
    const chartDom = document.getElementById(domId);
    if (chartDom !== null) {
        echarts.getInstanceByDom(chartDom)?.dispose();
        const myChart = echarts.init(chartDom);
        const { data, yAxisNum } = dataSource;
        const options = getBaseOption({ yAxis: range(0, yAxisNum), data, isPreview, t, theme, name });
        myChart.setOption(options);
        if (isPreview && setDropDownVisible !== undefined) {
            myChart.on('contextmenu', (e: echarts.ECElementEvent): void => {
                handleContextEvent(e, setDropDownVisible);
            });
        }
    }
};

const selectedCacheUnit = observable<{ value: CacheUnit }>({ value: { cachelineId: -1, addressRange: [] } });
function handleContextEvent(event: echarts.ECElementEvent, setDropDownVisible: (val: boolean) => void): void {
    const { data, seriesName } = event as any;
    const record = allCacheRecords[data[DataIndex.INDEX]];
    runInAction(() => {
        selectedCacheUnit.value = {
            addressRange: record?.[seriesName]?.[ADDRESS_RANGE] ?? [],
            cachelineId: record?.[CACHELINE_ID] ?? -1,
        };
    });
    setDropDownVisible(true);
}

const useMenuItems = (session: Session, t: TFunction): MenuProps['items'] => {
    const menus = useMemo(() => ([{
        label: t('Show Instructions'),
        key: 'showInstructions',
        // 如果没有source信息或Address Range是空数组
        disabled: session.coreList.length === 0 || selectedCacheUnit.value.addressRange.length === 0,
        onClick: (): void => {
            swtich2Source(selectedCacheUnit.value);
        },
    }]), [session.coreList, t, selectedCacheUnit.value]);
    return menus;
};

const PreviewCacheKitChart = observer(({ setShowPreChart, preChartData, title, session }:
{ setShowPreChart: (val: boolean) => void; preChartData: ChartDataItem; title: string ;session: Session}): JSX.Element => {
    const { t } = useTranslation('source');
    const theme = useTheme();
    const [dropDownVisible, setDropDownVisible] = useState(false);
    const menuItems = useMenuItems(session, t);
    const params = {
        domId: 'previewCacheKitChart',
        dataSource: preChartData,
        isPreview: true,
        t,
        theme,
        name: title,
        setDropDownVisible,
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
        <Dropdown
            menu={{
                items: menuItems,
                onClick: (): void => setDropDownVisible(false),
                onBlur: (e: React.FocusEvent<HTMLUListElement, Element>): void => {
                    const hasItem = menuItems?.findIndex(item =>
                        (e.relatedTarget as HTMLElement)?.dataset?.menuId?.includes(item?.key as string)) !== -1;
                    if (!hasItem) {
                        setDropDownVisible(false);
                    }
                },
            }}
            trigger={['contextMenu']}
            open={dropDownVisible}
            autoFocus
        >
            <div id="previewCacheKitChart" style={{ width: '1100px', height: '1000px', margin: 'auto' }}></div>
        </Dropdown>
        <div className="mapTitle">{`${t('EventRatio')}(%)`}</div>
    </PreviewCacheKitChartContainer>;
});

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
        getchartsData(session.parseStatus).then((data) => {
            setShowPreChart(false);
            setChartsData(data.chartsData);
            allCacheRecords = data.cacheRecords;
        });
    }, [session.updateId, session.parseStatus]);
    return <>
        <CacheKitChartsContainer style={{ display: showPreChart ? 'none' : 'grid' }}>
            {
                Object.keys(chartsData).map(key => (
                    <CacheKitChartBase key={key} title={key} session={session} data={chartsData[key]} handleClick={handleClick} />
                ))
            }
        </CacheKitChartsContainer>
        {showPreChart && <PreviewCacheKitChart preChartData={preChartData} setShowPreChart={setShowPreChart} title={preChartDataTitle} session={session}/>}
    </>;
});

export default CacheKitChart;
