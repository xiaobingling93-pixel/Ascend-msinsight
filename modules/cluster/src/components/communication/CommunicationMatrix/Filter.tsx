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
import React, { useEffect, useState } from 'react';
import { Label } from '../../Common';
import { Button, Checkbox, InputNumber, Select } from '@insight/lib/components';
import { useTranslation } from 'react-i18next';
import { optionDataType, VoidFunction } from '../../../utils/interface';
import { message } from 'antd';
import type { CheckboxChangeEvent } from 'antd/es/checkbox';

export interface Condition {
    type: MatrixType;
    showInner: boolean;
}

export interface Range {
    min: number;
    max: number;
}

export enum MatrixType {
    BANDWIDTH = 'bandwidth',
    TRANSIT_SIZE = 'transitSize',
    TRANSPORT_TYPE = 'transportType',
    TRANSIT_TIME = 'transitTime',
}

export type MatrixTypeValues = typeof MatrixType[keyof typeof MatrixType];

const useOptions = (): optionDataType[] => {
    const { t } = useTranslation('communication');
    return [
        {
            label: `${t('searchCriteria.Bandwidth')}(GB/s)`,
            value: MatrixType.BANDWIDTH,
        },
        {
            label: `${t('searchCriteria.TransitSize')}(MB)`,
            value: MatrixType.TRANSIT_SIZE,
        },
        {
            label: t('searchCriteria.TransportType'),
            value: MatrixType.TRANSPORT_TYPE,
        },
        {
            label: `${t('searchCriteria.TransitTime')}(ms)`,
            value: MatrixType.TRANSIT_TIME,
        },
    ];
};

interface IProps {
    handleChange: VoidFunction;
    condition: Condition;
    range: Range;
    onRangeChange: VoidFunction;
}

const Filter = observer(({ handleChange, condition, range, onRangeChange }: IProps) => {
    const { t } = useTranslation('communication');
    return <div>
        <Label name={t('searchCriteria.CommunicationMatrixType')}/>
        <Select
            defaultValue="0"
            style={{ width: 200, marginRight: '20px' }}
            onChange={(val: string): void => {
                handleChange('type', val);
            }}
            options={useOptions()}
            value={condition.type}
            id={'communicationMatrixType-select'}
        />
        <Checkbox checked={condition.showInner}
            onChange={(e: CheckboxChangeEvent): void => {
                handleChange('showInner', e.target.checked);
            }}
            data-testid={'showInnerCommunication'}
        >{t('searchCriteria.ShowInnerCommunication')}</Checkbox>
        {condition.type !== MatrixType.TRANSPORT_TYPE && <RangeFilter range={range} changeFilter={onRangeChange} />}
    </div>;
});

const RangeFilter = ({ range, changeFilter }: { range: Range; changeFilter: VoidFunction }): JSX.Element => {
    const { min, max } = range;
    const [minValue, setMinValue] = useState(min);
    const [maxValue, setMaxValue] = useState(max);
    const [isValid, setIsValid] = useState(true);
    const { t } = useTranslation();

    const onConfirm = (): void => {
        if (minValue > maxValue) {
            message.warning('Invalid Range: The start value cannot be greater than the end value.');
            setIsValid(false);
            return;
        }
        changeFilter({ min: minValue, max: maxValue });
    };
    const changeInput = (value: number, type: string): void => {
        setIsValid(true);
        if (type === 'min') {
            setMinValue(value);
        } else {
            setMaxValue(value);
        }
    };
    useEffect(() => {
        setMinValue(min);
        setMaxValue(max);
    }, [JSON.stringify(range)]);
    return (
        <>
            <Label name={t('searchCriteria.VisibleRange', { ns: 'communication' })} style={{ margin: '0 8px 0 24px' }} />
            <InputNumber value={minValue} size="small" style={{ marginRight: 8 }} min={min} max={max}
                onChange={(value: string | number | null): void => changeInput(value as number, 'min')} status={isValid ? '' : 'error'} step={0.1} data-testid={'communicationMatrixMinRangeInput'} />
            ~
            <InputNumber value={maxValue} size="small" style={{ margin: '0 32px 0 8px' }} min={min} max={max}
                onChange={(value: string | number | null): void => changeInput(value as number, 'max')} status={isValid ? '' : 'error'} step={0.1} data-testid={'communicationMatrixMaxRangeInput'} />
            <Button onClick={onConfirm} type="primary" size="middle">{t('Confirm', { ns: 'buttonText' })}</Button>
        </>
    );
};

export default Filter;
