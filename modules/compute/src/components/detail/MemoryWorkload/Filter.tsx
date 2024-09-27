/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { Select } from 'ascend-components';
import { useTranslation } from 'react-i18next';
import { getUsableVal, FormItem } from 'ascend-utils';
import type { optionDataType, optionMapDataType } from '../../../utils/interface';
import { limitInput, useHit } from '../../Common';

export interface Icondition {
    blockId: string ;
    showAs: string;
    isCompared: boolean;
};
interface IcomProps {
    optionMap: optionMapDataType;
    condition: Icondition;
    handleChange: (key: keyof Icondition, val: string) => void;
    t: any;
}

export const defaultCondition = {
    blockId: '',
    showAs: 'bandwidth',
    isCompared: false,
};
const defaultOptionMap = {
    blockIdOptions: [],
    showAsOptions: [
        { label: 'Bandwidth', value: 'bandwidth' },
        { label: 'Num of Request', value: 'request' },
    ],
};

const getOptionsAndValue = (initObj: Icondition, initOptionMap: optionMapDataType, t: any, isCompared: boolean):
{optionMap: optionMapDataType;condition: Icondition} => {
    // blockId
    const blockIdOptions: optionDataType[] = initOptionMap.blockIdOptions;
    const blockId = getUsableVal(initObj.blockId, blockIdOptions, defaultCondition.blockId) as string;

    // showAs
    const showAsOptions: optionDataType[] = defaultOptionMap.showAsOptions.map(item => ({ ...item, label: t(item.label) }));
    const showAs = getUsableVal(initObj.showAs, showAsOptions, defaultCondition.showAs) as string;
    return { optionMap: { blockIdOptions, showAsOptions }, condition: { blockId, showAs, isCompared } };
};

function Filter({ blockIdList, handleFilterChange, isCompared }: {blockIdList: string[];
    handleFilterChange: (condition: Icondition) => void;isCompared: boolean;}): JSX.Element {
    const [condition, setCondition] = useState(defaultCondition);
    const [optionMap, setOptionMap] = useState<optionMapDataType>(defaultOptionMap);
    const { t: tDetails } = useTranslation('details');
    const handleChange = (key: keyof Icondition, val: string): void => {
        setCondition({ ...condition, [key]: val });
    };
    useEffect(() => {
        limitInput();
    }, []);
    useEffect(() => {
        const blockIdOptions = blockIdList.map((item, index) => ({ label: item, value: item }));
        const { optionMap: newOptionMap, condition: newCondition } = getOptionsAndValue(condition, { blockIdOptions }, tDetails, isCompared);
        setCondition(newCondition);
        setOptionMap(newOptionMap);
    }, [blockIdList, tDetails, isCompared]);
    useEffect(() => {
        handleFilterChange(condition);
    }, [condition]);
    return (<FilterCom handleChange={handleChange} condition={condition} optionMap={optionMap} t={tDetails}/>);
}
function FilterCom({ condition, optionMap, handleChange, t }: IcomProps): JSX.Element {
    return (<div>
        <FormItem
            name={<><span>Block ID</span>{useHit()}</>}
            nameStyle={{ width: '90px' }}
            content={(<Select
                value={condition.blockId}
                style={{ width: '150px' }}
                onChange={(val: string): void => {
                    handleChange('blockId', val);
                }}
                options={optionMap.blockIdOptions}
                showSearch={true}
            />
            )}/>
        <FormItem
            name={t('Show As')}
            nameStyle={{ width: '70px' }}
            content={(<Select
                value={condition.showAs}
                style={{ width: '170px' }}
                onChange={(val: string): void => {
                    handleChange('showAs', val);
                }}
                options={optionMap.showAsOptions}
                showSearch={true}
            />
            )}/>
    </div>);
}

export default Filter;
