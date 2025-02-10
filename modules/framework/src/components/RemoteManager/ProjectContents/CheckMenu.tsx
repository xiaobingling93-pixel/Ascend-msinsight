/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import React from 'react';
import { observer } from 'mobx-react';
import { Checkbox } from 'ascend-components';
import { useTranslation } from 'react-i18next';
import { DeleteIcon } from 'ascend-icon';
import { BtnItem } from '../ImportData/Index';
import type { CheckboxChangeEvent } from 'antd/es/checkbox';
import styled from '@emotion/styled';
import { Popconfirm } from 'antd';
import { removeProjects } from '@/utils/Project';

const CheckMenuContainer = styled.div`
    padding: 0 8px;
    display: flex;
    align-items: end;
    justify-content: space-between;
    font-size: 12px!important;
    .ant-checkbox-wrapper{
        font-size: 12px!important;
    }
`;
interface IProps {
    editStatus: boolean;
    isAll: boolean;
    checkedKeys: React.Key[];
    toggleCheckAll: (val: boolean) => void;
}

// 目录树 全选、删除按钮
const CheckMenu = observer(({ editStatus, isAll, toggleCheckAll, checkedKeys }: IProps) => {
    const { t } = useTranslation('framework');

    const confirm = (): void => {
        removeProjects(checkedKeys);
    };

    return editStatus
        ? <CheckMenuContainer>
            <Checkbox onChange={(e: CheckboxChangeEvent): void => toggleCheckAll(e.target.checked)} checked={isAll}>{t('All')}</Checkbox>
            <Popconfirm placement="topLeft"
                title={t('DeleteConfirmDescribe')}
                onConfirm={confirm}
                okText={t('Yes')}
                cancelText={t('No')}
                destroyTooltipOnHide={{ keepParent: false }}
            >
                <BtnItem className={`btn-delete small danger ${checkedKeys.length === 0 ? 'disabled' : ''}`}><DeleteIcon/></BtnItem>
            </Popconfirm>
        </CheckMenuContainer>
        : <></>;
});

export default CheckMenu;
