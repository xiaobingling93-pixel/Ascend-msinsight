/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import * as React from 'react';
import styled from '@emotion/styled';
import BaseContainer from '../container/BaseContainer';
import { MIDescriptions, MIDescriptionsItem } from '../MIDescriptions';
import COLOR from './Color';
import { chartVisbilityListener, getResizeEcharts, getDefaultChartOptions } from './EchartUtils';
import { Empty } from '../components/index';
import { useTheme } from '@emotion/react';
import { useTranslation } from 'react-i18next';
import { BulbIcon } from '../icon/Icon';
export { customConsole } from './Console';

export { BaseContainer, MIDescriptions, MIDescriptionsItem, COLOR, chartVisbilityListener, getResizeEcharts, getDefaultChartOptions };

export const StyledEmpty = ({ descriptor, style }:
{ descriptor: string; style?: object; translation: any}): JSX.Element => {
    const theme = useTheme();
    const { t } = useTranslation();
    return (
        <Empty
            image={Empty.PRESENTED_IMAGE_SIMPLE}
            style={style}
            description={
                <span style={{ color: theme.fontColor }}>
                    {t(descriptor ?? 'No Data')}
                </span>}>
        </Empty>
    );
};

const StyledAdvice = styled.div`
    color: ${(p): string => p.theme.textColorPrimary};
    background-color: ${(p): string => p.theme.bgColorLight};
    line-height: 36px;
    font-size: 14px;
    padding: 0 15px;
    display: flex;
    flex-direction: row;
    & > div:first-child {
        flex: 0 0 auto;
        vertical-align: top;
        & > span {
            margin: 0 8px;
            font-weight: bold;
        }
    }
    & > div:nth-child(2) {
        flex: auto;
        word-break: break-all;
    }
`;

export function Advice({ text }: { text: string }): JSX.Element {
    const { t } = useTranslation();
    return <StyledAdvice>
        <div>
            <BulbIcon/>
            <span>{t('Advice')}:</span>
        </div>
        <div>{text}</div>
    </StyledAdvice>;
}

export function limitInput(maxlength?: string): void {
    setTimeout(() => {
        const inputs = document.querySelectorAll('input');
        inputs.forEach(input => {
            if (input.maxLength < 0) {
                input.setAttribute('maxlength', maxlength ?? '200');
            }
        });
    });
}

let logindex: number = 0;
const logRecord: Record<number, unknown> = {};
export function log(...param: unknown[]): void {
    logRecord[logindex++ % 1000] = param;
}

export function Label({ name, style }: {name: React.ReactNode;style?: object }): JSX.Element {
    return <span style={{ marginRight: 10, ...(style ?? {}) }}>{name}{name !== undefined && ':'} </span>;
};

export function getSet<T extends object>(list: T[], field: keyof T): unknown[] {
    return Array.from(new Set(list.map(item => item[field])));
}

export function firstLetterUpper(word: string): string {
    const list = word.split(/\s+/);
    return list.map(item => item.charAt(0).toUpperCase() + item.slice(1)).join(' ');
}

export function getUsableVal<T>(val: T, options: Array<{value: T}>, defaultVal: T): T {
    if (options.length === 0) {
        return defaultVal;
    }
    if (options.find(item => item.value === val)) {
        return val;
    }
    return options[0].value;
};

export const delayExecute = (doFunc: () => void, timeout = 500): void => {
    setTimeout(() => {
        doFunc();
    }, timeout);
};

export function sortFunc<T>(a: T, b: T, sorter = 'asc'): number {
    const aNum = Number(a);
    const bNum = Number(b);
    if (isNaN(aNum) && isNaN(bNum)) {
        // a、b不是数字，不变
        return 0;
    } else if (isNaN(aNum)) {
        // a不是数字，b是数字，a放后面
        return 1;
    } else if (isNaN(bNum)) {
        // a是数字，b不是数字，不变
        return 0;
    } else {
        return sorter === 'asc' ? (aNum - bNum) : (bNum - aNum);
    }
}

export const safeStr = (str: string, ignore?: string): string => {
    if (str === undefined || str === null) {
        return str;
    }
    if (ignore !== undefined && ignore !== null && ignore !== '') {
        const list = str.split(ignore);
        const safelist = list.map(item => item.replace(/</g, '&lt;').replace(/>/g, '&gt;'));
        return safelist.join(ignore);
    }
    return str?.replace(/</g, '&lt;').replace(/>/g, '&gt;');
};

export function FormItem({ name, style, content, nameStyle }:
{name: React.ReactNode;nameStyle?: React.CSSProperties;style?: React.CSSProperties;content: React.ReactElement}): JSX.Element {
    return (<div style={{
        display: 'inline-block',
        height: '30px',
        lineHeight: '30px',
        margin: '0 20px 10px 0',
        ...style ?? {},
    }}>
        <Label name={name} style={{ width: '80px', display: 'inline-block', ...nameStyle ?? {} }}/>
        {content}
    </div>);
};

export function GroupRankIdsByHost(rankIds: string[]): { hosts: string[]; ranks: Map<string, string[]> } {
    const host = new Set<string>();
    const ranks = new Map<string, string[]>();
    rankIds.forEach(item => {
        const list = item.split(' ');
        if (list.length > 1) {
            host.add(list[0]);
            ranks.set(list[0], [...ranks.get(list[0]) ?? [], item]);
        } else {
            ranks.set('', [...ranks.get('') ?? [], item]);
        }
    });
    return { hosts: Array.from(host), ranks };
};

interface AdaptDprResult {
    canvasWidth: number;
    canvasHeight: number;
}
export const adaptDpr = (canvas: HTMLCanvasElement, ctx: CanvasRenderingContext2D): AdaptDprResult => {
    const dpr = window.devicePixelRatio;
    const canvasWidth = canvas.clientWidth;
    const canvasHeight = canvas.clientHeight;
    canvas.width = canvasWidth * dpr;
    canvas.height = canvasHeight * dpr;
    ctx.scale(dpr, dpr);
    return { canvasWidth, canvasHeight };
};

export const chartColors = [
    '#0062DC',
    '#279C6E',
    '#34B1B9',
    '#6037DB',
    '#FF5432',
    '#0077FF',
    '#FD2F2F',
    '#D53F78',
    '#2F9CE0',
    '#EE891D',
    '#AA38CE',
    '#75A105',
    '#E44222',
    '#0158C5',
    '#248B62',
    '#2F9EA6',
    '#026AE5',
    '#E32A2A',
    '#BF376C',
    '#2A8CC9',
    '#5631C4',
    '#D57B18',
    '#9831B9',
    '#699006',
    '#CB2FOF',
    '#004EB0',
    '#1F7C58',
    '#2A8D93',
    '#005FCC',
    '#CA2425',
    '#AA3260',
    '#247CB3',
    '#4C2CAF',
    '#BE6D17',
    '#882CA4',
    '#5D8004',
    '#FE6E50',
    '#2679E1',
    '#48AA82',
    '#51BCC3',
    '#278AFF',
    '#FD4444',
    '#DB5B8C',
    '#4CAAE4',
    '#7754E0',
    '#F09A3D',
    '#B655D4',
    '#89AF2A',
    '#FF876F',
    '#4D91E5',
    '#67B999',
    '#71C8CE',
    '#4DA0FF',
    '#FD5857',
    '#E178A0',
    '#6DB9E8',
    '#9073E5',
    '#F3AC60',
    '#C374DC',
    '#9EBD51',
];
