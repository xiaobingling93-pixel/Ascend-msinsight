/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { useEffect } from 'react';
import { observer } from 'mobx-react-lite';
import { runInAction } from 'mobx';
import { useTranslation } from 'react-i18next';
import { Button, Input, InputNumber } from '@insight/lib/components';
import { SearchBox } from '../utils/styleUtils';
import { Label } from './Common';
import { Session } from '../entity/session';
import {
    MemorySession,
    MemoryGraphType,
    DEFAULT_SHOW_WITHIN_INTERVAL,
} from '../entity/memorySession';
import OptionalCheckbox from './OptionalCheckbox';
import type { CheckboxChangeEvent } from 'antd/es/checkbox';

const COMPARE_MIN_INPUT_NUMBER = -2147483648;
const MAX_INPUT_NUMBER = 4294967295;

const MemoryStaticAndCompareDetailTableFilter = observer(({ session, memorySession, queryDetailData }:
{ session: Session; memorySession: MemorySession; queryDetailData: (resetCurrent: boolean) => void }) => {
    // 是否为比对场景
    const isCompare: boolean = session.compareRank.isCompare;
    const isBtnDisabled: boolean = memorySession.isBtnDisabled;
    const { t } = useTranslation('memory');

    const onSearchEventOperatorChanged: React.ChangeEventHandler<HTMLInputElement | HTMLTextAreaElement> = (event) => {
        runInAction(() => {
            memorySession.searchEventOperatorName = (event.target.value as string).trim();
        });
    };

    const onFilterEventMinSizeInputChanged = (value: number | string | null): void => {
        runInAction(() => {
            memorySession.minSize = (value as number) ?? 0;
        });
    };

    const onFilterEventMaxSizeInputChanged = (value: number | string | null): void => {
        runInAction(() => {
            memorySession.maxSize = (value as number) ?? 0;
        });
    };

    const onShowPassThroughTimeIntervalDataCheckboxChanged = (value: CheckboxChangeEvent): void => {
        runInAction(() => {
            memorySession.isOnlyShowAllocatedOrReleasedWithinInterval = value.target.checked as boolean;
        });
    };

    const onQuery = (): void => {
        queryDetailData(true);
    };

    const onReset = (): void => {
        runInAction(() => {
            memorySession.searchEventOperatorName = '';
            memorySession.minSize = memorySession.defaultMinSize;
            memorySession.maxSize = memorySession.defaultMaxSize;
            memorySession.isOnlyShowAllocatedOrReleasedWithinInterval = DEFAULT_SHOW_WITHIN_INTERVAL;
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
                    id={'input-name'}
                    value={memorySession.searchEventOperatorName}
                    onChange={onSearchEventOperatorChanged}
                    placeholder={t('searchCriteria.Search by Name')}
                    allowClear
                    maxLength={200}
                />
            </div>
            <div className="flex items-center">
                <Label name={t('searchCriteria.Min Size')} />
                <InputNumber
                    id={'input-minSize'}
                    value={memorySession.minSize}
                    onChange={onFilterEventMinSizeInputChanged}
                    precision={0}
                    min={isCompare ? COMPARE_MIN_INPUT_NUMBER : memorySession.defaultMinSize}
                    max={MAX_INPUT_NUMBER}
                />
            </div>
            <div className="flex items-center">
                <Label name={t('searchCriteria.Max Size')} />
                <InputNumber
                    id={'input-maxSize'}
                    value={memorySession.maxSize}
                    onChange={onFilterEventMaxSizeInputChanged}
                    precision={0}
                    min={isCompare ? COMPARE_MIN_INPUT_NUMBER : memorySession.defaultMinSize}
                    max={MAX_INPUT_NUMBER}
                    minLength={1}
                />
            </div>
            {/* 当 MemoryGraphType.STATIC 时，不显示勾选项 */}
            <OptionalCheckbox
                idKey="input-onlyShowAllocatedOrReleased"
                visible={memorySession.memoryType !== MemoryGraphType.STATIC}
                name={t('searchCriteria.Show Allocated or Released Within Interval Data')}
                value={memorySession.isOnlyShowAllocatedOrReleasedWithinInterval}
                onChange={onShowPassThroughTimeIntervalDataCheckboxChanged}
            />
            <div className="flex items-center">
                <Button
                    data-testid={'query-btn'}
                    onClick={onQuery}
                    type="primary"
                    style={{ marginRight: 8 }}
                    disabled={isBtnDisabled}
                >
                    {t('searchCriteria.Button Query')}
                </Button>
                <Button
                    data-testid={'reset-btn'}
                    onClick={onReset}
                    disabled={isBtnDisabled}
                >
                    {t('searchCriteria.Button Reset')}
                </Button>
            </div>
        </SearchBox>
    );
});

const MemoryDynamicDetailTableFilter = observer(({ session, memorySession, queryDetailData, onReset }:
{ session: Session; memorySession: MemorySession; queryDetailData: (resetCurrent: boolean) => void; onReset: () => void }) => {
    // 是否为比对场景
    const isCompare: boolean = session.compareRank.isCompare;
    const isDisabled: boolean = memorySession.isBtnDisabled;
    const { t } = useTranslation('memory');

    const onShowPassThroughTimeIntervalDataCheckboxChanged = (value: CheckboxChangeEvent): void => {
        runInAction(() => {
            memorySession.isOnlyShowAllocatedOrReleasedWithinInterval = value.target.checked;
        });
        queryDetailData(true);
    };

    const handleReset = (): void => {
        runInAction(() => {
            memorySession.isOnlyShowAllocatedOrReleasedWithinInterval = DEFAULT_SHOW_WITHIN_INTERVAL;
        });
        onReset();
    };

    useEffect(() => {
        runInAction(() => {
            memorySession.isOnlyShowAllocatedOrReleasedWithinInterval = DEFAULT_SHOW_WITHIN_INTERVAL;
        });
    }, [isCompare]);

    return (
        <SearchBox>
            <OptionalCheckbox
                idKey="input-onlyShowAllocatedOrReleased"
                visible={true}
                name={t('searchCriteria.Show Allocated or Released Within Interval Data')}
                value={memorySession.isOnlyShowAllocatedOrReleasedWithinInterval}
                disabled={isDisabled}
                onChange={onShowPassThroughTimeIntervalDataCheckboxChanged}
            />
            <Button
                data-testid={'reset-btn'}
                onClick={handleReset}
            >
                {t('searchCriteria.Button Reset')}
            </Button>
        </SearchBox>
    );
});

const MemoryDetailTableFilter = observer(({ session, memorySession, queryDetailData, onReset }:
{ session: Session; memorySession: MemorySession; queryDetailData: (resetCurrent: boolean) => void; onReset: () => void }) => {
    return <>
        {
            memorySession.memoryType === MemoryGraphType.STATIC || session.compareRank.isCompare
                ? <MemoryStaticAndCompareDetailTableFilter session={session} memorySession={memorySession} queryDetailData={queryDetailData} />
                : <MemoryDynamicDetailTableFilter session={session} memorySession={memorySession} queryDetailData={queryDetailData} onReset={onReset} />
        }
    </>;
});

export default MemoryDetailTableFilter;
