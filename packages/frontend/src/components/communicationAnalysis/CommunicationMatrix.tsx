import { observer } from 'mobx-react-lite';
import React, { useEffect, useState } from 'react';
import { Select, Checkbox } from 'antd';
import type { CheckboxChangeEvent } from 'antd/es/checkbox';
import * as echarts from 'echarts';
import { addResizeEvent, Container, Label, COLOR } from '../Common';
import { ConditionDataType } from './Filter';
import { optionDataType, VoidFunction } from '../../utils/interface';
import { queryCommunicationMatrix, queryRanks } from '../../utils/RequestUtils';

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

const allTransporType = [ 'HCCS', 'PCIE', 'RDMA', 'LOCAL' ];
function wrapData(dataSource: any): any {
    const { data, rankIds, type } = dataSource;
    const option: any = baseOption;
    option.xAxis.data = rankIds;
    option.yAxis.data = rankIds;
    if (type === 'transportType') {
        data.forEach((item: any[]) => { item[2] = allTransporType.indexOf(item[2]); });
        option.series[0].data = data;
        option.visualMap = transportTypeOption.visualMap;
        option.series[0].label.formatter = (params: any) => {
            return allTransporType[params.value[2]];
        };
        option.series[0].tooltip.formatter = function (params: any) {
            const { data } = params;
            return `${data[0]} -> ${data[1]} : ${allTransporType[data[2]]}`;
        };
    } else {
        option.series[0].data = data;
        option.visualMap = {
            calculable: true,
            orient: 'horizontal',
            left: 'center',
            bottom: '15%',
            inRange: { color: [ COLOR.Band0, COLOR.Band1, COLOR.Band2, COLOR.Band3 ] },
        };
        if (data.length > 0) {
            const max = Math.max(...data.map((item: number[]) => item[2]));
            const min = Math.min(...data.map((item: number[]) => item[2]));
            option.visualMap.max = max;
            option.visualMap.min = min;
        }

        option.series[0].tooltip.formatter = function (params: any) {
            const { data } = params;
            return `${data[0]} -> ${data[1]} : ${data[2]}`;
        };
    }
    return option;
}

const baseOption: any = {
    tooltip: {
        position: 'top',
    },
    grid: {
        height: '50%',
        top: '10%',
    },
    xAxis: {
        type: 'category',
        name: 'Dst Rank Id',
        data: [ ],
        splitArea: {
            show: true,
        },
    },
    yAxis: {
        type: 'category',
        name: 'Src Rank Id',
        data: [ ],
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
        inRange: { color: [ COLOR.Band0, COLOR.Band1, COLOR.Band2, COLOR.Band3 ] },
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
            { value: 1, label: allTransporType[1], color: COLOR.Band2 },
            { value: 2, label: allTransporType[2], color: COLOR.Band3 },
        ],
        textStyle: { color: COLOR.Grey40 },
    },
};

const CommunicationMatrix = observer(function ({ isShow, conditions }: { isShow: boolean;conditions: ConditionDataType}) {
    const [ dataSource, setDataSource ] = useState<any>({});
    const [ switchCondition, setSwitchCondition ] = useState({ type: 'bandwidth', showInner: false });
    useEffect(() => {
        if (isShow) {
            updateData(conditions);
        }
    }, [ isShow, conditions ]);
    useEffect(() => {
        if (isShow) {
            let { data } = dataSource;
            data = data.map((item: any) => {
                return [ item.srcRank, item.dstRank, item[switchCondition.type] ];
            });
            if (!switchCondition.showInner) {
                data = data.filter((item: any[]) => item[0] !== item[1]);
            }
            InitCharts({ ...dataSource, data, type: switchCondition.type });
        }
    }, [ dataSource, switchCondition ]);
    const updateData = async(conditions: ConditionDataType): Promise<void> => {
        const param = { iterationId: conditions.iterationId, stage: conditions.stage, operatorName: conditions.operatorName };
        const res = await queryCommunicationMatrix(param);
        const data = res.matrixList ?? [];
        const rankRes: {iterationsOrRanks: Array<{rank_id: string } > } =
            await queryRanks({ iterationId: conditions.iterationId });
        const rankIds = rankRes.iterationsOrRanks.map(item => item.rank_id);
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
                    <div id={'matrixchart'} style={{ height: '600px' }}></div>
                </div>
            </div>}
        />
    </div>
    );
};

export default CommunicationMatrix;
