/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { HelpIcon } from '@insight/lib/icon';
import { Tooltip } from '@insight/lib/components';
import { safeStr } from '@insight/lib/utils';
import { useRootStore } from '../context/context';
import { GroupBy } from '../entity/memorySession';

const HOUR_TO_MICROSECOND = 1000 * 1000 * 60 * 60;
const MINUTE_TO_MICROSECOND = 1000 * 1000 * 60;
const SECOND_TO_MICROSECOND = 1000 * 1000;
const MILLISECOND_TO_MICROSECONTD = 1000;

export const Label = (props: {name: React.ReactNode;style?: object }): JSX.Element => {
    return <span style={{ marginRight: 8, ...(props.style ?? {}) }}>{props.name}{' :'} </span>;
};

export const useHit = (): React.ReactElement => {
    const { t } = useTranslation('memory');
    return <Tooltip title={
        (
            <div style={{ padding: '1rem' }}>
                <div>{safeStr(t('searchCriteria.Overall'))}: {safeStr(t('searchCriteria.OverallDescribe'))}</div>
                <div style={{ marginTop: '2rem' }}>{safeStr(t('searchCriteria.Stream'))}: {safeStr(t('searchCriteria.StreamDescribe'))}</div>
                <div style={{ marginTop: '2rem' }}>{safeStr(t('searchCriteria.Component'))}: {safeStr(t('searchCriteria.ComponentDescribe'))}</div>
            </div>
        )
    }>
        <HelpIcon style={{ cursor: 'pointer' }} height={20} width={20}/>
    </Tooltip>;
};

export const useChartCharacter = (): React.ReactElement => {
    const hitGroup: { [key: string]: string } = {
        [GroupBy.DEFAULT]: 'CurveDescribe',
        [GroupBy.COMPONENT]: 'CurveDescribeByCompenent',
    };
    const { t } = useTranslation('memory');
    const { memoryStore } = useRootStore();
    const memorySession = memoryStore.activeSession;
    const [hit, setHit] = useState<[]>();
    useEffect(() => {
        if (memorySession === undefined) {
            return;
        };
        setHit(hitGroup[memorySession.groupId] === undefined
            ? []
            : t(`searchCriteria.${hitGroup[memorySession.groupId]}`, { returnObjects: true }) as [],
        );
    }, [memorySession?.groupId, t]);
    return <Tooltip title={
        <div style={{ padding: '1rem' }}>
            {hit?.map((item: string, index: number) =>
                <div style={{ padding: '3px 0' }} key={index}>{safeStr(item)}</div>)
            }
        </div>
    }>
        <HelpIcon style={{ cursor: 'pointer' }} height={20} width={20}/>
    </Tooltip>;
};

export const convertTime = (time: any): string => {
    if ((typeof time !== 'string' && typeof time !== 'number') || isNaN(parseFloat(`${time}`))) {
        return safeStr(time);
    }

    // 时间原本为ms单位，最小可到小数点后三位即微秒级别，为防止浮点数运算丢失精度，统一转为微秒即整数进行运算
    let timeNum = Math.round(parseFloat(`${time}`) * MILLISECOND_TO_MICROSECONTD);
    let res = '';
    const hour = Math.floor(timeNum / HOUR_TO_MICROSECOND);
    timeNum = timeNum % HOUR_TO_MICROSECOND;
    res = `${res}${hour}`.padStart(2, '0');
    const min = Math.floor(timeNum / MINUTE_TO_MICROSECOND);
    timeNum = timeNum % MINUTE_TO_MICROSECOND;
    res = `${res}:${String(min).padStart(2, '0')}`;
    const second = Math.floor(timeNum / SECOND_TO_MICROSECOND);
    timeNum = timeNum % SECOND_TO_MICROSECOND;
    res = `${res}:${String(second).padStart(2, '0')}`;
    const millisecond = Math.floor(timeNum / MILLISECOND_TO_MICROSECONTD);
    timeNum = timeNum % MILLISECOND_TO_MICROSECONTD;
    res = `${res}.${String(millisecond).padStart(3, '0')}.${String(timeNum).padStart(3, '0')}`;
    return res;
};
