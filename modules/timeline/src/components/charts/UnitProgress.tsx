/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useState, useEffect } from 'react';
import type { KeyedInsightUnit } from '../ChartContainer/Units/types';
import { Progress } from '@insight/lib/components';
import { useTheme } from '@emotion/react';
import { runInAction } from 'mobx';

export const UnitProgress = ({ unit, realProgress, showProgress }: { unit: KeyedInsightUnit; realProgress: number; showProgress: boolean }): JSX.Element => {
    const [progress, setProgress] = useState(0);
    const [isShowProgress, setIsShowProgress] = useState(false);
    const theme = useTheme();

    useEffect(() => {
        setProgress(realProgress);
        if (realProgress === 100 && !showProgress) {
            setTimeout(() => {
                setIsShowProgress(showProgress);
            }, 300);
            setTimeout(() => {
                runInAction(() => {
                    unit.shouldParse = false;
                });
            }, 300);
        } else {
            setIsShowProgress(showProgress);
        }
    }, [realProgress, showProgress]);
    return isShowProgress ? <Progress strokeColor={theme.primaryColor} percent={progress} type="line" size="small"></Progress> : <></>;
};
