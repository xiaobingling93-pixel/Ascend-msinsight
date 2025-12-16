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
import React from 'react';
import { observer } from 'mobx-react';
import { Checkbox } from '@insight/lib/components';
import { useTranslation } from 'react-i18next';
import { DeleteIcon } from '@insight/lib/icon';
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
