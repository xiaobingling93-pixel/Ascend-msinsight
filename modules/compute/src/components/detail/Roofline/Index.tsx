/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useMemo, useState } from 'react';
import { observer } from 'mobx-react';
import { useTranslation } from 'react-i18next';
import { type Session } from '../../../entity/session';
import CollapsiblePanel from 'ascend-collapsible-panel';
import { Tabs } from 'ascend-components';
import type { Tab } from 'rc-tabs/lib/interface';
import { RooflineChartGroup } from './RooflineChart';
import { queryRoofline } from '../../../components/RequestUtils';
import { Hit } from 'ascend-utils';
import type { TFunction } from 'i18next';

/**
 * 表示原始数据的接口
 * @interface IOriginData
 * @property {string} soc - 系统操作条件。
 * @property {string} advice - 建议或说明。
 * @property {IOriginRooflineChart[]} data - 包含原始图表数据的数组。
 */
export interface IOriginData {
    soc: string;
    advice: string;
    data: IOriginRooflineChart[];
}

/**
 * 表示原始图表数据的接口
 * @interface IOriginRooflineChart
 * @property {string} title - 图表的标题。
 * @property {IOriginRoofline[]} rooflines - 包含原始线条数据的数组。
 */
interface IOriginRooflineChart {
    title: string;
    rooflines: IOriginRoofline[];
}

/**
 * 表示原始线条数据的接口
 * @interface IOriginRoofline
 * @property {string} bw - 带宽。
 * @property {string} computility - 计算能力。
 * @property {[string, string]} point - 数据点，包含两个字符串。
 * @property {string} computilityName - 计算能力的名称。
 * @property {string} bwName - 带宽的名称。
 * @property {string} ratio - 比率。
 */
interface IOriginRoofline {
    bw: string;
    computility: string;
    point: [string, string];
    computilityName: string;
    bwName: string;
    ratio: string;
}

/**
 * 处理后的数据的接口
 * @interface IData
 * @property {string} soc - 系统操作条件。
 * @property {string} advice - 建议或说明。
 * @property {IRooflineChart[]} data - 包含处理后的图表数据的数组。
 */
interface IData {
    soc: string;
    advice: string;
    data: IRooflineChart[];
}

/**
 * 处理后的图表数据的接口
 * @interface IRooflineChart
 * @property {string} title - 图表的标题。
 * @property {IRoofline[]} rooflines - 包含处理后的线条数据的数组。
 */
export interface IRooflineChart {
    title: string;
    rooflines: IRoofline[];
}

/**
 * 处理后的线条数据的接口
 * @interface IRoofline
 * @property {number} bw - 带宽，数值类型。
 * @property {number} computility - 计算能力，数值类型。
 * @property {Point} point - 数据点，包含两个数值。
 * @property {string} computilityName - 计算能力的名称。
 * @property {string} bwName - 带宽的名称。
 * @property {string} ratio - 比率。
 */
export interface IRoofline {
    bw: number;
    computility: number;
    point: Point;
    computilityName: string;
    bwName: string;
    ratio: string;
}

/**
 * 二维点的类型，包含两个数值
 * @typedef {Array<number>} Point
 * @property {number} [0] - 点的 x 坐标。
 * @property {number} [1] - 点的 y 坐标。
 */
export type Point = [number, number];
interface ITab {
    label: string;
    key: string;
    contents: string[];
    surportSocs?: string[];
}

const allTabItems: ITab[] = [
    {
        label: 'Memory Unit',
        key: 'memoryUnit',
        contents: ['Memory Unit', 'HBM/L2', 'GM/L2', 'Memory Unit(Cube)', 'Memory Unit(Vector)'],
    },
    {
        label: 'Memory Transfer',
        key: 'memoryPath',
        contents: ['Memory Pipe Cube', 'Memory Pipe Vector', 'Memory Transfer(Cube)', 'Memory Transfer(Vector)'],
        surportSocs: ['910'],
    },
    {
        label: 'Pipeline',
        key: 'pipeline',
        contents: ['Pipe Line Cube', 'Pipe Line Vector', 'Pipeline(Cube)', 'Pipeline(Vector)'],
        surportSocs: ['910'],
    },
];

const defaultData: IData = {
    soc: '',
    advice: '',
    data: [],
};

function getTabItems(data: IData, tDetails: TFunction): Tab[] {
    const soc = data?.soc ?? '';
    const allRooflineCharts = data?.data ?? [];
    const tabItems = allTabItems.filter(tab =>
        tab.surportSocs === undefined || tab.surportSocs.find(surportSoc => soc.includes(surportSoc)) !== undefined);
    return tabItems.map((tabItem, index) => {
        const rooflineCharts = tabItem.contents.reduce<IRooflineChart[]>((pre, cur) => {
            const curChart = allRooflineCharts.find(chart => chart.title === cur);
            if (curChart !== undefined && curChart !== null) {
                pre.push(curChart);
            }
            return pre;
        }, []);
        return {
            label: tDetails(tabItem.label),
            key: tabItem.key,
            children: <RooflineChartGroup key={tabItem.key} dataSource={rooflineCharts}/>,
        };
    });
}

const index = observer(({ session }: { session: Session }): JSX.Element => {
    const { t: tDetails } = useTranslation('details');
    const [data, setData] = useState<IData>(defaultData);

    const items = useMemo(() => getTabItems(data, tDetails), [data, tDetails]);

    const updateData = async (): Promise<void> => {
        const res = await queryRoofline();
        const originData = res ?? defaultData;
        const newData = {
            ...originData,
            data: originData.data.map(chart => ({
                ...chart,
                rooflines: chart.rooflines.map(roofline => {
                    const bw = Number(roofline.bw);
                    const computility = Number(roofline.computility);
                    const point: Point = [Number(roofline.point[0]), Number(roofline.point[1])];
                    return {
                        ...roofline,
                        bw,
                        computility,
                        point,
                    };
                }),
            })),
        };
        setData(newData);
    };

    useEffect(() => {
        if (!session.parseStatus) {
            setData(defaultData);
        }
        updateData();
    }, [session.parseStatus]);
    return data?.data?.length > 0
        ? (
            <CollapsiblePanel title={tDetails('Roofline')} collapsible>
                <Tabs items={items} type="card" data-testId={'rooflineChart'} />
                {data?.advice?.length > 0 && <Hit text={data.advice} type={'alarm'} data-testId={'rooflineAdvice'}/>}
            </CollapsiblePanel>
        )
        : <></>;
});

export default index;
