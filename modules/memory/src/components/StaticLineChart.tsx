/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
 
import React, { useState, useEffect } from 'react';
import { observer } from 'mobx-react-lite';
import { runInAction } from 'mobx';
import { SearchBox } from '../utils/styleUtils';
import { Label } from './Common';
import { Select, Spin } from 'ascend-components';
import { StyledEmpty, customConsole as console } from 'ascend-utils';
import { useTranslation } from 'react-i18next';
import { Graph, StaticOperatorCurve } from '../entity/memory';
import { LineChart } from './LineChart';
import { Session } from '../entity/session';
import { MemorySession, DEFAULT_SIZE_CONDITION } from '../entity/memorySession';
import { staticOpMemoryGraphGet } from '../utils/RequestUtils';
 
const StaticLineChart = observer(({ session, memorySession, isDark }:
{ session: Session; memorySession: MemorySession; isDark: boolean }) => {
    const isCompare: boolean = session.compareRank.isCompare;
    // 静态图曲线数据源
    const [memoryStaticCurveData, setMemoryStaticCurveData] = useState<StaticOperatorCurve | undefined>(undefined);
    // 静态图曲线绘制数据
    const [staticLineChartData, setStaticLineChartData] = useState<Graph | undefined>(undefined);
    const [staticCurveSpin, setStaticCurveSping] = useState<boolean>(false);
    const { t } = useTranslation('memory');
 
    const onMemoryGraphIdChanged = (value: string): void => {
        runInAction(() => {
            memorySession.memoryGraphId = value;
            memorySession.staticSelectedRange = undefined;
            memorySession.searchEventOperatorName = '';
            memorySession.minSize = isCompare ? -DEFAULT_SIZE_CONDITION : 0;
            memorySession.maxSize = DEFAULT_SIZE_CONDITION;
            memorySession.current = 1;
            memorySession.pageSize = 10;
        });
    };
 
    const onStaticSelectedRangeChanged = (start: number, end: number): void => {
        runInAction(() => {
            if (start > end || !memoryStaticCurveData) {
                memorySession.staticSelectedRange = undefined;
                return;
            }
            const allDataSet = new Set(memoryStaticCurveData.lines
                .map(item => {
                    return parseFloat(item[0] as string);
                }).sort((a, b) => a - b));
            if (allDataSet.size <= 1) {
                memorySession.staticSelectedRange = undefined;
                return;
            }
            const allDatas = Array.from(allDataSet);
            memorySession.staticSelectedRange = { startTs: allDatas[start], endTs: allDatas[end] };
            memorySession.current = 1;
            memorySession.pageSize = 10;
        });
    };
 
    useEffect(() => {
        if (memorySession.rankIdCondition.value === undefined || memorySession.rankIdCondition.value === '' || memorySession.memoryGraphId === undefined) {
            setStaticLineChartData(undefined);
            setMemoryStaticCurveData(undefined);
            return;
        }
        setStaticCurveSping(true);
        staticOpMemoryGraphGet({ rankId: memorySession.rankIdCondition.value, graphId: memorySession.memoryGraphId, isCompare }).then((resp) => {
            // Reset the select range to null when rankId changes
            runInAction(() => {
                memorySession.staticSelectedRange = undefined;
            });
            setMemoryStaticCurveData(resp);
            setStaticLineChartData({
                columns: resp.legends?.map(legend => t(legend)),
                rows: resp.lines,
            });
        }).catch(err => {
            console.error(err);
        }).finally(() => {
            setStaticCurveSping(false);
        });
    }, [memorySession.rankIdCondition.value, session.isClusterMemoryCompletedSwitch, memorySession.memoryGraphId, t]);
 
    return (
        <div className="mb-30">
            <SearchBox>
                <div className="flex items-center">
                    <Label name={t('searchCriteria.GraphId')} />
                    <Select
                        value={memorySession.memoryGraphId}
                        size="middle"
                        onChange={onMemoryGraphIdChanged}
                        options={memorySession.memoryGraphIdList.map((graphId: string) => {
                            return {
                                value: graphId,
                                label: graphId,
                            };
                        })}
                    />
                </div>
            </SearchBox>
            <Spin spinning={staticCurveSpin} tip="loading...">
                { staticLineChartData
                    ? <LineChart
                        hAxisTitle={t('Node Index')}
                        vAxisTitle={t('Memory Usage (MB)')}
                        graph={staticLineChartData}
                        onSelectionChanged={onStaticSelectedRangeChanged}
                        record={memorySession.selectedStaticRecord}
                        isDark={isDark}
                        isStatic={true}
                    />
                    : <StyledEmpty style={{ marginTop: 160 }} translation={t}/>
                }
            </Spin>
        </div>
    );
});
 
export default StaticLineChart;
