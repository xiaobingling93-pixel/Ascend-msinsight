/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/

import * as React from 'react';
import { Select } from 'antd';
import styled from '@emotion/styled';
import { CaretDownIcon } from '../icon/Icon';
import type { SelectProps } from 'antd/lib/select';
import { useTranslation } from 'react-i18next';

const sizeOption = {
    small: 100,
    middle: 160,
    large: 240,
};

const NoDataContent = styled.div`
    color: ${(props): string => props.theme.textColorPlaceholder};
`;

export const MISelect = styled((props: SelectProps & { width: number; height?: number }): JSX.Element => {
    const { t } = useTranslation('lib');
    const isDisabled = (props.options?.length ?? 0) <= 1;
    const { size, ...restProps } = props;
    return <Select
        suffixIcon={<CaretDownIcon width={12} height={12}/>}
        placeholder={t('No data')}
        disabled={isDisabled}
        notFoundContent={<NoDataContent>{t('No data')}</NoDataContent>}
        {...restProps}
    />;
})`
    height: ${(props): number => props.height ?? 32}px;
    width: ${(props): number => props.width ?? (props.size && sizeOption[props.size]) ?? sizeOption.middle}px;
    color: ${(props): string => props.theme.textColorPrimary};

    &:not(.ant-select-customize-input) .ant-select-selector {
        background-color: ${(props): string => props.theme.bgColor};
        border-color: ${(props): string => props.theme.borderColor};
    }

    &:hover, &.ant-select-focused:not(.ant-select-disabled) {
        &:not(.ant-select-customize-input) .ant-select-selector {
            border-color: ${(props): string => props.theme.primaryColor};
        }
    }

    &.ant-select-disabled:not(.ant-select-customize-input) .ant-select-selector{
        color: ${(props): string => props.theme.borderColorDisabled};
        background-color: ${(props): string => props.theme.bgColorDisabled};
        border-color: ${(props): string => props.theme.textColorPlaceholder};
    }

    .ant-select-selection-placeholder {
        color: ${(props): string => props.theme.textColorPlaceholder};
    }

    &.ant-select-open > .ant-select-arrow > span > svg {
        transform: rotate(180deg);
    }
`;
