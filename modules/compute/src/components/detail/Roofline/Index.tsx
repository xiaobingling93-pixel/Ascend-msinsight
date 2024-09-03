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

interface IOriginData {
    soc: string;
    advice: string | string[];
    data: IOriginRooflineChart[];
}

interface IOriginRooflineChart {
    title: string;
    rooflines: IOriginRoofline[];
}

interface IOriginRoofline {
    bw: string;
    computility: string;
    point: [string, string];
    computilityName: string;
    bwName: string;
    ratio: string;
}

interface IData {
    soc: string;
    advice: string | string[];
    data: IRooflineChart[];
}

export interface IRooflineChart {
    title: string;
    rooflines: IRoofline[];
}

export interface IRoofline {
    bw: number;
    computility: number;
    point: [number, number];
    bwName: string;
    computilityName: string;
    ratio: string;
}

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
        contents: ['Memory Unit', 'HBM/L2', 'Memory Unit(Cube)', 'Memory Unit(Vector)'],
    },
    {
        label: 'Memory Transfer',
        key: 'memoryPath',
        contents: ['Memory Pipe Cube', 'Memory Pipe Vector'],
        surportSocs: ['910B'],
    },
    {
        label: 'Pipeline',
        key: 'pipeline',
        contents: ['Pipe Line Cube', 'Pipe Line Vector'],
        surportSocs: ['910B'],
    },
];

const defaultData: IData =
    {
        soc: '',
        advice: '',
        data: [],
    };

function getTabItems(data: IData, tDetails: TFunction): Tab[] {
    const soc = data?.soc ?? '';
    const allRooflineCharts = data?.data ?? [];
    const tabItems = allTabItems.filter(tab =>
        tab.surportSocs === undefined || tab.surportSocs.find(surportSoc => soc.includes(surportSoc)) !== undefined);
    return tabItems.map(tabItem => {
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
        const originData = (res ?? defaultData) as IOriginData;
        const newData = {
            ...originData,
            data: originData.data.map(chart => ({
                ...chart,
                rooflines: chart.rooflines.map(roofline => {
                    const bw = Number(roofline.bw);
                    const computility = Number(roofline.computility);
                    const point = [Number(roofline.point[0]), Number(roofline.point[1])] as [number, number];
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
    return (
        <CollapsiblePanel title={tDetails('Roofline')}>
            <Tabs items={items}></Tabs>
            {data?.advice?.length > 0 && <Hit text={data.advice}/>}
        </CollapsiblePanel>
    );
});

export default index;
