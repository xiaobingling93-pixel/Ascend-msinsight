/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { useState, useEffect } from 'react';
import { observer } from 'mobx-react-lite';
import { runInAction } from 'mobx';
import { Spin, CollapsiblePanel } from '@insight/lib/components';
import { message } from 'antd';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import { CardInfo, Session } from '../entity/session';
import { MemorySession, MemoryGraphType, GroupBy } from '../entity/memorySession';
import MemoryDetailTableFilter from './MemoryDetailTableFilter';
import { AntTableChart, TableByComponent } from './AntTableChart';
import { MemorySizeQueryCondition, OperatorMemoryCondition, StaticMemoryCondition } from '../entity/memory';
import { fetchDynamicOperatorMaxMin, fetchStaticOperatorMaxMin, operatorsMemoryGet, staticOpMemoryListGet } from '../utils/RequestUtils';
import { customConsole as console } from '@insight/lib/utils';
import { SortOrder } from 'antd/lib/table/interface';

const enum CompareSource {
    DIFFERENCE = 'Difference',
    BASELINE = 'Baseline',
    COMPARISON = 'Comparison',
};

const setSelectedRecord = (memorySession: MemorySession, record?: any): void => {
    const memoryType: string = memorySession.memoryType;
    runInAction(() => {
        switch (memoryType) {
            case MemoryGraphType.DYNAMIC:
                memorySession.selectedRecord = record;
                break;
            case MemoryGraphType.STATIC:
                memorySession.selectedStaticRecord = record;
                break;
            default:
                break;
        };
    });
};

const buildDynamicSearchParam = (memorySession: MemorySession, tempCurrent: number, isCompare: boolean): OperatorMemoryCondition => {
    const rankValue = memorySession.getSelectedRankValue();
    const param: OperatorMemoryCondition = {
        rankId: rankValue.rankInfo.rankId,
        dbPath: rankValue.dbPath,
        type: memorySession.groupId,
        currentPage: tempCurrent,
        pageSize: memorySession.pageSize,
        searchName: memorySession.searchEventOperatorName,
        minSize: memorySession.minSize,
        maxSize: memorySession.maxSize,
        isOnlyShowAllocatedOrReleasedWithinInterval: memorySession.isOnlyShowAllocatedOrReleasedWithinInterval,
        isCompare,
    };
    if (memorySession.selectedRange) {
        param.startTime = memorySession.selectedRange.startTs;
        param.endTime = memorySession.selectedRange.endTs;
    };
    return param;
};

const buildStaticSearchParam = (memorySession: MemorySession, tempCurrent: number, isCompare: boolean): StaticMemoryCondition => {
    const rankValue = memorySession.getSelectedRankValue();
    const param: StaticMemoryCondition = {
        rankId: rankValue.rankInfo.rankId,
        dbPath: rankValue.dbPath,
        graphId: memorySession.memoryGraphId,
        currentPage: tempCurrent,
        pageSize: memorySession.pageSize,
        searchName: memorySession.searchEventOperatorName,
        minSize: memorySession.minSize,
        maxSize: memorySession.maxSize,
        isCompare,
    };
    if (memorySession.staticSelectedRange) {
        param.startNodeIndex = memorySession.staticSelectedRange.startTs;
        param.endNodeIndex = memorySession.staticSelectedRange.endTs;
    };
    return param;
};

const buildSearchParam = (memorySession: MemorySession, tempCurrent: number, isCompare: boolean): any => {
    const memoryType = memorySession.memoryType;
    switch (memoryType) {
        case MemoryGraphType.DYNAMIC:
            return buildDynamicSearchParam(memorySession, tempCurrent, isCompare);
        case MemoryGraphType.STATIC:
            return buildStaticSearchParam(memorySession, tempCurrent, isCompare);
        default:
            return buildDynamicSearchParam(memorySession, tempCurrent, isCompare);
    };
};

