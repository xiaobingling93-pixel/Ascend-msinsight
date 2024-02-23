import { observer } from 'mobx-react-lite';
import React, { useEffect, useState } from 'react';
import { Select, Checkbox } from 'antd';
import type { CheckboxChangeEvent } from 'antd/es/checkbox';
import * as echarts from 'echarts';
import { addResizeEvent, Container, Label, COLOR, getDecimalCount } from '../Common';
import { ConditionDataType } from './Filter';
import { optionDataType, VoidFunction } from '../../utils/interface';
import { queryCommunicationMatrix, queryRanks } from '../../utils/RequestUtils';
import _ from 'lodash';

const options: optionDataType[] = [
    {
        label: 'Bandwidth(GB/s)',
        value: 'bandwidth',
    },
    {
        label: 'Transit Size(MB)',
        value: 'transitSize',
    },
    {
        label: 'Transport Type',
        value: 'transportType',
    },
    {
        label: 'Transit Time(ms)',
        value: 'transitTime',
    },
];

function InitCharts(data: any): void {
    const chartDom = document.getElementById('matrixchart');
    if (chartDom !== null) {
        echarts.init(chartDom).dispose();
        const myChart = echarts.init(chartDom);
        myChart.setOption(wrapData(data));
        addResizeEvent(myChart);
    }
}

