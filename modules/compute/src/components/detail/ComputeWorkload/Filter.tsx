/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { Select } from 'ascend-components';
import { FormItem, getUsableVal } from 'ascend-utils';
import type { optionDataType, optionMapDataType } from '../../../utils/interface';
import { limitInput, useHit } from '../../Common';
import { type Session } from '../../../entity/session';

export interface Icondition {
    blockId: string;
    isCompared: boolean;
};
interface Iprops {
    optionMap: optionMapDataType;
    condition: Icondition;
    handleChange: (key: keyof Icondition, val: string) => void;
}

export const defaultCondition = {
    blockId: '',
    isCompared: false,
};
const defaultOptionMap = {
    blockIdOptions: [],
};

const getOptionsAndValue = (initObj: Icondition, initOptionMap: optionMapDataType, isCompared: boolean):
{optionMap: optionMapDataType;condition: Icondition} => {
    // blockId
    const blockIdOptions: optionDataType[] = initOptionMap.blockIdOptions;
    const blockId = getUsableVal(initObj.blockId, blockIdOptions, defaultCondition.blockId) as string;

    return { optionMap: { blockIdOptions }, condition: { blockId, isCompared } };
};

function Filter({ blockIdList, handleFilterChange, session }: {blockIdList: string[];
    handleFilterChange: (condition: Icondition) => void;session: Session;}): JSX.Element {
    const [condition, setCondition] = useState(defaultCondition);
    const [optionMap, setOptionMap] = useState<optionMapDataType>(defaultOptionMap);
    const handleChange = (key: keyof Icondition, val: string): void => {
        setCondition({ ...condition, [key]: val });
    };
    useEffect(() => {
        limitInput();
    }, []);
    useEffect(() => {
        const blockIdOptions = blockIdList.map(item => ({ label: item, value: item }));
        const { optionMap: newOptionMap, condition: newCondition } = getOptionsAndValue(condition, { blockIdOptions }, session.dirInfo.isCompare);
        setCondition(newCondition);
        setOptionMap(newOptionMap);
    }, [blockIdList, session.dirInfo.isCompare]);
    useEffect(() => {
        handleFilterChange(condition);
    }, [condition]);
    return (<FilterCom handleChange={handleChange} condition={condition} optionMap={optionMap}/>);
}

function FilterCom({ condition, optionMap, handleChange }: Iprops): JSX.Element {
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
                id={'compute_block_id'}
            />
            )}/>
    </div>);
}

export default Filter;
