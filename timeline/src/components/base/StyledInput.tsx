import * as React from 'react';
import styled from '@emotion/styled';
import { Input } from 'antd';
import { InputProps, InputRef } from 'antd/lib/input';

export const StyledInput = styled(React.forwardRef(
    function Support(props: InputProps & { minwidth: number; isshow: number; width?: number }, ref: React.ForwardedRef<InputRef>) {
        return <Input {...props} ref={ref} />;
    },
))`
    border-radius: 20px;
    min-width: ${(props) => props.minwidth}px;
    width: ${(props) => props.width}px;
    height: ${(props) => props.height}px;
    border: 1px solid  ${(props) => props.theme.enclosureBorder};
    background-color: transparent;
    padding: 0px 8px;
    margin: 0px 5px;
    display: ${(props) => props.isshow === 1 ? 'inline-flex' : 'none'};
    color: ${(props) => props.theme.fontColor};

    .ant-input {
        background-color: transparent;
        font-size: 14px;
        caret-color: ${props => props.theme.searchInputCaretColor};
        color: ${(props) => props.theme.fontColor};
    }

    .ant-input-suffix {
        height: 16px;
        color: ${(props) => props.theme.svgBackgroundColor};
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
