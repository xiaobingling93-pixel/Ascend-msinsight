import * as React from 'react';
import styled from '@emotion/styled';
import { Radio } from 'antd';
import { RadioProps } from 'antd/lib/radio';

export const StyledRadio = styled((props: RadioProps & { fontSize?: number }) => <Radio {...props}/>)`
    color: ${(props) => props.theme.fontColor};
    font-size: ${(props) => typeof props.fontSize === 'undefined' ? 12 : props.fontSize}px;

    .ant-radio-checked > .ant-radio-inner {
        border-color: #1890ff;
        background-color: #1890ff;
    }

    .ant-radio-input:focus + .ant-radio-inner {
        box-shadow: unset;
    }

    .ant-radio-inner {
        &::after {
            background-color: #FFF;
        }
        background-color: ${(props) => props.theme.deviceProcessBackgroundColor};
        border: 1px solid ${(props) => props.theme.enclosureBorder};
    }
`;
