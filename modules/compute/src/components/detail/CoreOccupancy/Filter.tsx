/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { Select } from 'ascend-components';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import { getUsableVal, FormItem } from 'ascend-utils';
import type { optionDataType, optionMapDataType } from '../../../utils/interface';

export type ShowAs = 'cycles' | 'throughput' | 'cacheHitRate';
export interface ICondition {
    showAs: ShowAs;
};
interface IComProps {
    optionMap: optionMapDataType;
    condition: ICondition;
    handleChange: (key: keyof ICondition, val: string) => void;
    t: TFunction;
}

export const defaultCondition: ICondition = {
    showAs: 'cycles',
};
const defaultOptionMap = {
    showAsOptions: [
        { label: 'Cycles', value: 'cycles' },
        { label: 'Throughput(GB/s)', value: 'throughput' },
        { label: 'Cache Hit Rate(%)', value: 'cacheHitRate' },
    ],
};

const getOptionsAndValue = (initObj: ICondition, initOptionMap: optionMapDataType, t: any):
{optionMap: optionMapDataType;condition: ICondition} => {
    // showAs
    const showAsOptions: optionDataType[] = defaultOptionMap.showAsOptions.map(item => ({ ...item, label: t(item.label) }));
    const showAs = getUsableVal(initObj.showAs, showAsOptions, defaultCondition.showAs) as ShowAs;
    return { optionMap: { showAsOptions }, condition: { showAs } };
};

function Filter({ handleFilterChange }: {handleFilterChange: (condition: ICondition) => void}): JSX.Element {
    const [condition, setCondition] = useState<ICondition>(defaultCondition);
    const [optionMap, setOptionMap] = useState<optionMapDataType>(defaultOptionMap);
    const { t: tDetails } = useTranslation('details');
    const handleChange = (key: keyof ICondition, val: string): void => {
        setCondition({ ...condition, [key]: val as any });
    };

    useEffect(() => {
        const { optionMap: newOptionMap, condition: newCondition } = getOptionsAndValue(condition, {}, tDetails);
        setCondition(newCondition);
        setOptionMap(newOptionMap);
    }, [tDetails]);
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
            />
            )}/>
    </div>);
}

export default Filter;
