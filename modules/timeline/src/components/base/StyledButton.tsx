/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/
import styled from '@emotion/styled';
import type { ButtonProps } from 'antd/lib/button';
import { Button } from 'antd';
import * as React from 'react';
import { useTheme } from '@emotion/react';
import type { ForwardedRef, FunctionComponent, SVGProps } from 'react';
import { StyledTooltip } from './StyledTooltip';

interface CustomButtonProps {
    isEmphasize?: boolean;
    isDisabled?: boolean;
    isSuspend?: boolean;
    tooltip?: string | null;
    icon?: FunctionComponent<SVGProps<SVGSVGElement> & { title?: string }>;
}

interface StyledButtonProps {
    width?: number;
}

export const StyledButton = styled(Button)<StyledButtonProps>`
    display: flex;
    align-items: center;
    width: ${(props): string => props.width !== undefined ? `${props.width}px` : '22px'};
    height: 20px;
    line-height: 20px;
    color: ${(props): string => props.theme.textColorPrimary};
    background-color: ${(props): string => props?.style?.backgroundColor ?? props.theme.bgColorLight};
    &:hover {
        background-color: ${(props): string => props?.style?.backgroundColor ?? props.theme.primaryColorHover};
        cursor: ${(props): string => props.disabled as boolean ? 'not-allowed' : 'pointer'};
    }

    border-radius: 4px;
    justify-content: center;
    border: none;
    margin-right: 3px;
`;

export const CustomButton = React.forwardRef(({ icon, isDisabled, isSuspend, tooltip, isEmphasize, ...props }: ButtonProps & CustomButtonProps,
    ref?: ForwardedRef<HTMLButtonElement>): JSX.Element => {
    const theme = useTheme();
    let buttonFillColor = theme.buttonColor.enableClickColor;
    if (isDisabled) {
        buttonFillColor = theme.buttonColor.disableClickColor;
    } else {
        if (isEmphasize) {
            buttonFillColor = theme.buttonColor.emphasizeColor;
        }
    }

    const Icon = icon;
    return <StyledTooltip title={tooltip}>
        <StyledButton ref={ref} disabled={isDisabled} {...props} icon={Icon && <Icon fill={buttonFillColor}/>}
            style={{ backgroundColor: isSuspend ? theme.buttonColor.suspendBGColor : theme.buttonColor.unSuspendBGColor }}>
        </StyledButton>
    </StyledTooltip>;
});

CustomButton.displayName = 'CustomButton';

export const PressButton = styled(Button)`
    display: flex;
    align-items: center;
    color: ${(props): string => props.theme.fontColor} !important;
    background-color: ${(props): string => props?.style?.backgroundColor ?? props.theme.insightHeaderButtonBackgroundColor}!important;
    border-radius: 4px;
    justify-content: center;
    height: 20px;
    margin-top: 6px;
    border: none;
    margin-right: 3px;
`;
