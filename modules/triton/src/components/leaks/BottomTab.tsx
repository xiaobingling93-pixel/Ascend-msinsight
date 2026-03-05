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
import { DrawerButton, Resizer } from '@insight/lib';
import { type Theme, useTheme } from '@emotion/react';
import { useTranslation } from 'react-i18next';
import { Session } from '@/entity/session';
import { observer } from 'mobx-react';
import styled from '@emotion/styled/macro';

const MARGIN = 38;
const HEIGHT_DEFAULT = 300;

export const BottomTab = ({ session }: { session: Session }): JSX.Element => {
    const { t } = useTranslation('triton');
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

const hiddenList = ['_firstAccessTimestamp', '_lastAccessTimestamp', 'maxAccessInterval', 'lazyUsed', 'delayedFree', 'longIdle', 'path'];
const SliceDetailItem = styled.div`
    font-size: 14px;
    display: flex;
    color: ${(props): string => props.theme.tableTextColor};
    .sliceDetailName {
        width: 200px;
        font-weight: bold;
    }
    .sliceDetailValue {
        white-space: pre-wrap;
        flex: 1;
    }
`;
const SliceDetail = observer(({ session }: { session: Session }): JSX.Element => {
    const [detailList, setDetailList] = useState<Array<{ key: string; value: string }>>([]);

    useEffect(() => {
        if (session.leaksWorkerInfo.clickItem === null) {
            setDetailList([]);
            return;
        }
        const result: Array<{ key: string; value: string }> = [];
        Object.entries(session.leaksWorkerInfo.clickItem).forEach(([key, value]) => {
            if (hiddenList.includes(key)) {
                return;
            }
            result.push({ key, value });
        });
        setDetailList(result);
    }, [session.leaksWorkerInfo.clickItem]);

    useEffect(() => {
        if (session.stateWorkerInfo.clickItem === null) {
            setDetailList([]);
            return;
        }
        const { type, data: { blocks } } = session.stateWorkerInfo.clickItem;
        if (type === 'block') {
            const result: Array<{ key: string; value: string }> = [];
            Object.entries(blocks[0]).forEach(([key, value]) => {
                if (hiddenList.includes(key)) {
                    return;
                }
                result.push({ key, value });
            });
            setDetailList(result);
        }
    }, [session.stateWorkerInfo.clickItem]);

    return <>
        {
            detailList.map(item => (<SliceDetailItem key={item.key} >
                <div className="sliceDetailName">{item.key}</div>
                <div className="sliceDetailValue">{item.value}</div>
            </SliceDetailItem>))
        }
    </>;
});