const getFetchApi = (memoryType: string): any => {
    switch (memoryType) {
        case MemoryGraphType.DYNAMIC:
            return operatorsMemoryGet;
        case MemoryGraphType.STATIC:
            return staticOpMemoryListGet;
        default:
            return operatorsMemoryGet;
    };
};

const getFetchSizeApi = (memoryType: string): any => {
    switch (memoryType) {
        case MemoryGraphType.DYNAMIC:
            return fetchDynamicOperatorMaxMin;
        case MemoryGraphType.STATIC:
            return fetchStaticOperatorMaxMin;
        default:
            return fetchDynamicOperatorMaxMin;
    };
};

const buildSearchSizeParam = (memorySession: MemorySession, isCompare: boolean): MemorySizeQueryCondition => {
    const selectedCard = memorySession.getSelectedRankValue();
    const param: MemorySizeQueryCondition = { rankId: selectedCard.rankInfo.rankId, dbPath: selectedCard.dbPath, type: memorySession.groupId, isCompare };
    switch (memorySession.memoryType) {
        case MemoryGraphType.STATIC:
            param.graphId = memorySession.memoryGraphId;
            break;
        default:
            break;
    };
    return param;
};

export const handleOperatorDetails = (operatorDetails: any[], isCompare: boolean, t: TFunction): any => {
    return isCompare
        ? operatorDetails.map(item => {
            if (item.diff === undefined || item.diff === null) {
                return item;
            }
            item.diff.source = t(CompareSource.DIFFERENCE);
            item.diff.children = [{ ...item.baseline, source: t(CompareSource.BASELINE) }, { ...item.compare, source: t(CompareSource.COMPARISON) }];
            return item.diff;
        })
        : operatorDetails.map(item => item.compare);
};

