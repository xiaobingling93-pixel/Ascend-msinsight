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
import { observer } from 'mobx-react';
import React, { type ReactElement, useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Select } from '@insight/lib/components';
import { Label } from '../Common';
import type { optionDataType, optionMapDataType } from '../../utils/interface';
import type { Session } from '../../entity/session';

interface ConditionType {
    core: string ;
    source: string;
};
const defaultCondition = {
    core: '',
    source: '',
};
const defaultOptionMap = {
    coreOptions: [],
    sourceOptions: [],
};

const getOptionsAndValue = (initObj: ConditionType, initOptionMap: optionMapDataType):
{optionMap: optionMapDataType;condition: ConditionType} => {
    // core
    const coreOptions: optionDataType[] = initOptionMap.coreOptions;
    const core = getUsableVal(initObj.core, coreOptions, defaultCondition.core) as string;
    // source
    const sourceOptions: optionDataType[] = initOptionMap.sourceOptions;
    const source = getUsableVal(initObj.source, sourceOptions, defaultCondition.source) as string;

    return { optionMap: { coreOptions, sourceOptions }, condition: { core, source } };
};

export function getUsableVal<T>(val: T, options: Array<{value: T}>, defaultVal: T): T {
    if (options.length === 0) {
        return defaultVal;
    }
    if (options.find(item => item.value === val)) {
        return val;
    }
    return options[0].value;
};

const Filter = observer(({ session, handleFilterChange }:
{session: Session;handleFilterChange: (condition: ConditionType) => void}) => {
    const [condition, setCondition] = useState(defaultCondition);
    const [optionMap, setOptionMap] = useState<optionMapDataType>(defaultOptionMap);

    const handleChange = (key: keyof ConditionType, val: string): void => {
        setCondition({ ...condition, [key]: val });
    };
    useEffect(() => {
        handleFilterChange(condition);
    }, [condition]);

    useEffect(() => {
        if (!session.parseStatus) {
            setCondition(defaultCondition);
            setOptionMap(defaultOptionMap);
        }
    }, [session.parseStatus]);

    useEffect(() => {
        const coreOptions = session.coreList.map((item, index) => ({ label: item, value: item }));
        const sourceOptions = session.sourceList.map(item => ({ label: item, value: item }));
        const { optionMap: newOptionMap, condition: newCondition } = getOptionsAndValue(condition, { coreOptions, sourceOptions });
        setCondition(newCondition);
        setOptionMap(newOptionMap);
    }, [session.coreList, session.sourceList]);
    return (<FilterCom handleChange={handleChange} condition={condition} optionMap={optionMap}/>);
});

interface Iprops {
    optionMap: optionMapDataType;
    condition: ConditionType;
    handleChange: (key: keyof ConditionType, val: string) => void;
}
function FilterCom({ condition, optionMap, handleChange }: Iprops): JSX.Element {
    const { t } = useTranslation('source');
    return (<div>
        <FormItem
            name={t('Core')}
            style={{ width: 'calc(30% - 40px)', minWidth: '270px' }}
            content={(<Select
                id={'coreSelect'}
                value={condition.core}
                style={{ width: 'calc(100% - 100px)' }}
                onChange={(val: string): void => {
                    handleChange('core', val);
                }}
                options={optionMap.coreOptions}
                showSearch={true}
            />
            )}/>
        <FormItem
            name={t('Source')}
            style={{ width: 'calc(70% - 300px)', minWidth: '700px' }}
            content={(<Select
                id={'sourceSelect'}
                value={condition.source}
                style={{ width: 'calc(100% - 100px)' }}
                onChange={(val: string): void => {
                    handleChange('source', val);
                }}
                options={optionMap.sourceOptions}
                showSearch={true}
            />
            )}/>
    </div>);
}

export function FormItem(props: {name: string;style?: React.CSSProperties;content: ReactElement}): JSX.Element {
    return (<div style={{
        display: 'inline-block',
        height: '30px',
        lineHeight: '30px',
        margin: '0 20px 10px 0',
        ...props.style ?? {},
    }}>
        <Label name={props.name} style={{ width: '80px', display: 'inline-block' }}/>
        {props.content}
    </div>);
};

export default Filter;
