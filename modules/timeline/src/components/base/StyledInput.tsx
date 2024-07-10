import * as React from 'react';
import styled from '@emotion/styled';
import { Input } from 'antd';
import { InputProps, InputRef } from 'antd/lib/input';

const Support = React.forwardRef(
    (props: InputProps & { minwidth: number; isshow: number; width?: number }, ref: React.ForwardedRef<InputRef>) => {
        return <Input {...props} ref={ref} />;
    },
);
Support.displayName = 'Support';
export const StyledInput = styled(Support)`
    border-radius: 20px;
    min-width: ${(props): number => props.minwidth}px;
    width: ${(props): number => props.width as number}px;
    height: ${(props): number => props.height as number}px;
    border: 1px solid  ${(props): string => props.theme.enclosureBorder};
    background-color: transparent;
    padding: 0px 8px;
    margin: 0px 5px;
    display: ${(props): string => props.isshow === 1 ? 'inline-flex' : 'none'};
    color: ${(props): string => props.theme.fontColor};

    .ant-input {
        background-color: transparent;
        font-size: 14px;
        caret-color: ${(props): string => props.theme.searchInputCaretColor};
        color: ${(props): string => props.theme.fontColor};
    }

    .ant-input-suffix {
        height: 16px;
        color: ${(props): string => props.theme.svgBackgroundColor};
        font-size: 12px;
        margin-top: 5px;
        margin-right: -4px;
    }

    input[type=number]::-webkit-outer-spin-button,
    input[type=number]::-webkit-inner-spin-button {
        -webkit-appearance: none;
        appearance: none;
        margin: 0;
    }
`;
