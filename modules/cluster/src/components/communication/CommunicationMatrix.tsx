/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
*/

import { observer } from 'mobx-react-lite';
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import { Select, Checkbox, InputNumber, Button } from 'ascend-components';
import { message } from 'antd';
import type { CheckboxChangeEvent } from 'antd/es/checkbox';
import * as echarts from 'echarts';
import { addResizeEvent, Label, COLOR, getDecimalCount, safeStr } from '../Common';
import type { ConditionDataType } from './Filter';
import type { optionDataType, VoidFunction } from '../../utils/interface';
import { queryCommunicationMatrix, queryRanks } from '../../utils/RequestUtils';
import _, { cloneDeep } from 'lodash';
import { type Session } from '../../entity/session';
import CollapsiblePanel from 'ascend-collapsible-panel';

interface FilterInfos {
    min: number;
    max: number;
}

interface RangeInfo {
    minRange: number;
    maxRange: number;
}

interface ICommunicationMatrixProps {
    isShow: boolean;
    handleChange: VoidFunction;
    switchCondition: any;
    range: RangeInfo;
    setFilter: VoidFunction;
}

const useOptions = (): optionDataType[] => {
    const { t } = useTranslation('communication');
    return [
        {
            label: `${t('searchCriteria.Bandwidth')}(GB/s)`,
            value: 'bandwidth',
        },
        {
            label: `${t('searchCriteria.TransitSize')}(MB)`,
            value: 'transitSize',
        },
        {
            label: t('searchCriteria.TransportType'),
            value: 'transportType',
        },
        {
            label: `${t('searchCriteria.TransitTime')}(ms)`,
            value: 'transitTime',
        },
    ];
};

function InitCharts(data: any, t: TFunction): void {
    const chartDom = document.getElementById('matrixchart');
    if (chartDom !== null) {
        echarts.init(chartDom).dispose();
        const myChart = echarts.init(chartDom);
        myChart.setOption(wrapData(data, t));
        addResizeEvent(myChart);
    }
}

function handleRepeatData(repeatDataToolTip: {[key: string]: string}, data: any[][]): any[][] {
    return data.reduce((prev: any[][], cur: any[]) => {
        const repeatItem = prev.find((item) => {
            return item[0] === cur[0] && item[1] === cur[1];
        });
        if (repeatItem) {
            const repeadKey = `${cur[0]},${cur[1]}`;
            if (repeadKey in repeatDataToolTip) {
                repeatDataToolTip[repeadKey] = `${repeatDataToolTip[repeadKey]},${cur[2]}`;
            } else {
                repeatDataToolTip[repeadKey] = `${cur[2]}`;
            }
            repeatItem[2] = cur[2];
            return prev;
        } else {
            return [...prev, cur];
        }
    }, []);
}

function handleTransportType(dataSource: any, option: any, type: any, t: TFunction): void {
    dataSource.forEach((item: any[]) => {
        item[2] = allTransporType.indexOf(item[2]);
    });
    option.series[0].data = dataSource;
    option.visualMap = transportTypeOption.visualMap;
    option.series[0].label.formatter = (params: any): string => {
        return allTransporType[params.value[2]];
    };
    option.series[0].tooltip.formatter = function (params: any): string {
        const data = params?.data;
        const str = data?.[3]?.length === 0
            ? `srcRank -> dstRank: ${data?.[0]} -> ${data?.[1]} <br/> ${t(type)}: ${allTransporType[data?.[2]]}`
            : `${t('operatorName')}: ${data?.[3]} <br/> srcRank -> dstRank: ${data?.[0]} -> ${data?.[1]} <br/> ${t(type)}: ${allTransporType[data?.[2]]}`;
        return safeStr(str, '<br/>');
    };
}

const allTransporType = ['HCCS', 'PCIE', 'RDMA', 'LOCAL', 'SIO'];
const defaultVisualMap = {
    calculable: true,
    orient: 'horizontal',
    left: 'center',
    bottom: '0',
    inRange: { color: [COLOR.BAND_0, COLOR.BAND_1, COLOR.BAND_2, COLOR.BAND_3] },
    textStyle: { color: COLOR.GREY_40 },
    dimension: 2,
    itemHeight: 300, // 调整宽度
};
function wrapData(dataSource: any, t: TFunction): any {
    const { data, rankIds, type, min, max } = dataSource;
    const option: any = baseOption;
    option.xAxis.data = rankIds;
    option.yAxis.data = rankIds;
    option.series[0].label.show = rankIds.length <= 16;

    if (type === 'transportType') {
        handleTransportType(data, option, type, t);
    } else {
        const repeatDataToolTip: {[key: string]: string} = {};
        const mixData = handleRepeatData(repeatDataToolTip, data);
        option.series[0].data = mixData;
        option.visualMap = cloneDeep(defaultVisualMap);
        if (data.length > 0 || isFinite(max)) {
            option.visualMap.max = max;
            option.visualMap.min = min;
            if (min === max) {
                option.visualMap.inRange = { color: [COLOR.BAND_1] };
            }
            const minDecimalCount = getDecimalCount(min);
            const maxDecimalCount = getDecimalCount(max);
            option.visualMap.precision = Math.max(minDecimalCount, maxDecimalCount);
        }
        option.series[0].label.formatter = function (params: any): string {
            const newData = params?.data;
            if (newData.length < 3) {
                return '';
            }
            const repeatedKey = `${newData[0]},${newData[1]}`;
            if (repeatedKey in repeatDataToolTip && repeatDataToolTip[repeatedKey]) {
                newData[2] = `[${newData[2]},${repeatDataToolTip[repeatedKey]}]`;
                repeatDataToolTip[repeatedKey] = '';
            }
            return newData[2];
        };
        option.series[0].tooltip.formatter = function (params: any): string {
            const datalist = params?.data;
            const str = datalist?.[3]?.length === 0
                ? `srcRank -> dstRank: ${datalist?.[0]} -> ${datalist?.[1]} <br/> ${t(type)}: ${datalist?.[2]}`
                : `${t('operatorName')}: ${datalist?.[3]} <br/> srcRank -> dstRank: ${datalist?.[0]} -> ${datalist?.[1]} <br/>${t(type)}: ${datalist?.[2]}`;
            return safeStr(str, '<br/>');
        };
    }
    return option;
}

