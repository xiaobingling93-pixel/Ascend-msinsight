/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React from 'react';
import styled from '@emotion/styled';
import { ReactComponent as HelpDarkSvg } from './img/help_dark.svg';
import { ReactComponent as HelpLightSvg } from './img/help_light.svg';
import { ReactComponent as PinSvg } from './img/icons_dark_normal_ascendinsight_pin.svg';
import { ReactComponent as UnPinSvg } from './img/icons_dark_normal_ascendinsight_pin_line.svg';
import { ReactComponent as StartSvg } from './img/icons_dark_normal_mindstudioinsight_start.svg';
import { ReactComponent as CaretDownSvg } from './img/caret-down.svg';
import { ReactComponent as SearchDarkIcon } from './img/search_dark.svg';
import { ReactComponent as SearchLightIcon } from './img/search_light.svg';
import { ReactComponent as PlusDarkIcon } from './img/plus_dark.svg';
import { ReactComponent as PlusLightIcon } from './img/plus_light.svg';
import { ReactComponent as MinusDarkIcon } from './img/minus_dark.svg';
import { ReactComponent as MinusLightIcon } from './img/minus_light.svg';
import { ReactComponent as FilterDarkIcon } from './img/filter_dark.svg';
import { ReactComponent as FilterLightIcon } from './img/filter_light.svg';
import { ReactComponent as FlagDarkIcon } from './img/flag_dark.svg';
import { ReactComponent as FlagLightIcon } from './img/flag_light.svg';
import { ReactComponent as LinkDarkIcon } from './img/link_dark.svg';
import { ReactComponent as LinkLightIcon } from './img/link_light.svg';
import { ReactComponent as ResetDarkIcon } from './img/reset_dark.svg';
import { ReactComponent as ResetLightIcon } from './img/reset_light.svg';
import { ReactComponent as BulbSvg } from './img/bulb.svg';
import { ReactComponent as ArrowDownDarkSvg } from './img/arrow_down_dark.svg';
import { ReactComponent as ArrowDownLightSvg } from './img/arrow_down_light.svg';
import { ReactComponent as ArrowUpDarkSvg } from './img/arrow_up_dark.svg';
import { ReactComponent as ArrowUpLightSvg } from './img/arrow_up_light.svg';
import { ReactComponent as ColumnFilterSvg } from './img/column_filter.svg';
import { themeInstance } from '../theme';
import CaretRightSvg from './img/caret-right.svg';
import { CaretUpOutlined, CaretDownOutlined } from '@ant-design/icons';

interface ISVGProps extends React.SVGProps< SVGSVGElement > {
    active?: boolean;
    disabled?: boolean;
    theme?: 'dark' | 'light';
}

interface IIconProps extends ISVGProps {
    type?: string;

    svgElement?: React.FunctionComponent<React.SVGProps< SVGSVGElement > & { title?: string }>;
}

interface IIconDivProps extends React.DetailedHTMLProps<React.HTMLAttributes<HTMLDivElement>, HTMLDivElement> {
    type?: string;
    url?: string;
    float?: string;
    height?: string;
    width?: string;
}

const iconMap: Record<string, any> = {
    pin: PinSvg,
    linePin: UnPinSvg,
    start: StartSvg,
    caretRight: CaretRightSvg,
    bulb: BulbSvg,
    columnFilter: ColumnFilterSvg,
    dark: {
        help: HelpDarkSvg,
        flag: FlagDarkIcon,
        filter: FilterDarkIcon,
        search: SearchDarkIcon,
        link: LinkDarkIcon,
        plus: PlusDarkIcon,
        minus: MinusDarkIcon,
        reset: ResetDarkIcon,
        arrowDown: ArrowDownDarkSvg,
        arrowUp: ArrowUpDarkSvg,
    },
    light: {
        help: HelpLightSvg,
        flag: FlagLightIcon,
        filter: FilterLightIcon,
        search: SearchLightIcon,
        link: LinkLightIcon,
        plus: PlusLightIcon,
        minus: MinusLightIcon,
        reset: ResetLightIcon,
        arrowDown: ArrowDownLightSvg,
        arrowUp: ArrowUpLightSvg,
    },
};

