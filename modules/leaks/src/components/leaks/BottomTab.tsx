/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

import React, { useEffect, useState } from 'react';
import { Tabs } from 'antd';
import { DrawerButton, Resizer, Button } from '@insight/lib';
import { type Theme, useTheme } from '@emotion/react';
import { useTranslation } from 'react-i18next';
import MemoryTable from '../MemoryTable';
import { Session } from '../../entity/session';
import { observer } from 'mobx-react';
import styled from '@emotion/styled/macro';
import { getSnapshotDetail } from '@/utils/RequestUtils';

const MARGIN = 38;
const HEIGHT_DEFAULT = 300;

export const BottomTab = ({ session }: { session: Session }): JSX.Element => {
    const { t } = useTranslation('leaks');
    const [isExpand, setIsExpand] = useState(false);
    const [containerHeight, setContainerHeight] = useState(HEIGHT_DEFAULT);
    const theme: Theme = useTheme();

    const changeHeight = (_: number, moveY: number): void => {
        if (!isExpand) {
            return;
        }
        setContainerHeight(oVal => {
            const newHeight = oVal - moveY;
            if (newHeight < MARGIN) {
                return oVal;
            }
            if (newHeight > window.innerHeight - 70) {
                return oVal;
            }
            return newHeight;
        });
    };

    const TabItems = [
        {
            label: t('sliceDetail'),
            key: 'sliceDetail',
            children: <TabContentWrapper height={containerHeight}><SliceDetail session={session} /></TabContentWrapper>,
        },
        {
            label: t('systemView'),
            key: 'systemView',
            children: <TabContentWrapper height={containerHeight}><MemoryTable session={session} /></TabContentWrapper>,
        },
    ];

    return <div
        style={{
            position: 'relative',
            overflow: 'hidden',
            background: theme.backgroundColor,
            height: isExpand ? containerHeight : MARGIN,
        }}>
        <Resizer
            style={{ width: '100%', height: 3, cursor: isExpand ? 'n-resize' : 'auto', zIndex: 1 }}
            callback={changeHeight}
        />
        <DrawerButton
            isExpand={isExpand} onClick={() => setIsExpand(oVal => !oVal)}
            style={{ position: 'absolute', top: 0, left: '50%', transform: 'translateX(-50%)', zIndex: 1 }}
        />
        <Tabs defaultActiveKey="sliceDetail" size="small" items={TabItems} tabBarStyle={{ padding: '0 25px' }} />
    </div>;
};

const TabContentWrapper = ({ children, height }: { children: React.ReactNode; height: number }): JSX.Element => {
    return <div style={{ overflowY: 'auto', height: height - 50, padding: '0 25px' }}>
        {children}
    </div>;
};

const SliceDetailItem = styled.div`
    font-size: 14px;
    display: flex;
    color: ${(props): string => props.theme.tableTextColor};
    .sliceDetailName {
        width: 220px;
        font-weight: bold;
    }
    .sliceDetailValue {
        white-space: pre-wrap;
        flex: 1;
    }
`;

const NoData = styled.div`
    font-size: 14px;
    color: ${(props): string => props.theme.tableTextColor};
`;
const hiddenList = ['_firstAccessTimestamp', '_lastAccessTimestamp', '_startTimestamp', '_endTimestamp',
    'maxAccessInterval', 'lazyUsed', 'delayedFree', 'longIdle', 'path'];
const SliceDetail = observer(({ session }: { session: Session }): JSX.Element => {
    const { t } = useTranslation('leaks', { keyPrefix: 'slice' });
    const [detailList, setDetailList] = useState<Array<{ key: string; value: any }>>([]);
    const [noData, setNoData] = useState(false);

    const getSnapshotDetailInfo = async (type: string, id: number): Promise<void> => {
        const data = await getSnapshotDetail({ type, id });
        const result: Array<{ key: string; value: any }> = [];
        Object.entries(data).forEach(([key, value]) => {
            if (typeof value === 'object') {
                result.push({ key, value: <SliceDetailObjectItem data={value} /> });
            } else {
                result.push({ key, value });
            }
        });
        setDetailList(result);
    };

    useEffect(() => {
        setNoData(false);
        if (session.module === 'leaks') {
            const result: Array<{ key: string; value: any }> = [];
            if (session.leaksWorkerInfo.clickItem === null) {
                return;
            }
            Object.entries(session.leaksWorkerInfo.clickItem).forEach(([key, value]) => {
                if (hiddenList.includes(key)) {
                    return;
                }
                result.push({ key, value });
            });
            setDetailList(result);
        } else {
            const id = session.leaksWorkerInfo.clickItem?.id;
            if (id === undefined) {
                setDetailList([]);
                return;
            }
            getSnapshotDetailInfo('block', id);
        }
    }, [session.leaksWorkerInfo.clickItem]);

    useEffect(() => {
        setNoData(false);
        if (session.stateWorkerInfo.clickItem === null) {
            setDetailList([]);
            return;
        }
        const { type, data } = session.stateWorkerInfo.clickItem;
        const id = type === 'segment' ? data.allocOrMapEventId : (data.blocks[0]?.id ?? -1);
        if (id < 0) {
            setDetailList([]);
            setNoData(true);
            return;
        }
        getSnapshotDetailInfo(type === 'segment' ? 'event' : 'block', id);
    }, [session.stateWorkerInfo.clickItem]);

    useEffect(() => {
        setNoData(false);
        if (session.clickEventItem === null) {
            setDetailList([]);
            return;
        }
        getSnapshotDetailInfo('event', session.clickEventItem.id);
    }, [session.clickEventItem]);

    useEffect(() => {
        setNoData(false);
        setDetailList([]);
    }, [session.deviceId]);

    return <>
        {
            noData
                ? <NoData>{(t('noData', { returnObjects: true }) as string[]).map((item, index) => <div key={index}>{item}</div>)}</NoData>
                : detailList.map(item => (<SliceDetailItem key={item.key} >
                    <div className="sliceDetailName">{t(item.key)}</div>
                    <div className="sliceDetailValue">{item.value}</div>
                </SliceDetailItem>))
        }
    </>;
});
const SliceDetailObjectItem = observer(({ data }: { data: { [key: string]: any } }): JSX.Element => {
    const { t } = useTranslation('leaks', { keyPrefix: 'slice' });
    const [isExpand, setIsExpand] = useState(false);
    const dataList: Array<{ key: string; value: string }> = [];
    Object.entries(data).forEach(([key, value]) => {
        if (typeof value === 'object') {
            dataList.push({ key, value: JSON.stringify(value) });
        } else {
            dataList.push({ key, value });
        }
    });

    return <>
        <Button type="link" size="small" style={{ padding: 0, minWidth: 0 }}
            disabled={dataList.length < 1} onClick={() => setIsExpand(oVal => !oVal)} >
            {`${isExpand ? '-' : '+'} ${t('detail')}`}
        </Button>
        {
            isExpand && dataList.map(item => (<SliceDetailItem key={item.key} >
                <div className="sliceDetailName">{t(item.key)}</div>
                <div className="sliceDetailValue">{item.value}</div>
            </SliceDetailItem>))
        }
    </>;
});