const baseOption: any = {
    dataZoom: [
        {
            type: 'inside',
            xAxisIndex: [0],
            start: 0,
            end: 100,
        },
        {
            type: 'inside',
            yAxisIndex: [0],
            start: 0,
            end: 100,
        },
        {
            type: 'inside',
            start: 0,
            end: 100,
        },
    ],
    tooltip: {
        position: 'top',
    },
    grid: {
        left: '100',
        right: '100',
        height: '80%',
        top: '10%',
    },
    xAxis: {
        type: 'category',
        name: 'Src Rank Id',
        data: [],
        splitArea: {
            show: true,
        },
    },
    yAxis: {
        type: 'category',
        name: 'Dst Rank Id',
        data: [],
    },
    visualMap: {
        min: 0,
        max: 10,
        calculable: true,
        orient: 'horizontal',
        left: 'center',
        bottom: '0',
        inRange: { color: [COLOR.BAND_0, COLOR.BAND_1, COLOR.BAND_2, COLOR.BAND_3] },
        textStyle: { color: COLOR.GREY_40 },
        dimension: 2,
    },
    series: [
        {
            type: 'heatmap',
            tooltip: {
                show: true,
                formatter: function (params: any): string {
                    const { data } = params;
                    return `${data[0]} -> ${data[1]} : ${data[2]}`;
                },
            },
            data: [],
            label: {
                show: true,
            },
            emphasis: {
                itemStyle: {
                    shadowBlur: 10,
                    shadowColor: COLOR.GREY_50,
                },
            },
        },
    ],
};

const transportTypeOption = {
    visualMap: {
        type: 'piecewise',
        calculable: true,
        orient: 'horizontal',
        left: 'center',
        bottom: '0',
        splitNumber: 1,
        pieces: [
            { value: 0, label: allTransporType[0], color: COLOR.BAND_0 },
            { value: 1, label: allTransporType[1], color: COLOR.BAND_1 },
            { value: 2, label: allTransporType[2], color: COLOR.BAND_2 },
            { value: 3, label: allTransporType[3], color: COLOR.BAND_3 },
            { value: 4, label: allTransporType[4], color: COLOR.BAND_4 },
        ],
        textStyle: { color: COLOR.GREY_40 },
        dimension: 2,
    },
};

const updateData = async(conditions: ConditionDataType, callback: VoidFunction): Promise<void> => {
    const param = { iterationId: conditions.iterationId, stage: conditions.stage, operatorName: conditions.operatorName };
    const res = await queryCommunicationMatrix(param);
    const data = res?.matrixList ?? [];
    const rankRes: {iterationOrRankId: string[] } =
        await queryRanks({ iterationId: conditions.iterationId });
    const iterationOrRankId = rankRes?.iterationOrRankId ?? [];
    const stageRanks = _.map(_.split(_.replace(conditions.stage, /[(),]/, ''), ','),
        value => Number.parseInt(value)).filter(value => !Number.isNaN(value))
        .sort((a, b) => a - b);
    let rankIds = iterationOrRankId.map(item => String(item));
    if (stageRanks.length > 0) {
        rankIds = _.filter(rankIds, value => Number(value) >= stageRanks[0] && Number(value) <= stageRanks[stageRanks.length - 1]);
    }
    rankIds.sort((a, b) => Number(a) - Number(b));
    callback({ data, rankIds });
};

