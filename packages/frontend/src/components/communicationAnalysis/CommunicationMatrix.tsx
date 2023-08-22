import { observer } from 'mobx-react-lite';
import React, { useEffect } from 'react';
import { addResizeEvent, Label } from '../Common';
import { Select } from 'antd';
import * as echarts from 'echarts';

const options = [
    {
        value: '0',
        label: '0',
    },
    {
        value: '1',
        label: '1',
    },
    {
        value: '2',
        label: '2',
    },
    {
        value: '3',
        label: '3',
    },
];

function InitCharts(): void {
    const chartDom = document.getElementById('matrixchart');
    if (chartDom !== null) {
        echarts.init(chartDom).dispose();
        const myChart = echarts.init(chartDom);
        myChart.setOption(wrapData());
        addResizeEvent(myChart);
    }
}

function wrapData(): any {
    return baseOption;
}

const data = [].map(function (item) {
    return [ item[1], item[0], item[2] || '-' ];
});
const baseOption = {
    tooltip: {
        position: 'top',
    },
    grid: {
        height: '50%',
        top: '10%',
    },
    xAxis: {
        type: 'category',
        data: [ '0', '1', '2', '3', '4', '5', '6', '7' ],
        splitArea: {
            show: true,
        },
    },
    yAxis: {
        type: 'category',
        data: [ '0', '1', '2', '3', '4', '5', '6', '7' ],
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
        inRange: { color: [ '#f82d18', '#eac299', '#c7eef5', '#0177ff' ] },
    },
    series: [
        {
            name: 'Punch Card',
            type: 'heatmap',
            data,
            label: {
                show: true,
            },
            emphasis: {
                itemStyle: {
                    shadowBlur: 10,
                    shadowColor: 'rgba(0, 0, 0, 0.5)',
                },
            },
        },
    ],
};

const updateChart = (val: string): void => {
    InitCharts();
};

const CommunicationMatrix = observer(function (props: { isShow: boolean}) {
    useEffect(() => {
        if (props.isShow) {
            InitCharts();
        }
    }, [props.isShow]);
    const handleChange = (val: string): void => {
        updateChart(val);
    };
    return (
        <div style={{ display: props.isShow ? 'block' : 'none' }}>
            <div>
                Suggestions
                <div>
                    <p>The time ratio of transiting data in HCCS,PCIE  and RDMA aer 0.74,0.26 and 0.00 respectively.
                        <p>HCCS general information and optimization suggestion:</p>
                    </p>
                </div>
            </div>
            <div>
                <div>Matrix Model</div>
                <div>
                    <Label name={'Communication Matrix Type'}/>
                    <Select
                        defaultValue="0"
                        style={{ width: 200 }}
                        onChange={handleChange}
                        options={options}
                    />
                </div>
                <div>
                    <div id={'matrixchart'} style={{ height: '400px' }}></div>
                </div>
            </div>
        </div>
    );
});

export default CommunicationMatrix;