const MemoryDetailTable = observer(({ session, memorySession }:
{ session: Session; memorySession: MemorySession }) => {
    const isCompare: boolean = session.compareRank.isCompare;
    const memoryType: string = memorySession.memoryType;
    // 算子表格内存信息
    const [memoryTableData, setMemoryTableData] = useState<any>([]);
    // 算子表格表头信息
    const [memoryTableHead, setMemoryTableHead] = useState<any>([]);
    const [tableSpin, setTableSpin] = useState<boolean>(false);
    const [total, setTotal] = useState<number>(0);
    const { t } = useTranslation('memory');

    const onRowSelected = (record?: any, rowIndex?: number): void => {
        setSelectedRecord(memorySession, record);
    };

    const handlePageChanged = (newCurrent: number, newPageSize: number): void => {
        runInAction(() => {
            memorySession.current = newCurrent;
            memorySession.pageSize = newPageSize;
        });
    };

    const handleOrderChange = (newOrder: SortOrder): void => {
        runInAction(() => {
            memorySession.order = newOrder;
        });
    };

    const handleOrderByChange = (newOrderBy: string): void => {
        runInAction(() => {
            memorySession.orderBy = newOrderBy;
        });
    };

    const preParamCheck = (): boolean => {
        const isRankIdConditionInvalid = memorySession.selectedRankId === '';
        const isStaticMemoryInvalid = memorySession.memoryType === MemoryGraphType.STATIC && memorySession.memoryGraphId === '';
        if (isRankIdConditionInvalid || isStaticMemoryInvalid) {
            setMemoryTableData([]);
            setTotal(0);
            runInAction(() => {
                memorySession.isBtnDisabled = true;
                memorySession.current = 1;
                memorySession.pageSize = 10;
                memorySession.minSize = 0;
                memorySession.maxSize = 0;
            });
            return false;
        };
        if (memorySession.maxSize < memorySession.minSize) {
            message.warning(t('Invalid Size Warning'));
            return false;
        };
        return true;
    };

    const setTempCurrent = (resetCurrent = false): number => {
        let tempCurrent = memorySession.current;
        if (resetCurrent) {
            tempCurrent = 1;
            runInAction(() => {
                memorySession.current = 1;
            });
        }
        return tempCurrent;
    };

    const setParamOtherCondition = (param: any): any => {
        let newParam = param;
        if (memorySession.order !== null) {
            newParam = { order: memorySession.order, orderBy: memorySession.orderBy, ...param };
        };
        setTableSpin(true);
        runInAction(() => {
            memorySession.isBtnDisabled = true;
        });
        return newParam;
    };

    const fetchDetailTableData = (param: any): void => {
        const fetchApi = getFetchApi(memorySession.memoryType);
        fetchApi(param).then((resp: any) => {
            const tableDataDetails = resp.operatorDetail;
            setTotal(resp.totalNum);
            setMemoryTableData(handleOperatorDetails(tableDataDetails, isCompare, t));
            if (JSON.stringify(memoryTableHead) !== JSON.stringify(resp.columnAttr)) {
                setMemoryTableHead(resp.columnAttr);
            }
        }).catch((err: any) => {
            console.error(err);
        }).finally(() => {
            setTableSpin(false);
            runInAction(() => {
                memorySession.isBtnDisabled = false;
            });
        });
    };

    const setDetailTableData = (resetCurrent = false): void => {
        if (!preParamCheck() || memorySession.groupId === GroupBy.COMPONENT) {
            return;
        };
        const tempCurrent = setTempCurrent(resetCurrent);
        let param = buildSearchParam(memorySession, tempCurrent, isCompare);
        param = setParamOtherCondition(param);
        fetchDetailTableData(param);
    };

    const selectedCard = React.useMemo(() => {
        const rankValue = memorySession.getSelectedRankValue();
        return {
            cardId: memorySession.selectedRankId,
            dbPath: rankValue.dbPath,
            index: rankValue.index,
        } as CardInfo;
    }, [memorySession.selectedRankId]);

    // 动静图切换时重置排序
    useEffect(() => {
        runInAction(() => {
            memorySession.order = null;
            memorySession.orderBy = undefined;
        });
    }, [memoryType]);

    useEffect(() => {
        if (memorySession.selectedRankId === '') {
            setDetailTableData();
            return;
        }
        const fetchSizeApi = getFetchSizeApi(memorySession.memoryType);
        const params = buildSearchSizeParam(memorySession, isCompare);
        fetchSizeApi(params).then((res: { minSize: number; maxSize: number }) => {
            runInAction(() => {
                memorySession.defaultMinSize = res.minSize ?? 0;
                memorySession.defaultMaxSize = res.maxSize ?? 0;
                memorySession.minSize = memorySession.defaultMinSize;
                memorySession.maxSize = memorySession.defaultMaxSize;
                setDetailTableData();
            });
        }).catch((err: any) => {
            console.error(err);
        });
    }, [memorySession.selectedRankId, memorySession.groupId, memorySession.memoryGraphId, isCompare, memoryType]);

    useEffect(() => {
        setDetailTableData();
    }, [memorySession.selectedRange, memorySession.staticSelectedRange, memorySession.current, memorySession.pageSize,
        session.isAllMemoryCompletedSwitch, memorySession.order, memorySession.orderBy, t]);
    return (
        <CollapsiblePanel title={t('Memory Allocation/Release Details')} secondary>
            {memorySession.groupId === GroupBy.COMPONENT
                ? <TableByComponent session={session} />
                : <>
                    <MemoryDetailTableFilter session={session} memorySession={memorySession} queryDetailData={setDetailTableData}></MemoryDetailTableFilter>
                    <Spin spinning={tableSpin}>
                        <AntTableChart
                            tableData={{
                                columns: memoryTableHead,
                                rows: memoryTableData,
                            }}
                            onRowSelected={onRowSelected}
                            current={memorySession.current}
                            pageSize={memorySession.pageSize}
                            onPageChange={handlePageChanged}
                            onOrderChange={handleOrderChange}
                            onOrderByChange={handleOrderByChange}
                            total={total}
                            isCompare={isCompare}
                            selectedCard={selectedCard}
                        />
                    </Spin>
                </>}
        </CollapsiblePanel>
    );
});

export default MemoryDetailTable;
