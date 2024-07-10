/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import React, { useState, useEffect } from 'react';
import { useTranslation } from 'react-i18next';
import { observer } from 'mobx-react-lite';
import { AntTableChart } from '../components/AntTableChart';
import { LineChart } from '../components/LineChart';
import { Button, Col, Input, InputNumber, message, Row, Select, Spin } from 'antd';
import { Session } from '../entity/session';
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
import { GroupRankIdsByHost, StyledEmpty } from 'lib/CommonUtils';
interface SelectedRange {
    startTs: number;
    endTs: number;
}

interface ConditionType {
    options: string[];
    value: string;
    ranks?: Map<string, string[]>;
}

const MemoryWrapper = styled.div`
      display: flex;
      flex-direction: column;
      width: 100%;
      height: 100%;
    `;

const FlexDiv = styled.div`
    display: flex;
    align-items: center;
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
const MemoryAnalysis = observer(function({ session, isDark }: { session: Session; isDark: boolean }) {
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
    // 最大内存范围，默认1000000KB
    const [maxSize, setMaxSize] = useState<number>(1000000);
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
            message.error(err);
        });
    };

    const fetchResourceType = (memoryRankId: string | undefined): void => {
        if (memoryRankId === undefined || memoryRankId === '') {
            return;
        }
        resourceTypeGet({ rankId: memoryRankId }).then((resp) => {
            const type = resp.type;
            setResourceType(type);
        }).catch(err => {
            message.error(err);
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

    const onFilterEventMinSizeInputChanged = (value: number | null): void => {
        setMinSize(value as number);
    };

    const onFilterEventMaxSizeInputChanged = (value: number | null): void => {
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
            type: groupId,
            currentPage: tempCurrent,
            pageSize,
            searchName,
            minSize: minimumSize,
            maxSize: maximumSize,
        };
        if (selectedRange) {
            param.startTime = selectedRange.startTs; param.endTime = selectedRange.endTs;
        }
        param = setParamOtherCondition(param);
        operatorsMemoryGet(param).then((resp) => {
            const operatorDetails = resp.operatorDetail;
            setTotal(resp.totalNum);
            setMemoryTableData(operatorDetails);
            if (JSON.stringify(memoryTableHead) !== JSON.stringify(resp.columnAttr)) {
                setMemoryTableHead(resp.columnAttr);
            }
            setBtnDisabled(false);
        }).catch(err => {
            message.error(err);
        }).finally(() => {
            setTableSpin(false);
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
        };
        if (selectedRange) {
            param.startNodeIndex = selectedRange.startTs; param.endNodeIndex = selectedRange.endTs;
        }
        param = setParamOtherCondition(param);
        staticOpMemoryListGet(param).then((resp) => {
            const staticOperatorListDetails = resp.staticOperatorListDetail;
            setTotal(resp.totalNum);
            setMemoryTableData(staticOperatorListDetails);
            if (JSON.stringify(memoryTableHead) !== JSON.stringify(resp.columnAttr)) {
                setMemoryTableHead(resp.columnAttr);
            }
            setBtnDisabled(false);
        }).catch(err => {
            message.error(err);
        }).finally(() => {
            setTableSpin(false);
        });
    };

    const onBaseChanged = (): void => {
        setSelectedRange(undefined);
        setSearchEventOperatorName('');
        setCurrent(1);
        setPageSize(10);
        setMinSize(0);
        setMaxSize(1000000);
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
        setMinSize(0);
        setMaxSize(1000000);
        switch (memoryType) {
            case memoryGraphType.dynamic:
                onSearch('', 0, 1000000);
                break;
            case memoryGraphType.static:
                onStaticSearch('', 0, 1000000);
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
    }, [selectedRange, rankIdCondition.value, current, pageSize, order, orderBy, session.isClusterMemoryCompletedSwitch, groupId, memoryGraphId, t]);

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
        setCurveSpin(true);
        memoryCurveGet({ rankId: rankIdCondition.value, type: groupId }).then((resp) => {
            // Reset the select range to null when rankId changes
            setSelectedRange(undefined);
            setMemoryCurveData(resp);
            setLineChartData({
                title: resp.title,
                columns: resp.legends?.map(legend => t(legend)),
                rows: resp.lines,
            });
        }).catch(err => {
            message.error(err);
        }).finally(() => {
            setCurveSpin(false);
        });
    }, [rankIdCondition.value, groupId, t]);

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
        staticOpMemoryGraphGet({ rankId: rankIdCondition.value, graphId: memoryGraphId }).then((resp) => {
            // Reset the select range to null when rankId changes
            setSelectedRange(undefined);
            setMemoryStaticCurveData(resp);
            setStaticLineChartData({
                columns: resp.legends?.map(legend => t(legend)),
                rows: resp.lines,
            });
        }).catch(err => {
            message.error(err);
        }).finally(() => {
            setStaticCurveSping(false);
        });
    }, [rankIdCondition.value, session.isClusterMemoryCompletedSwitch, memoryGraphId, t]);

    useEffect(() => {
        // 只对RandId为数字做排序，不能转为数字的字符串则不排序
        const rankIdOptions = JSON.parse(JSON.stringify(hostCondition.ranks?.get(hostCondition.value) ?? [])).sort((a: any, b: any) => Number(a) - Number(b));
        setRankIdCondition({ options: rankIdOptions, value: rankIdOptions[0] });
    }, [hostCondition.options, hostCondition.value, hostCondition.ranks]);

    return (
        <div className="memory-analysis-wrapper">
            <MemoryWrapper>
                <FlexDiv style={{ flexWrap: 'wrap', gap: '10px', padding: '14px', alignContent: 'center' }}>
                    {hostCondition.options.length > 0
                        ? <FlexDiv>
                            <Label name={t('searchCriteria.Host')} />
                            <Select
                                value={hostCondition.value}
                                style={{ width: 200 }}
                                onChange={(value: string): void => setHostCondition({ ...hostCondition, value })}
                                options={hostCondition.options.map((host) => {
                                    return { value: host, label: host };
                                })}
                            />
                        </FlexDiv>
                        : <div></div>
                    }
                    <FlexDiv>
                        <Label name={t('searchCriteria.RankId')} />
                        <Select
                            value={rankIdCondition.value}
                            style={{ width: 200 }}
                            onChange={onRankIdChanged}
                            options={rankIdCondition.options.map((rankId) => {
                                return {
                                    value: rankId,
                                    label: rankId.replace(`${hostCondition.value} `, ''),
                                };
                            })}
                        />
                    </FlexDiv>
                    {
                        resourceType === dataResourceType.pytorch
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
                </FlexDiv>
                <Spin spinning={curveSpin} tip="loading...">
                    <Row style={{ height: 400 }}>
                        <Col span={24}>

                            { lineChartData
                                ? <LineChart
                                    hAxisTitle={t('Time (ms)')}
                                    vAxisTitle={t('Memory Usage (MB)')}
                                    graph={lineChartData}
                                    onSelectionChanged={onSelectedRangeChanged}
                                    record={selectedRecord}
                                    isDark={isDark}
                                />
                                : <StyledEmpty style={{ marginTop: 160 }} translation={t}/>
                            }

                        </Col>
                    </Row>
                </Spin>
                { memoryType === memoryGraphType.static
                    ? <div>
                        <Row style={{ height: 60, alignContent: 'center' }}>
                            <Col span={4}>
                                <Label name={t('searchCriteria.GraphId')} />
                                <Select
                                    value={memoryGraphId}
                                    style={{ width: 180 }}
                                    onChange={onMemoryGraphIdChanged}
                                    options={memoryGraphIdList.map((graphId) => {
                                        return {
                                            value: graphId,
                                            label: graphId,
                                        };
                                    })}
                                />
                            </Col>
                        </Row>
                        <Spin spinning={staticCurveSpin} tip="loading...">
                            <Row style={{ height: 400 }}>
                                <Col span={24}>

                                    { staticLineChartData
                                        ? <LineChart
                                            hAxisTitle={t('Node Index')}
                                            vAxisTitle={t('Memory Usage (MB)')}
                                            graph={staticLineChartData}
                                            onSelectionChanged={onSelectedRangeChanged}
                                            record={selectedStaticRecord}
                                            isDark={isDark}
                                        />
                                        : <StyledEmpty style={{ marginTop: 160 }} translation={t}/>
                                    }

                                </Col>
                            </Row>
                        </Spin>
                    </div>
                    : null }
                <Row style={{ height: 60, alignContent: 'center', marginTop: '75px' }}>
                    <Col span={6}>
                        <Label name={t('searchCriteria.Name')} />
                        <Input
                            value={searchEventOperatorName}
                            style={{ width: 200 }}
                            onChange={onSearchEventOperatorChanged}
                            placeholder={t('searchCriteria.Search by Name')}
                            allowClear
                            maxLength={200}
                        />
                    </Col>
                    <Col span={6}>
                        <Label name={t('searchCriteria.Min Size')} />
                        <InputNumber
                            value={minSize}
                            style={{ width: 200 }}
                            onChange={onFilterEventMinSizeInputChanged}
                            min={0}
                            max={4294967295}
                            formatter={(value): string => `${Number(value)}`}
                        />
                    </Col>
                    <Col span={6}>
                        <Label name={t('searchCriteria.Max Size')} />
                        <InputNumber
                            value={maxSize}
                            style={{ width: 200 }}
                            onChange={onFilterEventMaxSizeInputChanged}
                            min={0}
                            max={4294967295}
                            minLength={1}
                            formatter={(value): string => `${Number(value)}`}
                        />
                    </Col>
                    <Col span={6}>
                        <Button
                            onClick={(): void => memoryType === memoryGraphType.dynamic
                                ? onSearch(searchEventOperatorName, minSize, maxSize, true)
                                : onStaticSearch(searchEventOperatorName, minSize, maxSize, true)}
                            type="primary"
                            style={{ marginRight: 10, width: 100 }}
                            disabled={isBtnDisabled}
                        >
                            {t('searchCriteria.Button Query')}
                        </Button>
                        <Button
                            onClick={onReset}
                            style={{ width: 100 }}
                            disabled={isBtnDisabled}
                        >
                            {t('searchCriteria.Button Reset')}
                        </Button>
                    </Col>
                </Row>
                <Row style={{ flex: 1, height: 0 }}>
                    <Col span={24}>
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
                            />
                        </Spin>
                    </Col>
                </Row>
            </MemoryWrapper>
        </div>
    );
});
export default MemoryAnalysis;
