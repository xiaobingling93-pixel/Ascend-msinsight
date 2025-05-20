/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import React, { useState, useEffect } from 'react';
import { observer } from 'mobx-react-lite';
import { runInAction } from 'mobx';
import CollapsiblePanel from 'ascend-collapsible-panel';
import { Spin } from 'ascend-components';
import { StyledEmpty, customConsole as console } from 'ascend-utils';
import { useTranslation } from 'react-i18next';
import { Graph, Curve } from '../entity/curve';
import { LineChart } from './LineChart';
import { CurveSession } from '../entity/curveSession';
import { curveGet } from '../utils/RequestUtils';

const DynamicLineChart = observer(({ curveSession, isDark }:
{ curveSession: CurveSession; isDark: boolean }) => {
    // 内存曲线数据源
    const [curveData, setCurveData] = useState<Curve | undefined>(undefined);
    // 内存曲线绘制数据
    const [lineChartData, setLineChartData] = useState<Graph | undefined>(undefined);
    const [curveSpin, setCurveSpin] = useState<boolean>(false);
    const { t } = useTranslation('statistic');

    const onSelectedRangeChanged = (start: number, end: number): void => {
        runInAction(() => {
            if (start > end || !curveData) {
                curveSession.selectedRange = undefined;
                return;
            }
            const allDataSet = new Set(curveData.lines
                .map(item => {
                    return item[0] as string;
                }));
            if (allDataSet.size <= 1) {
                curveSession.selectedRange = undefined;
                return;
            }
            const allDatas = Array.from(allDataSet);
            curveSession.selectedRange = { startTs: allDatas[start].toString(), endTs: allDatas[end].toString() };
            curveSession.current = 1;
            curveSession.pageSize = 10;
        });
    };

    const onMemoryCurveGet = (): void => {
        setCurveSpin(true);
        curveGet({ rankId: curveSession.rankIdCondition.value, type: curveSession.groupId }).then((resp) => {
            // Reset the select range to null when rankId changes
            runInAction(() => {
                curveSession.selectedRange = undefined;
            });
            setCurveData(resp);
            let columns: string[] = [];
            columns = resp.legends?.map(legend => t(legend));
            setLineChartData({
                columns,
                rows: resp.lines,
            });
        }).catch(err => {
            console.error(err);
        }).finally(() => {
            setCurveSpin(false);
        });
    };

    useEffect(() => {
        if (curveSession.rankIdCondition.value === undefined || curveSession.rankIdCondition.value === '') {
            setLineChartData(undefined);
            setCurveData(undefined);
            return;
        }
        onMemoryCurveGet();
    }, [curveSession.rankIdCondition.value, curveSession.groupId]);

    return (
        <div className="mb-30">
            <CollapsiblePanel title={t('Curve Data')}>
                <Spin spinning={curveSpin} tip="loading...">
                    { lineChartData
                        ? <LineChart
                            hAxisTitle={'x'}
                            vAxisTitle={'y'}
                            graph={lineChartData}
                            onSelectionChanged={onSelectedRangeChanged}
                            record={curveSession.selectedRecord}
                            isDark={isDark}
                        />
                        : <StyledEmpty style={{ marginTop: 160 }} />
                    }
                </Spin>
            </CollapsiblePanel>
        </div>
    );
});

export default DynamicLineChart;
