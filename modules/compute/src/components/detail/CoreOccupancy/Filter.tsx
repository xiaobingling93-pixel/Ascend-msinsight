/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
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
import { Select } from '@insight/lib/components';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import { CustomFormItem as FormItem, getUsableVal } from '@insight/lib/utils';
import type { optionDataType, optionMapDataType } from '../../../utils/interface';
import type { Session } from '../../../entity/session';

export type ShowAs = 'cycles' | 'throughput' | 'cacheHitRate';
export interface ICondition {
    showAs: ShowAs;
    isCompared: boolean;
};
interface IComProps {
    optionMap: optionMapDataType;
    condition: ICondition;
    handleChange: (key: keyof ICondition, val: string) => void;
    t: TFunction;
}

export const defaultCondition: ICondition = {
    showAs: 'cycles',
    isCompared: false,
};
const defaultOptionMap = {
    showAsOptions: [
        { label: 'Cycles', value: 'cycles' },
        { label: 'Throughput', value: 'throughput' },
        { label: 'Cache Hit Rate(%)', value: 'cacheHitRate' },
    ],
};

const getOptionsAndValue = (initObj: ICondition, initOptionMap: optionMapDataType, t: any, isCompared: boolean):
{optionMap: optionMapDataType;condition: ICondition} => {
    // showAs
    const showAsOptions: optionDataType[] = defaultOptionMap.showAsOptions.map(item => ({ ...item, label: t(item.label) }));
    const showAs = getUsableVal(initObj.showAs, showAsOptions, defaultCondition.showAs) as ShowAs;
    return { optionMap: { showAsOptions }, condition: { showAs, isCompared } };
};

function Filter({ handleFilterChange, session }: {handleFilterChange: (condition: ICondition) => void; session: Session}): JSX.Element {
    const [condition, setCondition] = useState<ICondition>(defaultCondition);
    const [optionMap, setOptionMap] = useState<optionMapDataType>(defaultOptionMap);
    const { t: tDetails } = useTranslation('details');
    const handleChange = (key: keyof ICondition, val: string): void => {
        setCondition({ ...condition, [key]: val as any });
    };

    useEffect(() => {
        const { optionMap: newOptionMap, condition: newCondition } = getOptionsAndValue(condition, {}, tDetails, session.dirInfo.isCompare);
        setCondition(newCondition);
        setOptionMap(newOptionMap);
    }, [tDetails, session.dirInfo]);
    useEffect(() => {
        handleFilterChange(condition);
    }, [condition]);
    return (<FilterCom handleChange={handleChange} condition={condition} optionMap={optionMap} t={tDetails}/>);
}
function FilterCom({ condition, optionMap, handleChange, t }: IComProps): JSX.Element {
    return (<div>
        <FormItem
            name={t('Show As')}
            nameStyle={{ width: '70px' }}
            content={(<Select
                value={condition.showAs}
                style={{ width: '160px' }}
                onChange={(val: string): void => {
                    handleChange('showAs', val);
                }}
                options={optionMap.showAsOptions}
                name={'core_show_as'}
                id={'core_show_as'}
            />
            )}/>
    </div>);
}

export default Filter;
