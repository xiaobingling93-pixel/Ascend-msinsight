/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import React, { useState, useEffect } from 'react';
import { observer } from 'mobx-react-lite';
import { runInAction } from 'mobx';
import CollapsiblePanel from 'ascend-collapsible-panel';
import { Spin } from 'ascend-components';
import { useTranslation } from 'react-i18next';
import { Session } from '../entity/session';
import { CurveSession } from '../entity/curveSession';
import { AntTableChart } from './AntTableChart';
import { TableInfo, TableCondition } from '../entity/curve';
import { tableDataGet } from '../utils/RequestUtils';
import { customConsole as console } from 'ascend-utils';

const buildDynamicSearchParam = (memorySession: CurveSession, tempCurrent: number): any => {
    const param: TableCondition = {
        rankId: memorySession.rankIdCondition.value,
        type: memorySession.groupId,
        currentPage: tempCurrent,
        pageSize: memorySession.pageSize,
    };
    if (memorySession.selectedRange) {
        param.startTime = memorySession.selectedRange.startTs;
        param.endTime = memorySession.selectedRange.endTs;
    }
    return param;
};

const CurveDetailTable = observer(({ session, curveSession }:
{ session: Session; curveSession: CurveSession }) => {
    // 算子表格内存信息
    const [memoryTableData, setMemoryTableData] = useState<any>([]);
    // 算子表格表头信息
    const [memoryTableHead, setMemoryTableHead] = useState<any>([]);
    const [tableSpin, setTableSpin] = useState<boolean>(false);
    const [total, setTotal] = useState<number>(0);
    const [orderBy, setOrderBy] = useState<string | undefined>(undefined);
    const [order, setOrder] = useState<string | undefined>(undefined);
    const { t } = useTranslation('statistic');

    const onRowSelected = (record?: any, rowIndex?: number): void => {
        curveSession.selectedRecord = record;
    };

    const handlePageChanged = (newCurrent: number, newPageSize: number): void => {
        runInAction(() => {
            curveSession.current = newCurrent;
            curveSession.pageSize = newPageSize;
        });
    };

    const preParamCheck = (): boolean => {
        const isRankIdConditionInvalid = curveSession.rankIdCondition.value === undefined || curveSession.rankIdCondition.value === '';
        if (isRankIdConditionInvalid) {
            setMemoryTableData([]);
            setTotal(0);
            runInAction(() => {
                curveSession.isBtnDisabled = true;
                curveSession.current = 1;
                curveSession.pageSize = 10;
            });
            return false;
        };
        return true;
    };

    const setTempCurrent = (resetCurrent = false): number => {
        let tempCurrent = curveSession.current;
        if (resetCurrent) {
            tempCurrent = 1;
            runInAction(() => {
                curveSession.current = 1;
            });
        }
        return tempCurrent;
    };

    const setParamOtherCondition = (param: any): any => {
        let newParam = param;
        if (order !== undefined) {
            newParam = { order, orderBy, ...param };
        };
        setTableSpin(true);
        runInAction(() => {
            curveSession.isBtnDisabled = true;
        });
        return newParam;
    };

    const fetchDetailTableData = (param: any): void => {
        tableDataGet(param).then((resp: TableInfo) => {
            const tableDataDetails = resp.operatorDetail;
            setTotal(resp.totalNum);
            setMemoryTableData(tableDataDetails);
            if (JSON.stringify(memoryTableHead) !== JSON.stringify(resp.columnAttr)) {
                setMemoryTableHead(resp.columnAttr);
            }
        }).catch((err: any) => {
            console.error(err);
        }).finally(() => {
            setTableSpin(false);
            runInAction(() => {
                curveSession.isBtnDisabled = false;
            });
        });
    };

    const setDetailTableData = (resetCurrent = false): void => {
        if (!preParamCheck()) {
            return;
        };
        const tempCurrent = setTempCurrent(resetCurrent);
        let param = buildDynamicSearchParam(curveSession, tempCurrent);
        param = setParamOtherCondition(param);
        fetchDetailTableData(param);
    };

    useEffect(() => {
        setOrder(undefined);
        setOrderBy(undefined);
        setDetailTableData();
    }, [curveSession.rankIdCondition.value, curveSession.groupId]);
    useEffect(() => {
        setDetailTableData();
    }, [curveSession.selectedRange, curveSession.current, curveSession.pageSize, order, orderBy, t]);

    return (
        <CollapsiblePanel title={t('Table Data Details')} secondary>
            {
                <>
                    <Spin spinning={tableSpin} tip={t('Loading')}>
                        <AntTableChart
                            tableData={{
                                columns: memoryTableHead,
                                rows: memoryTableData,
                            }}
                            onRowSelected={onRowSelected}
                            current={curveSession.current}
                            pageSize={curveSession.pageSize}
                            onPageChange={handlePageChanged}
                            onOrderChange={setOrder}
                            onOrderByChange={setOrderBy}
                            total={total}
                            rankId={curveSession.rankIdCondition.value}
                            groupName={curveSession.groupId}
                        />
                    </Spin>
                </>}
        </CollapsiblePanel>
    );
});

export default CurveDetailTable;