const CommunicationMatrix = observer(({ isShow, conditions, session }: { isShow: boolean;conditions: ConditionDataType;session: Session}) => {
    const [dataSource, setDataSource] = useState<{data: any[];rankIds: any[]}>({ data: [], rankIds: [] });
    const [switchCondition, setSwitchCondition] = useState({ type: 'bandwidth', showInner: false });
    const [range, setRange] = useState<RangeInfo>({ minRange: 0, maxRange: 1 });
    const { t } = useTranslation('communication');

    const updateCharts = (shouldUpdateRange: boolean, filterInfo?: FilterInfos): void => {
        let data: any = dataSource.data.map((item: any) => {
            return [String(item.srcRank), String(item.dstRank),
                item[switchCondition.type] !== undefined ? item[switchCondition.type] : null, item.opName];
        });
        if (!switchCondition.showInner) {
            data = data.filter((item: any[]) => item[0] !== item[1]);
        }
        data = data.filter((item: any[]) => dataSource.rankIds.includes(item[0]) && dataSource.rankIds.includes(item[1]));
        if (filterInfo) {
            data = data.filter((item: any[]) =>
                typeof item[2] === 'number' && item[2] >= filterInfo.min && item[2] <= filterInfo.max);
        }
        const min = data.length > 0 ? Math.min(...data.map((item: number[]) => item[2])) : 0;
        const max = data.length > 0 ? Math.max(...data.map((item: number[]) => item[2])) : 0;
        if (shouldUpdateRange) {
            setRange({ minRange: min, maxRange: max });
        }
        InitCharts({ ...dataSource, data, type: switchCondition.type, min: filterInfo?.min ?? min, max: filterInfo?.max ?? max }, t);
    };
    useEffect(() => {
        if (isShow) {
            if (session.clusterCompleted) {
                updateData(conditions, setDataSource);
            } else {
                setDataSource({ data: [], rankIds: [] });
            }
        }
    }, [isShow, conditions]);
    useEffect(() => {
        updateCharts(true);
    }, [dataSource, switchCondition]);
    const handleChange = (filed: string, val: string | boolean): void => {
        setSwitchCondition({ ...switchCondition, [filed]: val });
    };
    const handleFilterChange = (data: FilterInfos): void => {
        updateCharts(false, data);
    };

    return <CommunicationMatrixCom isShow={isShow} handleChange={handleChange} switchCondition={switchCondition} range={range} setFilter={handleFilterChange}/>;
});

const RangeFilter = ({ range, changeFilter }: { range: RangeInfo; changeFilter: VoidFunction }): JSX.Element => {
    const { minRange, maxRange } = range;
    const [minValue, setMin] = useState(minRange);
    const [maxValue, setMax] = useState(maxRange);
    const [isValid, setIsValid] = useState(true);
    const { t } = useTranslation();

    const onConfirm = (): void => {
        if (minValue > maxValue) {
            message.warning('Invalid Range: The start value cannot be greater than the end value.');
            setIsValid(false);
            return;
        }
        changeFilter({ min: minValue, max: maxValue });
    };
    const changeInput = (value: number, type: string): void => {
        setIsValid(true);
        if (type === 'min') {
            setMin(value);
        } else {
            setMax(value);
        }
    };
    useEffect(() => {
        setMin(minRange);
        setMax(maxRange);
    }, [JSON.stringify(range)]);
    return (
        <>
            <Label name={t('searchCriteria.VisibleRange', { ns: 'communication' })} style={{ margin: '0 8px 0 24px' }} />
            <InputNumber value={minValue} size="small" style={{ marginRight: 8 }} min={minRange} max={maxRange}
                onChange={(value: string | number | null): void => changeInput(value as number, 'min')} status={isValid ? '' : 'error'} step={0.1} />
            ~
            <InputNumber value={maxValue} size="small" style={{ margin: '0 32px 0 8px' }} min={minRange} max={maxRange}
                onChange={(value: string | number | null): void => changeInput(value as number, 'max')} status={isValid ? '' : 'error'} step={0.1} />
            <Button onClick={onConfirm} type="primary" size="middle">{t('Confirm', { ns: 'buttonText' })}</Button>
        </>
    );
};

const CommunicationMatrixCom = ({ isShow, handleChange, switchCondition, range, setFilter }: ICommunicationMatrixProps): JSX.Element => {
    const { t } = useTranslation('communication');
    return <CollapsiblePanel style={{ display: isShow ? 'block' : 'none' }} title={t('sessionTitle.MatrixModel')} padding={'16px 24px'}>
        <div>
            <Label name={t('searchCriteria.CommunicationMatrixType')}/>
            <Select
                defaultValue="0"
                style={{ width: 200, marginRight: '20px' }}
                onChange={(val: string): void => {
                    handleChange('type', val);
                }}
                options={useOptions()}
                value={switchCondition.type}
            />
            <Checkbox checked={switchCondition.showInner}
                onChange={(e: CheckboxChangeEvent): void => {
                    handleChange('showInner', e.target.checked);
                }}
            >{t('searchCriteria.ShowInnerCommunication')}</Checkbox>
            {switchCondition.type !== 'transportType' && <RangeFilter range={range} changeFilter={setFilter} />}
        </div>
        <div>
            <div id={'matrixchart'} style={{ width: 'calc(100vw - 80px)', height: '800px' }}></div>
        </div>
    </CollapsiblePanel>;
};

export default CommunicationMatrix;
