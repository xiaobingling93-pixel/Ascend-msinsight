/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import React, { useState, useEffect } from 'react';
import { useTranslation } from 'react-i18next';
import { observer } from 'mobx-react-lite';
import { AntTableChart } from '../components/AntTableChart';
import { LineChart } from '../components/LineChart';
import { Button, Input, InputNumber, Select, Spin } from 'ascend-components';
import { message } from 'antd';
import type { Session } from '../entity/session';
import type {
    Graph, MemoryCurve, OperatorDetail, StaticOperatorCurve,
    StaticOperatorListDetail, OperatorMemoryCondition, StaticMemoryCondition,
} from '../entity/memory';
import {
    memoryTypeGet, staticOpMemoryGraphGet, staticOpMemoryListGet,
    memoryCurveGet, operatorsMemoryGet, resourceTypeGet,
} from '../utils/RequestUtils';
import { useHit, Label } from '../components/Common';
import styled from '@emotion/styled';
import { GroupRankIdsByHost, StyledEmpty, customConsole as console } from 'ascend-utils';
import { Layout } from 'ascend-layout';
import CollapsiblePanel from 'ascend-collapsible-panel';

interface SelectedRange {
    startTs: number;
    endTs: number;
}

interface ConditionType {
    options: string[];
    value: string;
    ranks?: Map<string, string[]>;
}

const COMPARE_MIN_INPUT_NUMBER = -2147483648;
const MAX_INPUT_NUMBER = 4294967295;
const DEFAULT_SIZE_CONDITION = 1000000;

const FlexDiv = styled.div`
    display: flex;
    align-items: center;
`;

const SearchBox = styled(FlexDiv)`
    margin: 0 0 20px;
    flex-wrap: wrap;
    gap: 24px;
    padding: 0 24px;
`;

const groupBy = [
    { label: 'Overall', value: 'Overall' },
    { label: 'Stream', value: 'Stream' },
];

const memoryGraphType = {
    dynamic: 'dynamic',
    static: 'static',
    mix: 'mix',
};

const dataResourceType = {
    pytorch: 'Pytorch',
    mindspore: 'MindSpore',
};

