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
import { CustomFormItem as FormItem, getUsableVal } from '@insight/lib/utils';
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
