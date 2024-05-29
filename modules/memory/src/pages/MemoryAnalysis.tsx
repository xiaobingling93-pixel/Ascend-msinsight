/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import React, { useState, useEffect } from 'react';
import { useTranslation } from 'react-i18next';
import { observer } from 'mobx-react-lite';
import { AntTableChart } from '../components/AntTableChart';
import { LineChart } from '../components/LineChart';
import { Button, Col, Empty, Input, InputNumber, message, Row, Select, Spin } from 'antd';
import { Session } from '../entity/session';
import { Graph, MemoryCurve, MemoryTableColumn, OperatorDetail, OperatorMemoryCondition } from '../entity/memory';
import { memoryCurveGet, operatorsMemoryGet } from '../utils/RequestUtils';
import { useHit, Label } from '../components/Common';
import styled from '@emotion/styled';
import { GroupRankIdsByHost } from 'lib/CommonUtils';

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

const groupBy = [
    { label: 'Overall', value: 'Overall' },
    { label: 'Stream', value: 'Stream' },
];

// eslint-disable-next-line max-lines-per-function
const MemoryAnalysis = observer(function({ session, isDark }: { session: Session; isDark: boolean }) {
    // 算子表格内存信息
    const [memoryTableData, setMemoryTableData] = useState<OperatorDetail[]>([]);
    // 算子表格表头信息
    const [memoryTableHead, setMemoryTableHead] = useState<MemoryTableColumn[]>([]);
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
    // 监听窗口唤醒状态以重绘echarts
    const [isWakeup, setIsWakeup] = useState<boolean>(false);
    const { t } = useTranslation('memory');

    const onSearchEventOperatorChanged: React.ChangeEventHandler<HTMLInputElement | HTMLTextAreaElement> = (event) => {
        setSearchEventOperatorName(event.target.value as string);
    };

    const [selectedRecord, setSelectedRecord] = useState<OperatorDetail | undefined>();
    const onRowSelected = (record?: OperatorDetail, rowIndex?: number): void => {
        setSelectedRecord(record);
    };

    const onFilterEventMinSizeInputChanged = (value: number | null): void => {
        setMinSize(value as number);
    };

    const onFilterEventMaxSizeInputChanged = (value: number | null): void => {
        setMaxSize(value as number);
    };

    const onSearch = (searchName: string, minimumSize: number, maximumSize: number, resetCurrent = false): void => {
        if (rankIdCondition.value === undefined) {
            return;
        }
        if (maximumSize < minimumSize) {
            message.warning(t('Invalid Size Warning'));
            return;
        }
        let tempCurrent = current;
        if (resetCurrent) {
            tempCurrent = 1; setCurrent(1);
        }
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
        if (order !== undefined) {
            param = { order, orderBy, ...param };
        }

        setTableSpin(true);
        setBtnDisabled(true);
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

    const onRankIdChanged = (value: string): void => {
        setRankIdCondition({ ...rankIdCondition, value });
        setSelectedRange(undefined);
        setSearchEventOperatorName('');
        setCurrent(1);
        setPageSize(10);
        setMinSize(0);
        setMaxSize(1000000);
    };

    const onReset = (): void => {
        setSearchEventOperatorName('');
        setMinSize(0);
        setMaxSize(1000000);
        onSearch('', 0, 1000000);
    };

    const onSelectedRangeChanged = (start: number, end: number): void => {
        if (start > end || !memoryCurveData) {
            setSelectedRange(undefined);
            return;
        }
        const allDataSet = new Set(memoryCurveData.lines
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
        onSearch(searchEventOperatorName, minSize, maxSize);
    }, [selectedRange, rankIdCondition.value, current, pageSize, order, orderBy, session.isClusterMemoryCompletedSwitch, groupId]);

    useEffect(() => {
        if (rankIdCondition.value === undefined) {
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
    }, [rankIdCondition, session.isClusterMemoryCompletedSwitch, groupId, t]);

    useEffect(() => {
        const { hosts, ranks } = GroupRankIdsByHost(session.memoryRankIds);
        setHostCondition({ options: hosts, value: hosts[0] ?? '', ranks });
    }, [JSON.stringify(session.memoryRankIds)]);

    useEffect(() => {
        // 只对RandId为数字做排序，不能转为数字的字符串则不排序
        const rankIdOptions = JSON.parse(JSON.stringify(hostCondition.ranks?.get(hostCondition.value) ?? [])).sort((a: any, b: any) => Number(a) - Number(b));
        setRankIdCondition({ options: rankIdOptions, value: rankIdOptions[0] });
    }, [hostCondition.options, hostCondition.value, hostCondition.ranks]);

    useEffect(() => {
        setIsWakeup(session.isWakeup);
    }, [session.isWakeup]);

    return (
        <div className="memory-analysis-wrapper">
            <MemoryWrapper>
                <Row style={{ height: 60, alignContent: 'center' }}>
                    {hostCondition.options.length > 0
                        ? <Col span={4}>
                            <Label name={t('searchCriteria.Host')} />
                            <Select
                                value={hostCondition.value}
                                style={{ width: 200 }}
                                onChange={(value: string): void => setHostCondition({ ...hostCondition, value })}
                                options={hostCondition.options.map((host) => {
                                    return { value: host, label: host };
                                })}
                            />
                        </Col>
                        : <div></div>
                    }
                    <Col span={4}>
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
                    </Col>
                    <Col span={4}>
                        <Label name={<span>{t('searchCriteria.GroupBy')}{useHit()}</span>} />
                        <Select
                            value={groupId}
                            style={{ width: 180 }}
                            onChange={(value: string): void => setGroupId(value)}
                            options={groupByOptions}
                        />
                    </Col>
                </Row>
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
                                    isWakeup={isWakeup}
                                />
                                : <Empty image={Empty.PRESENTED_IMAGE_SIMPLE} style={{ marginTop: 160 }} />
                            }

                        </Col>
                    </Row>
                </Spin>
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
                            formatter={value => `${Number(value)}`}
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
                            formatter={value => `${Number(value)}`}
                        />
                    </Col>
                    <Col span={6}>
                        <Button
                            onClick={() => onSearch(searchEventOperatorName, minSize, maxSize, true)}
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
