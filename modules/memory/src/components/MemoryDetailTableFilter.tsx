/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
 
import React, { useState, useEffect } from 'react';
import { observer } from 'mobx-react-lite';
import { runInAction } from 'mobx';
import { SearchBox } from '../utils/styleUtils';
import { Label } from './Common';
import { Button, Input, InputNumber } from 'ascend-components';
import { Session } from '../entity/session';
import { MemorySession, DEFAULT_SIZE_CONDITION } from '../entity/memorySession';
import { useTranslation } from 'react-i18next';

const COMPARE_MIN_INPUT_NUMBER = -2147483648;
const MAX_INPUT_NUMBER = 4294967295;

const MemoryDetailTableFilter = observer(({ session, memorySession, queryDetailData }:
{ session: Session; memorySession: MemorySession; queryDetailData: (resetCurrent: boolean) => void }) => {
    // 是否为比对场景
    const isCompare: boolean = session.compareRank.isCompare;
    const [searchEventOperatorName, setSearchEventOperatorName] = useState<string>('');
    const [minSize, setMinSize] = useState<number>(0);
    // 最大内存范围，默认DEFAULT_SIZE_CONDITION KB
    const [maxSize, setMaxSize] = useState<number>(memorySession.maxSize);
    const isBtnDisabled: boolean = memorySession.isBtnDisabled;
    const { t } = useTranslation('memory');

    const onSearchEventOperatorChanged: React.ChangeEventHandler<HTMLInputElement | HTMLTextAreaElement> = (event) => {
        setSearchEventOperatorName(event.target.value as string);
        runInAction(() => {
            memorySession.searchEventOperatorName = event.target.value as string;
        });
    };

    const onFilterEventMinSizeInputChanged = (value: number | string | null): void => {
        setMinSize(value as number);
        runInAction(() => {
            memorySession.minSize = value as number;
        });
    };

    const onFilterEventMaxSizeInputChanged = (value: number | string | null): void => {
        setMaxSize(value as number);
        runInAction(() => {
            memorySession.maxSize = value as number;
        });
    };

    const onQuery = (): void => {
        queryDetailData(true);
    };

    const onReset = (): void => {
        setSearchEventOperatorName('');
        setMinSize(isCompare ? -DEFAULT_SIZE_CONDITION : 0);
        setMaxSize(DEFAULT_SIZE_CONDITION);
        runInAction(() => {
            memorySession.searchEventOperatorName = '';
            memorySession.minSize = isCompare ? -DEFAULT_SIZE_CONDITION : 0;
            memorySession.maxSize = DEFAULT_SIZE_CONDITION;
        });
        queryDetailData(false);
    };

    useEffect(() => {
        onReset();
    }, [isCompare]);

    return (
        <SearchBox>
            <div className="flex items-center">
                <Label name={t('searchCriteria.Name')} />
                <Input
                    value={searchEventOperatorName}
                    onChange={onSearchEventOperatorChanged}
                    placeholder={t('searchCriteria.Search by Name')}
                    allowClear
                    maxLength={200}
                />
            </div>
            <div className="flex items-center">
                <Label name={t('searchCriteria.Min Size')} />
                <InputNumber
                    value={minSize}
                    onChange={onFilterEventMinSizeInputChanged}
                    min={isCompare ? COMPARE_MIN_INPUT_NUMBER : 0}
                    max={MAX_INPUT_NUMBER}
                />
            </div>
            <div className="flex items-center">
                <Label name={t('searchCriteria.Max Size')} />
                <InputNumber
                    value={maxSize}
                    onChange={onFilterEventMaxSizeInputChanged}
                    min={isCompare ? COMPARE_MIN_INPUT_NUMBER : 0}
                    max={MAX_INPUT_NUMBER}
                    minLength={1}
                />
            </div>
            <div className="flex items-center">
                <Button
                    onClick={onQuery}
                    type="primary"
                    style={{ marginRight: 8 }}
                    disabled={isBtnDisabled}
                >
                    {t('searchCriteria.Button Query')}
                </Button>
                <Button
                    onClick={onReset}
                    disabled={isBtnDisabled}
                >
                    {t('searchCriteria.Button Reset')}
                </Button>
            </div>
        </SearchBox>
    );
});

export default MemoryDetailTableFilter;
