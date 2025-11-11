/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { Form } from 'antd';
import React, { useEffect } from 'react';
import { Button, InputNumber, Select, Tooltip } from '@insight/lib/components';
import { useTranslation } from 'react-i18next';
import type { SelectProps } from 'antd';
import type { FormData } from './index';
import { ImportOutlined } from '@ant-design/icons';

const useModelStageOptions = (): SelectProps['options'] => {
    const { t } = useTranslation('summary');

    return [
        { label: t('Prefill'), value: 'prefill' },
        { label: t('Decode'), value: 'decode' },
    ];
};

const useDataVersionOptions = (): SelectProps['options'] => {
    const { t } = useTranslation('summary');

    return [
        { label: t('Profiling'), value: 'profiling' },
        { label: t('Dump unbalanced'), value: 'unbalanced' },
        { label: t('Dump balanced'), value: 'balanced' },
    ];
};

const useDenseLayerListOptions = (layerNum: number | null): SelectProps['options'] => {
    return Array.from({ length: layerNum ?? 0 }, (_, index) => ({ label: index, value: index }));
};

interface ExpertLoadBalancingFormProps {
    data: FormData;
    loading: boolean;
    onFormChange: (changedValues: any, data: FormData) => void;
    onFormSubmit: () => void;
    onImport: () => void;
}

export const ExpertLoadBalancingForm = ({ data, loading, onFormChange, onFormSubmit, onImport }: ExpertLoadBalancingFormProps): React.ReactElement => {
    const { t } = useTranslation('summary');
    const [form] = Form.useForm();
    const modelStageOptions = useModelStageOptions();
    const dataVersionOptions = useDataVersionOptions();
    const denseLayerListOptions = useDenseLayerListOptions(data.layerNum);

    const handleValueChange = (changedValues: any, allValues: FormData): void => {
        onFormChange(changedValues, allValues);
    };

    const handleFinish = (): void => {
        onFormSubmit();
    };

    useEffect(() => {
        form.setFieldsValue(data);
    }, [data]);

    return <Form
        layout={'inline'}
        form={form}
        initialValues={data}
        onValuesChange={handleValueChange}
        onFinish={handleFinish}
    >
        <Form.Item label={t('Model Layer Num')} name="layerNum">
            <InputNumber
                min={1}
                max={500}
                size="small"
            />
        </Form.Item>
        <Form.Item label={t('Dense Layer List')} name="denseLayerList">
            <Select
                allowClear
                mode="multiple"
                options={denseLayerListOptions}
                style={{ minWidth: 100 }}
            />
        </Form.Item>
        <Form.Item label={t('Expert Num')} name="expertNum">
            <InputNumber
                min={1}
                max={500}
                size="small"
            />
        </Form.Item>
        <Form.Item label={t('Model Stage')} name="modelStage">
            <Select
                id="select-expert-load"
                options={modelStageOptions}
                style={{ minWidth: 100 }}
            />
        </Form.Item>
        <Form.Item label={t('Data Version')} name="version">
            <Select
                options={dataVersionOptions}
                style={{ minWidth: 100 }}
            />
        </Form.Item>
        {
            data.version !== 'profiling' &&
                <Form.Item>
                    <Tooltip title={t('Import data')}>
                        <Button
                            style={{ minWidth: 'auto' }}
                            type="primary"
                            icon={<ImportOutlined/>}
                            onClick={onImport}/>
                    </Tooltip>
                </Form.Item>
        }
        <Form.Item>
            <Button type="primary" htmlType="submit" loading={loading}>
                {t('Search')}
            </Button>
        </Form.Item>
    </Form>;
};
