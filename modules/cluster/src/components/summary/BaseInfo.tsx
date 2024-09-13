/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import React, { useEffect, useState } from 'react';
import { runInAction } from 'mobx';
import { useTranslation } from 'react-i18next';
import { formatDate } from '../Common';
import type { StringMap } from '../../utils/interface';
import type { Session } from '../../entity/session';
import { queryTopSummary } from '../../utils/RequestUtils';
import { defaultConditions } from './Filter';
import CollapsiblePanel from 'ascend-collapsible-panel';
import { MIDescriptions, MIDescriptionsItem } from 'ascend-utils';

export interface BaseInfoDataType {
    [prop: string]: any;
    collectDuration: string | number;
}

interface ListItem {
    label: string;
    key: string;
    value?: string;
}

export const defaultBaseInfo = {
    filePath: '',
    dataSize: '',
    collectStartTime: '',
    rankCount: '',
    stepNum: '',
    collectDuration: '',
};

const useList = (): ListItem[] => {
    const { t } = useTranslation('summary');
    return [
        {
            label: t('ReportFile'),
            key: 'filePath',
            value: '',
        },
        {
            label: `${t('ReportSize')}(MB)`,
            key: 'dataSize',
        },
        {
            label: t('ReportCaptureTime'),
            key: 'collectStartTime',
        },
        {
            label: t('DeviceCount'),
            key: 'rankCount',
        },
        {
            label: t('StepCount'),
            key: 'stepNum',
        },
        {
            label: t('ProfilingSessionDuration'),
            key: 'collectDuration',
        },
    ];
};

const formateTime = (t: number): string => {
    if (isNaN(t)) {
        return '';
    }
    let leftTime = t;
    // 小时级
    if (t >= 1000 * 1000 * 60 * 60) {
        const h = Math.floor(leftTime / (1000 * 1000 * 60 * 60));
        leftTime = leftTime % (1000 * 1000 * 60 * 60);
        const m = Math.floor(leftTime / (1000 * 1000 * 60));
        leftTime = leftTime % (1000 * 1000 * 60);
        const s = Number((leftTime / (1000 * 1000)).toFixed(2));
        return `${h}h${m}m${s}s`;
    }
    // 分钟级
    if (t >= 1000 * 1000 * 60) {
        const m = Math.floor(leftTime / (1000 * 1000 * 60));
        leftTime = leftTime % (1000 * 1000 * 60);
        const s = Number((leftTime / (1000 * 1000)).toFixed(2));
        return `${m}m${s}s`;
    }
    // 秒级
    if (t >= 1000 * 1000) {
        const s = Number((leftTime / (1000 * 1000)).toFixed(2));
        return `${s}s`;
    }
    // 毫秒级
    if (t >= 1000) {
        const s = Number((leftTime / (1000)).toFixed(2));
        return `${s}ms`;
    }
    return `${t}μs`;
};

const initBaseInfo = async (setData: any, session: Session): Promise<void> => {
    const res: any = await queryTopSummary(defaultConditions);
    const resObj = res ?? {};
    runInAction(() => {
        session.rankCount = res.rankCount;
        session.summaryList = res.summaryList;
    });
    setData({
        ...resObj,
        collectDuration: formateTime(Number(resObj.collectDuration)),
        collectStartTime: formatDate(new Date(resObj.collectStartTime)),
        dataSize: resObj.dataSize !== undefined && resObj.dataSize > 0.01 ? Number(resObj.dataSize?.toFixed(2)) : resObj.dataSize,
    });
};

const useDisplayItems = (session: Session): any[] => {
    const list = useList();
    if (session.unitcount === 0) {
        return list.filter(item => !['collectStartTime', 'collectDuration'].includes(item.key));
    }
    return list;
};
const BaseInfo = ({ session }: { session: Session}): JSX.Element => {
    const [data, setData] = useState<StringMap>({});
    const { t } = useTranslation('summary');
    useEffect(() => {
        if (!session.clusterCompleted) {
            setData({});
            return;
        }
        setTimeout(() => {
            initBaseInfo(setData, session);
        });
    }, [session.parseCompleted, session.renderId]);
    const displaylist = useDisplayItems(session);

    return <CollapsiblePanel title={t('BaseInfo')}>
        <MIDescriptions title={''}>
            {
                displaylist.map((item, index) => <MIDescriptionsItem key={index} label={item.label}>
                    { data[item.key] }
                </MIDescriptionsItem>)
            }
        </MIDescriptions>
    </CollapsiblePanel>;
};

export default BaseInfo;