const StyledIcon = styled.div<{ color?: string;disabled?: boolean }>`
    display: inline-flex;
    align-items: center;
    color: inherit;
    font-style: normal;
    line-height: 0;
    text-align: center;
    text-transform: none;
    vertical-align: text-top;
    text-rendering: optimizelegibility;
    -webkit-font-smoothing: antialiased;
    & path {
        fill: ${(p): string | null => {
        const { disabled, color, theme } = p;
        if (disabled) {
            return theme.disableButtonBackgroundColor;
        } else if (color !== null && color !== undefined) {
            return color;
        } else {
            return null;
        }
    }};
    }
`;

export function Icon({ type = '', svgElement, className = '', theme, color, active, disabled, style, ...restProp }: IIconProps): JSX.Element {
    const curTheme = themeInstance.getCurrentTheme();
    const Svg = iconMap[type] ?? iconMap[theme ?? curTheme][type] ?? svgElement;
    if (Svg === null || Svg === undefined) {
        return <StyledIcon/>;
    }

    return <StyledIcon disabled={disabled} color={color} style={{ ...(style ?? {}) }} className={className}>
        <Svg
            width={16}
            height={16}
            className={`svg-icon anticon ${active ? 'active' : ''}`}
            {...restProp}
        />
    </StyledIcon>;
}

const IconDiv = styled.div<{ type?: string;url?: string;height?: string;width?: string }>`
    display: inline-flex;
    align-items: center;
    color: inherit;
    font-style: normal;
    line-height: 0;
    text-align: center;
    text-transform: none;
    vertical-align: text-top;
    text-rendering: optimizelegibility;
    -webkit-font-smoothing: antialiased;
    height:16px;
    width: 16px;
    background:center no-repeat url('${(p): string =>
        iconMap[p.type ?? ''] ?? iconMap[themeInstance.getCurrentTheme()][p.type ?? ''] ?? p.url ?? ''}') ;
    background-size: ${(p): string => `${p.height ?? '16px'} ${p.width ?? '16px'}`};
`;

export function HelpIcon(props: ISVGProps): JSX.Element {
    return <Icon type={'help'} {...props }/>;
}

export function PinIcon(props: ISVGProps): JSX.Element {
    return <Icon type={'pin'} active={true} {...props }/>;
}

export function UnPinIcon(props: ISVGProps): JSX.Element {
    return <Icon type={'linePin'} active={true} {...props }/>;
}

export function CaretDownIcon(props: ISVGProps): JSX.Element {
    return <Icon svgElement={CaretDownSvg} {...props }/>;
}

export function CaretRightIcon(props: IIconDivProps): JSX.Element {
    return <IconDiv type={'caretRight'} {...props } />;
}

export function FlagIcon(props: ISVGProps): JSX.Element {
    return <Icon type={'flag'} {...props } />;
}
export function FilterIcon(props: ISVGProps): JSX.Element {
    return <Icon type={'filter'} {...props } />;
}
export function SearchIcon(props: ISVGProps): JSX.Element {
    return <Icon type={'search'} {...props } />;
}
export function LinkIcon(props: ISVGProps): JSX.Element {
    return <Icon type={'link'} {...props } />;
}
export function PlusIcon(props: ISVGProps): JSX.Element {
    return <Icon type={'plus'} {...props } />;
}
export function MinusIcon(props: ISVGProps): JSX.Element {
    return <Icon type={'minus'} {...props } />;
}
export function ResetIcon(props: ISVGProps): JSX.Element {
    return <Icon type={'reset'} {...props } />;
}
export function StartIcon(props: ISVGProps): JSX.Element {
    return <Icon type={'start'} {...props }/>;
}
export function BulbIcon(props: ISVGProps): JSX.Element {
    return <Icon type={'bulb'} {...props }/>;
}
export function ArrowUpIcon(props: ISVGProps): JSX.Element {
    return <Icon type={'arrowUp'} width={92} {...props }/>;
}
export function ArrowDownIcon(props: ISVGProps): JSX.Element {
    return <Icon type={'arrowDown'} width={92} {...props }/>;
}

export function ColumnSorterIcon(): JSX.Element {
    return <span className={'ant-table-column-sorter-inner ant-table-column-sorter'} style={{ fontSize: '11px' }}>
        <CaretUpOutlined className={'sorter-up'}/>
        <CaretDownOutlined className={'sorter-down'} style={{ marginTop: '-.3em' }}/>
    </span>;
}
export function ColumnFilterIcon(props: ISVGProps): JSX.Element {
    return <Icon type={'columnFilter'} {...props }/>;
}
