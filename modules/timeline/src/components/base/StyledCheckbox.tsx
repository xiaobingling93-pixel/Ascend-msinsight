import * as React from 'react';
import styled from '@emotion/styled';
import { Checkbox } from 'antd';
import { CheckboxProps } from 'antd/lib/checkbox';

export const StyledCheckbox = styled((props: CheckboxProps & { fontSize?: number }) => <Checkbox {...props}/>)`
    & > span {
        color: ${(props): string => props.disabled ? props.theme.disabledFontColor : props.theme.fontColor} !important;
        font-size: ${(props): number => props.fontSize === undefined ? 12 : props.fontSize}px;
    }

    .ant-checkbox-checked > .ant-checkbox-inner {
        border-color: ${(props): string => props.disabled ? 'transparent' : '#1890ff'} !important;
        background-color: ${(props): string => props.disabled ? props.theme.templateBackgroundColor : '#1890ff'};
    }

    .ant-checkbox-inner {
        &:focus {
            box-shadow: unset;
        }
        background-color: ${(props): string => props.theme.deviceProcessBackgroundColor};
        border: 1px solid ${(props): string => props.theme.enclosureBorder};
    }
`;