const allTransporType = ['HCCS', 'PCIE', 'RDMA', 'LOCAL'];
// eslint-disable-next-line max-lines-per-function
function wrapData(dataSource: any): any {
    const { data, rankIds, type } = dataSource;
    const option: any = baseOption;
    option.xAxis.data = rankIds;
    option.yAxis.data = rankIds;
    if (rankIds.length > 16) {
        option.series[0].label.show = false;
    }

    if (type === 'transportType') {
        data.forEach((item: any[]) => { item[2] = allTransporType.indexOf(item[2]); });
        option.series[0].data = data;
        option.visualMap = transportTypeOption.visualMap;
        option.series[0].label.formatter = (params: any) => {
            return allTransporType[params.value[2]];
        };
        option.series[0].tooltip.formatter = function (params: any) {
            const { data } = params;
            return data[3].length === 0
                ? `srcRank -> dstRank: ${data[0]} -> ${data[1]} <br/> ${type}: ${allTransporType[data[2]]}`
                : `operatorName: ${data[3]} <br/> srcRank -> dstRank: ${data[0]} -> ${data[1]} <br/> ${type}: ${allTransporType[data[2]]}`;
        };
    } else {
        option.series[0].data = data;
        option.visualMap = {
            calculable: true,
            orient: 'horizontal',
            left: 'center',
            bottom: '15%',
            inRange: { color: [COLOR.Band0, COLOR.Band1, COLOR.Band2, COLOR.Band3] },
            textStyle: { color: COLOR.Grey40 },
        };
        if (data.length > 0) {
            const max = Math.max(...data.map((item: number[]) => item[2]));
            const min = Math.min(...data.map((item: number[]) => item[2]));
            option.visualMap.max = max;
            option.visualMap.min = min;
            if (min === max) {
                option.visualMap.inRange = { color: [COLOR.Band1] };
            }
            const minDecimalCount = getDecimalCount(min);
            const maxDecimalCount = getDecimalCount(max);
            option.visualMap.precision = Math.max(minDecimalCount, maxDecimalCount);
        }
        option.series[0].tooltip.formatter = function (params: any) {
            const { data } = params;
            return data[3].length === 0
                ? `srcRank -> dstRank: ${data[0]} -> ${data[1]} <br/> ${type}: ${data[2]}`
                : `operatorName: ${data[3]} <br/> srcRank -> dstRank: ${data[0]} -> ${data[1]} <br/> ${type}: ${data[2]}`;
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
        height: '50%',
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
        splitArea: {
            show: true,
        },
    },
    visualMap: {
        min: 0,
        max: 10,
        calculable: true,
        orient: 'horizontal',
        left: 'center',
        bottom: '15%',
        inRange: { color: [COLOR.Band0, COLOR.Band1, COLOR.Band2, COLOR.Band3] },
        textStyle: { color: COLOR.Grey40 },
    },
    series: [
        {
            type: 'heatmap',
            tooltip: {
                show: true,
                formatter: function (params: any) {
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
                    shadowColor: COLOR.Grey50,
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
        bottom: '15%',
        splitNumber: 1,
        pieces: [
            { value: 0, label: allTransporType[0], color: COLOR.Band0 },
            { value: 1, label: allTransporType[1], color: COLOR.Band1 },
            { value: 2, label: allTransporType[2], color: COLOR.Band2 },
            { value: 3, label: allTransporType[3], color: COLOR.Band3 },
        ],
        textStyle: { color: COLOR.Grey40 },
    },
};

const CommunicationMatrix = observer(function ({ isShow, conditions }: { isShow: boolean;conditions: ConditionDataType}) {
    const [dataSource, setDataSource] = useState<{data: any[];rankIds: any[]}>({ data: [], rankIds: [] });
    const [switchCondition, setSwitchCondition] = useState({ type: 'bandwidth', showInner: false });
    useEffect(() => {
        if (isShow) {
            updateData(conditions);
        }
    }, [isShow, conditions]);
    useEffect(() => {
        if (isShow) {
            let data: any = dataSource.data.map((item: any) => {
                return [String(item.srcRank), String(item.dstRank),
                    item[switchCondition.type] !== undefined ? item[switchCondition.type] : null, item.opName];
            });
            if (!switchCondition.showInner) {
                data = data.filter((item: any[]) => item[0] !== item[1]);
            }
            data = data.filter((item: any[]) =>
                dataSource.rankIds.includes(item[0]) && dataSource.rankIds.includes(item[1]));
            InitCharts({ ...dataSource, data, type: switchCondition.type });
        }
    }, [dataSource, switchCondition]);
    const updateData = async(conditions: ConditionDataType): Promise<void> => {
        const param = { iterationId: conditions.iterationId, stage: conditions.stage, operatorName: conditions.operatorName };
        const res = await queryCommunicationMatrix(param);
        const data = res.matrixList ?? [];
        const rankRes: {iterationOrRankId: string[] } =
            await queryRanks({ iterationId: conditions.iterationId });
        const stageRanks = _.map(_.split(_.replace(conditions.stage, /[(),]/, ''), ' '),
            value => Number.parseInt(value)).filter(value => !Number.isNaN(value))
            .sort((a, b) => a - b);
        let rankIds = rankRes.iterationOrRankId.map(item => String(item));
        if (stageRanks.length > 0) {
            rankIds = _.filter(rankIds, value => Number(value) >= stageRanks[0] && Number(value) <= stageRanks[stageRanks.length - 1]);
        }
        rankIds.sort((a, b) => Number(a) - Number(b));
        setDataSource({ data, rankIds });
    };
    const handleChange = (filed: string, val: string | boolean): void => {
        setSwitchCondition({ ...switchCondition, [filed]: val });
    };
    return <CommunicationMatrixCom isShow={isShow} handleChange={handleChange} switchCondition={switchCondition}/>;
});

const CommunicationMatrixCom = ({ isShow, handleChange, switchCondition }:
{isShow: boolean;handleChange: VoidFunction;switchCondition: any}): JSX.Element => {
    return (<div style={{ display: isShow ? 'block' : 'none', overflow: 'auto' }}>
        <Container
            type={'headerfixed'}
            title={'Matrix Model'}
            style={{ margin: '0 20px' }}
            content={<div>
                <div>
                    <Label name={'Communication Matrix Type'}/>
                    <Select
                        defaultValue="0"
                        style={{ width: 200, marginRight: '20px' }}
                        onChange={val => {
                            handleChange('type', val);
                        }}
                        options={options}
                        value={switchCondition.type}
                    />
                    <Checkbox checked={switchCondition.showInner}
                        onChange={(e: CheckboxChangeEvent): void => {
                            handleChange('showInner', e.target.checked);
                        }}
                    >Show Inner Communication</Checkbox>
                </div>
                <div>
                    <div id={'matrixchart'} style={{ height: '800px' }}></div>
                </div>
            </div>}
        />
    </div>
    );
};

export default CommunicationMatrix;
