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
