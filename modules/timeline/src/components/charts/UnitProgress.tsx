/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useState, useEffect } from 'react';
import { Progress } from 'antd';

export const UnitProgress = ({ realProgress, showProgress }: { realProgress: number; showProgress: boolean }): JSX.Element => {
    const [progress, setProgress] = useState(0);
    const [isShowProgress, setIsShowProgress] = useState(false);

    useEffect(() => {
        setProgress(realProgress);
        if (realProgress === 100 && !showProgress) {
            setTimeout(() => {
                setIsShowProgress(showProgress);
            }, 1000);
        } else {
            setIsShowProgress(showProgress);
        }
    }, [realProgress, showProgress]);
    return <div style={{ textAlign: 'center' }}>
        {isShowProgress ? <Progress percent={progress} type="line" size="small"></Progress> : null}
    </div>;
};
