/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React from 'react';
import './Icon.css';
import { ReactComponent as HelpSvg } from './img/icons_dark_normal_ascendinsight_help.svg';
import { ReactComponent as PinSvg } from './img/icons_dark_normal_ascendinsight_pin.svg';
import { ReactComponent as LinePinSvg } from './img/icons_dark_normal_ascendinsight_pin_line.svg';

interface ISVGProps extends React.SVGProps< SVGSVGElement > {
    active?: boolean;
    disabled?: boolean;
}

interface IIconProps extends ISVGProps {
    type?: string ;

    svgElement?: React.FunctionComponent<React.SVGProps< SVGSVGElement > & { title?: string }>;
}

const theme = {
    light: {
        normal: '#595959',
        disabled: '#BDBDBD',
        active: '#007AFF',
    },
    dark: {
        normal: '#D1D1D1',
        disabled: '#595959',
        active: '#007AFF',
    },
};

const defaultIconStyle = `
        .svg-icon path {
          fill: ${theme.light.normal};
        }
        .svg-icon.active path {
          fill: ${theme.light.active};
        }
        .svg-icon.disabled path {
          fill: ${theme.light.disabled};
        }
        .theme_dark .svg-icon path {
          fill: ${theme.dark.normal};
        }
        .theme_dark .svg-icon.active path {
          fill: ${theme.dark.active};
        }
        .theme_dark .svg-icon.disabled path {
          fill: ${theme.dark.disabled};
        }
        `;

const iconMap: Record<string, any> = {
    help: HelpSvg,
    pin: PinSvg,
    linePin: LinePinSvg,
};

export function Icon({ type = '', svgElement, className = '', color, active, disabled, style, ...restProp }: IIconProps): JSX.Element {
    const Svg = iconMap[type] ?? svgElement ?? <></>;

    let colorClass = '';
    let colorCss = '';
    if (color !== undefined && color !== null && color !== '') {
        // 替换颜色中的 # ：( ) ， . 空格
        colorClass = `color-${color.replace(/#|:|\(|\)|,|\.|\s/g, '')}`;
        colorCss = `.svg-icon.${colorClass} path {
          fill: ${color};
        }`;
    }

    return <span className={'icon-box'} style={{ ...(style ?? {}) }}>
        <style>
            {`${defaultIconStyle} ${colorCss}`}
        </style>
        <Svg
            width={16}
            height={16}
            className={`svg-icon anticon ${colorClass} ${active ? 'active' : ''} ${disabled ? 'disabled' : ''} ${className}`}
            {...restProp}
        />
    </span>;
}

export function HelpIcon(props: ISVGProps): JSX.Element {
    return <Icon type={'help'} {...props }/>;
}

export function PinIcon(props: ISVGProps): JSX.Element {
    return <Icon type={'pin'} active={true} {...props }/>;
}

export function LinePinIcon(props: ISVGProps): JSX.Element {
    return <Icon type={'linePin'} active={true} {...props }/>;
}
