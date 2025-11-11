/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/
import styled from '@emotion/styled';
import type { ButtonProps } from 'antd/lib/button';
import { Button } from 'antd';
import * as React from 'react';
import { useTheme } from '@emotion/react';
import type { ForwardedRef, FunctionComponent, SVGProps } from 'react';
import { Tooltip } from '@insight/lib/components';

interface CustomButtonProps {
    isEmphasize?: boolean;
    isDisabled?: boolean;
    isSuspend?: boolean;
    tooltip?: string | null;
    icon?: FunctionComponent<SVGProps<SVGSVGElement> & { title?: string }>;
}

interface StyledButtonProps {
    width?: number;
    transparent?: boolean;
}

export const StyledButton = styled(Button)<StyledButtonProps>`
    display: flex;
    align-items: center;
    width: ${(props): string => props.width !== undefined ? `${props.width}px` : '22px'};
    height: 20px;
    line-height: 20px;
    color: ${(props): string => props.theme.textColorPrimary};
    background-color: ${(props): string => props.transparent ? 'transparent' : props?.style?.backgroundColor ?? props.theme.bgColorLight};
    &:hover {
        background-color: ${(props): string => props.transparent ? 'transparent' : props?.style?.backgroundColor ?? props.theme.primaryColorHover};
        cursor: ${(props): string => props.disabled as boolean ? 'not-allowed' : 'pointer'};
    }

    border-radius: 4px;
    justify-content: center;
    border: none;
    margin-right: 3px;

    &:before {
        content: ${(props): string => props.transparent ? 'normal' : ''};
    }
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
    return <Tooltip title={tooltip}>
        <StyledButton ref={ref} disabled={isDisabled} {...props} icon={Icon && <Icon fill={buttonFillColor}/>}
            style={{ backgroundColor: isSuspend ? theme.buttonColor.suspendBGColor : theme.buttonColor.unSuspendBGColor }}>
        </StyledButton>
    </Tooltip>;
});

CustomButton.displayName = 'CustomButton';
