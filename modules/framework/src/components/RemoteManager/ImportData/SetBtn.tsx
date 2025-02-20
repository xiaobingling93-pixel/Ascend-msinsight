/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import React, { useEffect } from 'react';
import { observer } from 'mobx-react';
import { Session } from '@/entity/session';
import { SetIcon } from 'ascend-icon';
import { BtnItem } from './Index';
import { useTranslation } from 'react-i18next';
import { runInAction } from 'mobx';

// 设置（按钮）
const SetBtn = observer(({ session }: {session: Session}) => {
    const { t } = useTranslation('framework');
    const switchEditStatus = (): void => {
        runInAction(() => {
            session.projectContentEditStatus = !session.projectContentEditStatus;
        });
    };
    useEffect(() => {
        // 没有数据时
        if (session.dataSources.length === 0) {
            runInAction(() => {
                session.projectContentEditStatus = false;
            });
        };
    }, [session.dataSources.length]);
    return <BtnItem className={`btn-set ${session.dataSources.length > 0 ? '' : 'disabled'}`} onClick={switchEditStatus}>
        { session.projectContentEditStatus ? t('Cancel') : <SetIcon/> }
    </BtnItem>;
});

export default SetBtn;
