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

import React, { useEffect } from 'react';
import { Button, Form, FormItem } from '@insight/lib/components';
import { observer } from 'mobx-react-lite';
import { useStores } from '@/stores';
import { useTranslation } from 'react-i18next';
import { SearchBoxStyled } from '@/styles/styles';

export const SearchForm: React.FC = observer(() => {
    const { traceStore } = useStores();
    const [form] = Form.useForm();
    const { t } = useTranslation('RL');
    const { getTraceData, formData, loading } = traceStore;

    // 监听 store 中的搜索参数变化，同步到表单
    useEffect(() => {
        form.setFieldsValue(traceStore.formData);
    }, [form, traceStore.formData]);

    const handleSearch = (): void => {
        getTraceData();
    };

    return (
        <SearchBoxStyled>
            <FormItem label={t('Framework')}>
                {formData.framework}
            </FormItem>

            <FormItem label={t('Algorithm')}>
                {traceStore.formData.algorithm}
            </FormItem>

            <FormItem>
                <Button type="primary" loading={loading} onClick={() => handleSearch()}>{t('Refresh')}</Button>
            </FormItem>
        </SearchBoxStyled>
    );
});

export default SearchForm;
