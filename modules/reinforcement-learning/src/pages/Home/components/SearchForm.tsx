/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
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
