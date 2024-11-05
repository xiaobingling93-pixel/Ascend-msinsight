/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
 
import React, { useState, useEffect } from 'react';
import { observer } from 'mobx-react-lite';
import { runInAction } from 'mobx';
import CollapsiblePanel from 'ascend-collapsible-panel';
import { Spin } from 'ascend-components';
import { StyledEmpty, customConsole as console } from 'ascend-utils';
import { useTranslation } from 'react-i18next';
import { Graph, MemoryCurve } from '../entity/memory';
import { LineChart } from './LineChart';
import { Session } from '../entity/session';
import { MemorySession, DEFAULT_SIZE_CONDITION } from '../entity/memorySession';
import { memoryCurveGet } from '../utils/RequestUtils';
 
const DynamicLineChart = observer(({ session, memorySession, isDark }:
{ session: Session; memorySession: MemorySession; isDark: boolean }) => {
    const isCompare: boolean = session.compareRank.isCompare;
    // 内存曲线数据源
    const [memoryCurveData, setMemoryCurveData] = useState<MemoryCurve | undefined>(undefined);
    // 内存曲线绘制数据
    const [lineChartData, setLineChartData] = useState<Graph | undefined>(undefined);
    const [curveSpin, setCurveSpin] = useState<boolean>(false);
    const { t } = useTranslation('memory');
 
    const onSelectedRangeChanged = (start: number, end: number): void => {
        runInAction(() => {
            if (start > end || !memoryCurveData) {
                memorySession.selectedRange = undefined;
                return;
            }
            const allDataSet = new Set(memoryCurveData.lines
                .map(item => {
                    return parseFloat(item[0] as string);
                }).sort((a, b) => a - b));
            if (allDataSet.size <= 1) {
                memorySession.selectedRange = undefined;
                return;
            }
            const allDatas = Array.from(allDataSet);
            memorySession.selectedRange = { startTs: allDatas[start], endTs: allDatas[end] };
            memorySession.current = 1;
            memorySession.pageSize = 10;
        });
    };
 
    const onMemoryCurveGet = (): void => {
        setCurveSpin(true);
        memoryCurveGet({ rankId: memorySession.rankIdCondition.value, type: isCompare ? 'Overall' : memorySession.groupId, isCompare }).then((resp) => {
            // Reset the select range to null when rankId changes
            runInAction(() => {
                memorySession.selectedRange = undefined;
            });
            setMemoryCurveData(resp);
            let columns: string[] = [];
            if (memorySession.groupId === 'Stream') {
                columns = resp.legends?.map(legend => {
                    const index = legend.lastIndexOf(' ');
                    if (index > -1) {
                        return t(legend.slice(0, index), { stream: legend.slice(index + 1) });
                    } else {
                        return t(legend);
                    }
                });
            } else {
                columns = resp.legends?.map(legend => t(legend));
            }
            setLineChartData({
                title: resp.title,
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
        if (memorySession.rankIdCondition.value === undefined || memorySession.rankIdCondition.value === '') {
            setLineChartData(undefined);
            setMemoryCurveData(undefined);
            return;
        }
        onMemoryCurveGet();
    }, [memorySession.rankIdCondition.value, memorySession.groupId, t, session.isClusterMemoryCompletedSwitch]);
 
    useEffect(() => {
        runInAction(() => {
            memorySession.selectedRange = undefined;
            memorySession.searchEventOperatorName = '';
            memorySession.minSize = isCompare ? -DEFAULT_SIZE_CONDITION : 0;
            memorySession.maxSize = DEFAULT_SIZE_CONDITION;
            memorySession.current = 1;
            memorySession.pageSize = 10;
        });
        if (memorySession.rankIdCondition.value === undefined || memorySession.rankIdCondition.value === '') {
            return;
        }
        onMemoryCurveGet();
    }, [isCompare]);
 
    return (
        <div className="mb-30">
            <CollapsiblePanel title={t('Memory Analysis')}>
                <Spin spinning={curveSpin} tip="loading...">
                    { lineChartData
                        ? <LineChart
                            hAxisTitle={t('Time (ms)')}
                            vAxisTitle={t('Memory Usage (MB)')}
                            graph={lineChartData}
                            onSelectionChanged={onSelectedRangeChanged}
                            record={memorySession.selectedRecord}
                            isDark={isDark}
                            isStatic={false}
                        />
                        : <StyledEmpty style={{ marginTop: 160 }} />
                    }
                </Spin>
            </CollapsiblePanel>
        </div>
    );
});
 
export default DynamicLineChart;
