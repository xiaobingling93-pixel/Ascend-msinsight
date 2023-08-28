import { observer } from 'mobx-react-lite';
import React, { useEffect, useState } from 'react';
import { Select, Checkbox } from 'antd';
import type { CheckboxChangeEvent } from 'antd/es/checkbox';
import * as echarts from 'echarts';
import { addResizeEvent, Container, Label, COLOR } from '../Common';
import { conditionDataType } from './Filter';
import { communicationMatrixData } from '../../utils/__test__/mockData';
import { optionDataType, VoidFunction } from '../../utils/interface';

const options: optionDataType[] = [
    {
        value: 'Bandwidth(GB/s)',
        label: 'Bandwidth(GB/s)',
    },
    {
        value: 'Transit Size(MB)',
        label: 'Transit Size(MB)',
    },
    {
        value: 'Transit Time(ms)',
        label: 'Transit Time(ms)',
    },
    {
        value: 'Transport Type',
        label: 'Transport Type',
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

function wrapData(data: any): any {
    baseOption.series[0].data = data;
    const max = Math.max(...data.map((item: number[]) => item[2]));
    const min = Math.min(...data.map((item: number[]) => item[2]));
    baseOption.visualMap.max = max;
    baseOption.visualMap.min = min;
    const ranklist = [ '0', '1', '2', '3', '4', '5', '6', '7' ];
    baseOption.xAxis.data = ranklist;
    baseOption.yAxis.data = ranklist;
    return baseOption;
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
        data: [ ],
        splitArea: {
            show: true,
        },
    },
    yAxis: {
        type: 'category',
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
            name: 'Punch Card',
            type: 'heatmap',
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

const CommunicationMatrix = observer(function ({ isShow, conditions }: { isShow: boolean;conditions: conditionDataType}) {
    const [ dataSource, setDataSource ] = useState<any>({});
    const [ switchCondition, setSwitchCondition ] = useState({ type: 'Bandwidth(GB/s)', showInner: false });
    useEffect(() => {
        if (isShow) {
            updateData();
        }
    }, [ isShow, conditions ]);
    useEffect(() => {
        if (isShow) {
            const keys = Object.keys(dataSource);
            let data = keys.map(key => {
                const list = key.split('_');
                return [ list[0], list[1], dataSource[key][switchCondition.type] ];
            });
            if (!switchCondition.showInner) {
                data = data.filter(item => item[0] !== item[1]);
            }
            InitCharts(data);
        }
    }, [ dataSource, switchCondition ]);
    const updateData = (): void => {
        setDataSource(communicationMatrixData);
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