// eslint-disable-next-line max-lines-per-function
const MemoryAnalysis = observer(({ session, isDark }: { session: Session; isDark: boolean }) => {
    // 是否为比对场景
    const isCompare: boolean = session.compareRank.isCompare;
    // memory数据类型，默认为dynamic
    const [memoryType, setMemoryType] = useState<string>(memoryGraphType.dynamic);
    // memory数据来源，默认为pytorch
    const [resourceType, setResourceType] = useState<string>(dataResourceType.pytorch);
    // 静态图graphId
    const [memoryGraphId, setMemoryGraphId] = useState<string | undefined>(undefined);
    // 静态图graphIdList
    const [memoryGraphIdList, setMemoryGraphIdList] = useState<string[]>([]);
    // 静态图曲线数据源
    const [memoryStaticCurveData, setMemoryStaticCurveData] = useState<StaticOperatorCurve | undefined>(undefined);
    // 静态图曲线绘制数据
    const [staticLineChartData, setStaticLineChartData] = useState<Graph | undefined>(undefined);
    // 算子表格内存信息
    const [memoryTableData, setMemoryTableData] = useState<any>([]);
    // 算子表格表头信息
    const [memoryTableHead, setMemoryTableHead] = useState<any>([]);
    // 内存曲线数据源
    const [memoryCurveData, setMemoryCurveData] = useState<MemoryCurve | undefined>(undefined);
    // 内存曲线绘制数据
    const [lineChartData, setLineChartData] = useState<Graph | undefined>(undefined);
    const [selectedRange, setSelectedRange] = useState<SelectedRange | undefined>();
    const [searchEventOperatorName, setSearchEventOperatorName] = useState<string>('');
    const [minSize, setMinSize] = useState<number>(0);
    // 最大内存范围，默认DEFAULT_SIZE_CONDITION KB
    const [maxSize, setMaxSize] = useState<number>(DEFAULT_SIZE_CONDITION);
    const [curveSpin, setCurveSpin] = useState<boolean>(false);
    const [staticCurveSpin, setStaticCurveSping] = useState<boolean>(false);
    const [tableSpin, setTableSpin] = useState<boolean>(false);
    const [groupId, setGroupId] = useState<string>('Overall');
    const [current, setCurrent] = useState<number>(1);
    const [pageSize, setPageSize] = useState<number>(10);
    const [total, setTotal] = useState<number>(0);
    const [orderBy, setOrderBy] = useState<string | undefined>(undefined);
    const [order, setOrder] = useState<string | undefined>(undefined);
    const [isBtnDisabled, setBtnDisabled] = useState<boolean>(true);
    const [hostCondition, setHostCondition] = useState<ConditionType>({ options: [], value: '' });
    const [rankIdCondition, setRankIdCondition] = useState<ConditionType>({ options: [], value: '' });
    const { t } = useTranslation('memory');
    const hit = useHit();

    const fetchMemoryType = (memoryRankId: string | undefined): void => {
        if (memoryRankId === undefined || memoryRankId === '') {
            return;
        }
        memoryTypeGet({ rankId: memoryRankId }).then((resp) => {
            const type = resp.type;
            const graphIdList = resp.graphId;
            setMemoryType(type);
            if (graphIdList.length > 0) {
                setMemoryGraphId(graphIdList[0]);
            }
            setMemoryGraphIdList(graphIdList);
        }).catch(err => {
            console.error(err);
        });
    };

    const fetchResourceType = (memoryRankId: string | undefined): void => {
        if (memoryRankId === undefined || memoryRankId === '') {
            return;
        }
        resourceTypeGet({ rankId: memoryRankId }).then((resp) => {
            const type = resp.type;
            if (type === dataResourceType.mindspore) {
                setGroupId('Overall');
            }
            setResourceType(type);
        }).catch(err => {
            console.error(err);
        });
    };

    const onSearchEventOperatorChanged: React.ChangeEventHandler<HTMLInputElement | HTMLTextAreaElement> = (event) => {
        setSearchEventOperatorName(event.target.value as string);
    };

    const [selectedRecord, setSelectedRecord] = useState<OperatorDetail | undefined>();
    const [selectedStaticRecord, setSelectedStaticRecord] = useState<StaticOperatorListDetail | undefined>();
    const onRowSelected = (record?: any, rowIndex?: number): void => {
        switch (memoryType) {
            case memoryGraphType.dynamic:
                setSelectedRecord(record);
                break;
            case memoryGraphType.static:
                setSelectedStaticRecord(record);
                break;
            default:
                break;
        }
    };

    const onFilterEventMinSizeInputChanged = (value: number | string | null): void => {
        setMinSize(value as number);
    };

    const onFilterEventMaxSizeInputChanged = (value: number | string | null): void => {
        setMaxSize(value as number);
    };

    const setTempCurrent = (resetCurrent = false): number => {
        let tempCurrent = current;
        if (resetCurrent) {
            tempCurrent = 1;
            setCurrent(1);
        }
        return tempCurrent;
    };

    const setParamOtherCondition = (param: any): any => {
        let newParam = param;
        if (order !== undefined) {
            newParam = { order, orderBy, ...param };
        }

        setTableSpin(true);
        setBtnDisabled(true);

        return newParam;
    };

    const handleOperatorDetails = (operatorDetails: any[]): any => {
        return isCompare
            ? operatorDetails.map(item => {
                if (item.diff === undefined || item.diff === null) {
                    return item;
                }
                item.diff.source = t('Difference');
                item.diff.children = [{ ...item.baseline, source: t('Baseline') }, { ...item.compare, source: t('Comparison') }];
                return item.diff;
            })
            : operatorDetails.map(item => item.compare);
    };

    const onSearch = (searchName: string, minimumSize: number, maximumSize: number, resetCurrent = false): void => {
        if (rankIdCondition.value === undefined || rankIdCondition.value === '') {
            return;
        }
        if (maximumSize < minimumSize) {
            message.warning(t('Invalid Size Warning'));
            return;
        }
        const tempCurrent = setTempCurrent(resetCurrent);
        let param: OperatorMemoryCondition = {
            rankId: rankIdCondition.value,
            type: isCompare ? 'Overall' : groupId,
            currentPage: tempCurrent,
            pageSize,
            searchName,
            minSize: minimumSize,
            maxSize: maximumSize,
            isCompare,
        };
        if (selectedRange) {
            param.startTime = selectedRange.startTs;
            param.endTime = selectedRange.endTs;
        }
        param = setParamOtherCondition(param);
        operatorsMemoryGet(param).then((resp) => {
            const operatorDetails = resp.operatorDetail;
            setTotal(resp.totalNum);
            setMemoryTableData(handleOperatorDetails(operatorDetails));
            if (JSON.stringify(memoryTableHead) !== JSON.stringify(resp.columnAttr)) {
                setMemoryTableHead(resp.columnAttr);
            }
        }).catch(err => {
            console.error(err);
        }).finally(() => {
            setTableSpin(false);
            setBtnDisabled(false);
        });
    };

    const onStaticSearch = (searchName: string, minimumSize: number, maximumSize: number, resetCurrent = false): void => {
        if (rankIdCondition.value === undefined || rankIdCondition.value === '' || memoryGraphId === undefined) {
            return;
        }
        if (maximumSize < minimumSize) {
            message.warning(t('Invalid Size Warning'));
            return;
        }
        const tempCurrent = setTempCurrent(resetCurrent);
        let param: StaticMemoryCondition = {
            rankId: rankIdCondition.value,
            graphId: memoryGraphId,
            currentPage: tempCurrent,
            pageSize,
            searchName,
            minSize: minimumSize,
            maxSize: maximumSize,
            isCompare,
        };
        if (selectedRange) {
            param.startNodeIndex = selectedRange.startTs; param.endNodeIndex = selectedRange.endTs;
        }
        param = setParamOtherCondition(param);
        staticOpMemoryListGet(param).then((resp) => {
            const staticOperatorListDetails = resp.staticOperatorListDetail;
            setTotal(resp.totalNum);
            setMemoryTableData(handleOperatorDetails(staticOperatorListDetails));
            if (JSON.stringify(memoryTableHead) !== JSON.stringify(resp.columnAttr)) {
                setMemoryTableHead(resp.columnAttr);
            }
        }).catch(err => {
            console.error(err);
        }).finally(() => {
            setBtnDisabled(false);
            setTableSpin(false);
        });
    };

    const onBaseChanged = (): void => {
        setSelectedRange(undefined);
        setSearchEventOperatorName('');
        setCurrent(1);
        setPageSize(10);
        setMinSize(isCompare ? -DEFAULT_SIZE_CONDITION : 0);
        setMaxSize(DEFAULT_SIZE_CONDITION);
    };

    const onRankIdChanged = (value: string): void => {
        setRankIdCondition({ ...rankIdCondition, value });
        onBaseChanged();
    };

    const onMemoryGraphIdChanged = (value: string): void => {
        setMemoryGraphId(value);
        onBaseChanged();
    };

    const onReset = (): void => {
        setSearchEventOperatorName('');
        setMinSize(isCompare ? -DEFAULT_SIZE_CONDITION : 0);
        setMaxSize(DEFAULT_SIZE_CONDITION);
        switch (memoryType) {
            case memoryGraphType.dynamic:
                onSearch('', 0, DEFAULT_SIZE_CONDITION);
                break;
            case memoryGraphType.static:
                onStaticSearch('', 0, DEFAULT_SIZE_CONDITION);
                break;
            default:
                break;
        };
    };

    const onSelectedRangeChanged = (start: number, end: number): void => {
        if (start > end || !memoryCurveData) {
            setSelectedRange(undefined);
            return;
        }
        const curveData = memoryType === memoryGraphType.dynamic ? memoryCurveData : memoryStaticCurveData;
        if (curveData === undefined) {
            return;
        }
        const allDataSet = new Set(curveData.lines
            .map(item => {
                return parseFloat(item[0] as string);
            }).sort((a, b) => a - b));
        if (allDataSet.size <= 1) {
            setSelectedRange(undefined);
            return;
        }
        const allDatas = Array.from(allDataSet);
        setSelectedRange({ startTs: allDatas[start], endTs: allDatas[end] });
        setCurrent(1);
        setPageSize(10);
    };

    const groupByOptions = groupBy.map(item => ({
        ...item,
        label: t(`searchCriteria.${item.label}`),
    }));

    const onMemoryCurveGet = (): void => {
        setCurveSpin(true);
        memoryCurveGet({ rankId: rankIdCondition.value, type: isCompare ? 'Overall' : groupId, isCompare }).then((resp) => {
            // Reset the select range to null when rankId changes
            setSelectedRange(undefined);
            setMemoryCurveData(resp);
            setLineChartData({
                title: resp.title,
                columns: resp.legends?.map(legend => t(legend)),
                rows: resp.lines,
            });
        }).catch(err => {
            console.error(err);
        }).finally(() => {
            setCurveSpin(false);
        });
    };

    useEffect(() => {
        fetchMemoryType(rankIdCondition.value);
        fetchResourceType(rankIdCondition.value);
    }, [rankIdCondition.value]);

    useEffect(() => {
        switch (memoryType) {
            case memoryGraphType.dynamic:
                onSearch(searchEventOperatorName, minSize, maxSize);
                break;
            case memoryGraphType.static:
                onStaticSearch(searchEventOperatorName, minSize, maxSize);
                break;
            default:
                break;
        }
    }, [selectedRange, rankIdCondition.value, current, pageSize, order, orderBy,
        session.isClusterMemoryCompletedSwitch, groupId, memoryGraphId, t, isCompare, memoryType]);

    useEffect(() => {
        if (rankIdCondition.value === undefined || rankIdCondition.value === '') {
            setBtnDisabled(true);
            setLineChartData(undefined);
            setMemoryCurveData(undefined);
            setMemoryTableData([]);
            setTotal(0);
            setCurrent(1);
            setPageSize(10);
            return;
        }
        onMemoryCurveGet();
    }, [rankIdCondition.value, groupId, t, session.isClusterMemoryCompletedSwitch]);

    useEffect(() => {
        onBaseChanged();
        if (rankIdCondition.value === undefined || rankIdCondition.value === '') {
            return;
        }
        onMemoryCurveGet();
    }, [isCompare]);

    useEffect(() => {
        const { hosts, ranks } = GroupRankIdsByHost(session.memoryRankIds);
        setHostCondition({ options: hosts, value: hosts[0] ?? '', ranks });
    }, [JSON.stringify(session.memoryRankIds)]);

    useEffect(() => {
        if (rankIdCondition.value === undefined || rankIdCondition.value === '' || memoryGraphId === undefined) {
            setBtnDisabled(true);
            setStaticLineChartData(undefined);
            setMemoryStaticCurveData(undefined);
            setMemoryTableData([]);
            setTotal(0);
            setCurrent(1);
            setPageSize(10);
            return;
        }
        setStaticCurveSping(true);
        staticOpMemoryGraphGet({ rankId: rankIdCondition.value, graphId: memoryGraphId, isCompare }).then((resp) => {
            // Reset the select range to null when rankId changes
            setSelectedRange(undefined);
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
    }, [rankIdCondition.value, session.isClusterMemoryCompletedSwitch, memoryGraphId, t]);

    useEffect(() => {
        // 只对RankId为数字做排序，不能转为数字的字符串则不排序
        const rankIdOptions: string[] = JSON.parse(JSON.stringify(hostCondition.ranks?.get(hostCondition.value) ?? []))
            .sort((a: any, b: any) => Number(a) - Number(b));
        if (rankIdOptions.length === 0) {
            setRankIdCondition({ options: [], value: '' });
            return;
        }
        const rankIdValue = (rankIdCondition.value === undefined || rankIdCondition.value === '') ? rankIdOptions[0] : rankIdCondition.value;
        setRankIdCondition({ options: rankIdOptions, value: rankIdValue });
    }, [hostCondition.options, hostCondition.value, hostCondition.ranks]);

    useEffect(() => {
        setRankIdCondition({ options: rankIdCondition.options, value: rankIdCondition.options[0] });
    }, [session.isClusterMemoryCompletedSwitch]);

    useEffect(() => {
        if (session.compareRank.rankId === rankIdCondition.value) {
            return;
        }
        if (session.memoryRankIds.includes(session.compareRank.rankId)) {
            onRankIdChanged(session.compareRank.rankId);
        }
    }, [session.compareRank.rankId, JSON.stringify(session.memoryRankIds)]);

    return (
        <Layout>
            <div className="mb-30">
                <SearchBox style={{ padding: '0 24px' }}>
                    {hostCondition.options.length > 0
                        ? <FlexDiv>
                            <Label name={t('searchCriteria.Host')} />
                            <Select
                                value={hostCondition.value}
                                size="middle"
                                disabled={isCompare}
                                onChange={(value: string): void => setHostCondition({ ...hostCondition, value })}
                                options={hostCondition.options.map((host) => {
                                    return { value: host, label: host };
                                })}
                            />
                        </FlexDiv>
                        : <></>
                    }
                    <FlexDiv>
                        <Label name={t('searchCriteria.RankId')} />
                        <Select
                            value={rankIdCondition.value}
                            size="middle"
                            onChange={onRankIdChanged}
                            disabled={isCompare}
                            options={rankIdCondition.options.map((rankId) => {
                                return {
                                    value: rankId,
                                    label: rankId.replace(`${hostCondition.value} `, ''),
                                };
                            })}
                        />
                    </FlexDiv>
                    {
                        (resourceType === dataResourceType.pytorch && !isCompare)
                            ? <FlexDiv>
                                <Label name={<span>{t('searchCriteria.GroupBy')}{hit}</span>} />
                                <Select
                                    value={groupId}
                                    style={{ width: 180 }}
                                    onChange={(value: string): void => setGroupId(value)}
                                    options={groupByOptions}
                                />
                            </FlexDiv>
                            : null
                    }
                </SearchBox>
                <CollapsiblePanel title={t('Memory Analysis')}>
                    <Spin spinning={curveSpin} tip="loading...">
                        { lineChartData
                            ? <LineChart
                                hAxisTitle={t('Time (ms)')}
                                vAxisTitle={t('Memory Usage (MB)')}
                                graph={lineChartData}
                                onSelectionChanged={onSelectedRangeChanged}
                                record={selectedRecord}
                                isDark={isDark}
                                isStatic={false}
                            />
                            : <StyledEmpty style={{ marginTop: 160 }} />
                        }
                    </Spin>
                </CollapsiblePanel>
            </div>

            { memoryType === memoryGraphType.static
                ? <div className="mb-30">
                    <SearchBox>
                        <div className="flex items-center">
                            <Label name={t('searchCriteria.GraphId')} />
                            <Select
                                value={memoryGraphId}
                                size="middle"
                                onChange={onMemoryGraphIdChanged}
                                options={memoryGraphIdList.map((graphId) => {
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
                                onSelectionChanged={onSelectedRangeChanged}
                                record={selectedStaticRecord}
                                isDark={isDark}
                                isStatic={true}
                            />
                            : <StyledEmpty style={{ marginTop: 160 }} translation={t}/>
                        }
                    </Spin>
                </div>
                : null }

            <CollapsiblePanel title={t('Memory Allocation/Release Details')} secondary>
                <SearchBox>
                    <div className="flex items-center">
                        <Label name={t('searchCriteria.Name')} />
                        <Input
                            value={searchEventOperatorName}
                            onChange={onSearchEventOperatorChanged}
                            placeholder={t('searchCriteria.Search by Name')}
                            allowClear
                            maxLength={200}
                        />
                    </div>
                    <div className="flex items-center">
                        <Label name={t('searchCriteria.Min Size')} />
                        <InputNumber
                            value={minSize}
                            onChange={onFilterEventMinSizeInputChanged}
                            min={isCompare ? COMPARE_MIN_INPUT_NUMBER : 0}
                            max={MAX_INPUT_NUMBER}
                        />
                    </div>
                    <div className="flex items-center">
                        <Label name={t('searchCriteria.Max Size')} />
                        <InputNumber
                            value={maxSize}
                            onChange={onFilterEventMaxSizeInputChanged}
                            min={isCompare ? COMPARE_MIN_INPUT_NUMBER : 0}
                            max={MAX_INPUT_NUMBER}
                            minLength={1}
                        />
                    </div>
                    <div className="flex items-center">
                        <Button
                            onClick={(): void => memoryType === memoryGraphType.dynamic
                                ? onSearch(searchEventOperatorName, minSize, maxSize, true)
                                : onStaticSearch(searchEventOperatorName, minSize, maxSize, true)}
                            type="primary"
                            style={{ marginRight: 8 }}
                            disabled={isBtnDisabled}
                        >
                            {t('searchCriteria.Button Query')}
                        </Button>
                        <Button
                            onClick={onReset}
                            disabled={isBtnDisabled}
                        >
                            {t('searchCriteria.Button Reset')}
                        </Button>
                    </div>
                </SearchBox>
                <Spin spinning={tableSpin} tip="loading...">
                    <AntTableChart
                        tableData={{
                            columns: memoryTableHead,
                            rows: memoryTableData,
                        }}
                        onRowSelected={onRowSelected}
                        current={current}
                        pageSize={pageSize}
                        onCurrentChange={setCurrent}
                        onPageSizeChange={setPageSize}
                        onOrderChange={setOrder}
                        onOrderByChange={setOrderBy}
                        total={total}
                        isCompare={isCompare}
                    />
                </Spin>
            </CollapsiblePanel>
        </Layout>
    );
});
export default MemoryAnalysis;
