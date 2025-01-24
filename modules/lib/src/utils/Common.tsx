/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import * as React from 'react';
import styled from '@emotion/styled';
import BaseContainer from '../container/BaseContainer';
import { MIDescriptions, MIDescriptionsItem } from '../MIDescriptions';
import COLOR from './Color';
import { chartVisbilityListener, getAdaptiveEchart, disposeAdaptiveEchart, getDefaultChartOptions, getLegendStyle } from './EchartUtils';
import { Empty } from '../components/index';
import { useTheme } from '@emotion/react';
import { useTranslation } from 'react-i18next';
import { AlarmIcon, BulbIcon } from '../icon/Icon';
import ResizeObserver from 'resize-observer-polyfill';
export { localStorageService, LocalStorageKey } from './local-storage';
export { customConsole } from './Console';
export {
    BaseContainer,
    MIDescriptions,
    MIDescriptionsItem,
    COLOR,
    chartVisbilityListener,
    getAdaptiveEchart,
    disposeAdaptiveEchart,
    getDefaultChartOptions,
    getLegendStyle,
};

const BREAK_LINE_REGEXP = /\r\n|\r|\n/g;
export const StyledEmpty = ({ descriptor, style }:
{ descriptor?: string; style?: object; translation?: any}): JSX.Element => {
    const theme = useTheme();
    const { t } = useTranslation();
    return (
        <Empty
            image={Empty.PRESENTED_IMAGE_SIMPLE}
            style={style}
            imageStyle={{ height: 30 }}
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
    margin-bottom: 5px;
    display: flex;
    flex-direction: row;
    & > div:first-child {
        flex: 0 0 auto;
        vertical-align: top;
        & > span,div {
            margin-right: 8px;
            font-weight: bold;
        }
    }
    & > div:nth-child(2) {
        flex: auto;
        word-break: break-all;
    }
`;

interface IHitProps {
    title?: React.ReactNode;
    text: React.ReactNode | React.ReactNode[];
    style?: React.CSSProperties ;
    type?: string;
}
export function Hit(props: IHitProps): JSX.Element {
    const { type, title, text, style, ...restProps } = props;
    const icon = type === 'alarm' ? <AlarmIcon/> : <BulbIcon/>;
    const splitText = (str: React.ReactNode): React.ReactNode => {
        if (typeof str !== 'string') {
            return str;
        }
        const list = str.split(BREAK_LINE_REGEXP);
        return list.map((item, index) => (<div key={index}>{item}</div>));
    };
    return <StyledAdvice style={style} {...restProps}>
        <div>
            {icon}
            {(title !== undefined && title !== null) ? (<span>{title}</span>) : <></>}
        </div>
        <div>{Array.isArray(text) ? text.map((item, index) => (<div key={index}>{splitText(item)}</div>)) : splitText(text)}</div>
    </StyledAdvice>;
}
export function Advice(props: IHitProps): JSX.Element {
    const { t } = useTranslation();
    return <Hit type={'advice'} title={`${t('Advice')}:`} {...props}/>;
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

export function getUsableVal<T>(val: T, options: Array<{value: T}>, defaultVal: T, func?: (inputArray: Array<{value: T}>) => T): T {
    if (options.length === 0) {
        return defaultVal;
    }
    if (options.find(item => item.value === val)) {
        return val;
    }
    if (func !== undefined) {
        return func(options);
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
export function formatDecimal(val: number | string, fixed = 2): number {
    const num = Number(val);
    if (isNaN(num) || num === 0) {
        return num;
    } else {
        let decimal = Math.floor(Math.log10(Math.abs(num)));
        decimal = decimal >= 0 ? 0 : -decimal;
        return Number(num.toFixed(decimal + fixed));
    }
}

const htmlEscapeMap = {
    '&': '&amp;',
    '<': '&lt;',
    '>': '&gt;',
    '"': '&quot;',
    '\'': '&#39;',
    '/': '&#x2F;',
};

const escapeHTML = (input: string): string => {
    return input.replace(/[&<>"'/]/g, match => htmlEscapeMap[match as keyof typeof htmlEscapeMap]);
};

export const safeStr = (val: string | number, ignore?: string): string => {
    if (typeof val === 'number') {
        return val.toString();
    }
    if (typeof val !== 'string') {
        return '';
    }
    if (ignore) {
        return val.split(ignore).map(item => escapeHTML(item)).join(ignore);
    }
    return escapeHTML(val);
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

export type SizeProp = 'width' | 'height';
export function useWatchDomResize<T extends Element>(prop: SizeProp): [number, React.RefObject<T>] {
    const [rect, setRect] = React.useState<DOMRectReadOnly | null>(null);
    const ref = React.useRef<T>(null);
    React.useEffect(() => {
        const observer = new ResizeObserver(([entry]) => {
            window.requestAnimationFrame(() => {
                setRect(entry.contentRect);
            });
        });
        if (ref.current) {
            observer.observe(ref.current);
        }
        return () => {
            observer.disconnect();
        };
    }, []);
    return [rect?.[prop] ?? 0, ref];
}

const removePrototypePollution = (obj: any): void => {
    if (obj && typeof obj === 'object') {
        for (const key in obj) {
            if (key === '__proto__' || key === 'constructor') {
                delete obj[key];
            } else if (typeof obj[key] === 'object') {
                removePrototypePollution(obj[key]);
            }
        }
    }
};

export const safeJSONParse = (str: any, defaultValue: any = null): any => {
    try {
        const res = JSON.parse(str);
        removePrototypePollution(res);
        return res;
    } catch (error) {
        return defaultValue;
    }
};

export const disableShortcuts = (forbiddenComboKeys = [], forbiddenSingleKeys = []): void => {
    document.addEventListener('keydown', (e) => {
        const defaultForbiddenComboKeys = ['f', 'p', 'g', 'j', 'r'];
        const defaultForbiddenSingleKeys = ['F3', 'F5', 'F7'];
        const comboKeys = forbiddenComboKeys.length ? forbiddenComboKeys : defaultForbiddenComboKeys;
        const singleKeys = forbiddenSingleKeys.length ? forbiddenSingleKeys : defaultForbiddenSingleKeys;
        const isCtrlCombo = (e.ctrlKey || e.metaKey) && comboKeys.includes(e.key.toLowerCase());

        if (isCtrlCombo || singleKeys.includes(e.key)) {
            e.preventDefault();
        }
    });
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
    '#CB2F0F',
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

export function clamp(value: number, min: number, max: number): number {
    return Math.min(Math.max(value, min), max);
}

export function formateMicrosecond(t: number): string {
    if (isNaN(t)) {
        return '';
    }
    let leftTime = t;
    // 小时级
    if (t >= 1000 * 1000 * 60 * 60) {
        const h = Math.floor(leftTime / (1000 * 1000 * 60 * 60));
        leftTime = leftTime % (1000 * 1000 * 60 * 60);
        const m = Math.floor(leftTime / (1000 * 1000 * 60));
        leftTime = leftTime % (1000 * 1000 * 60);
        const s = Number((leftTime / (1000 * 1000)).toFixed(2));
        return `${h}h${m}m${s}s`;
    }
    // 分钟级
    if (t >= 1000 * 1000 * 60) {
        const m = Math.floor(leftTime / (1000 * 1000 * 60));
        leftTime = leftTime % (1000 * 1000 * 60);
        const s = Number((leftTime / (1000 * 1000)).toFixed(2));
        return `${m}m${s}s`;
    }
    // 秒级
    if (t >= 1000 * 1000) {
        const s = Number((leftTime / (1000 * 1000)).toFixed(2));
        return `${s}s`;
    }
    // 毫秒级
    if (t >= 1000) {
        const s = Number((leftTime / (1000)).toFixed(2));
        return `${s}ms`;
    }
    return `${t}μs`;
}

const CompareDiv = styled.div`
    width: 100%;
    overflow: hidden;
    white-space: nowrap;
    text-overflow: ellipsis;
    word-break: keep-all;
    &.positive {
        color: ${(props): string => props.theme.successColor};
    }
    &.negative {
        color: ${(props): string => props.theme.dangerColor};
    }
`;

export const CompareNumber = ({ data }: {data: string | number}): JSX.Element => {
    const dataNum = Number(data);
    if (isNaN(dataNum)) {
        return <>{data}</>;
    }
    return <CompareDiv className={dataNum >= 0 ? 'positive' : 'negative'} title={`${data}`}>{data}</CompareDiv>;
};

export const FONT_FAMILY = '\'Inter\', -apple-system, BlinkMacSystemFont, \'Segoe UI\', Roboto, Oxygen, Ubuntu, Cantarell, \'Fira Sans\', \'Droid Sans\', sans-serif';

export const disableIframePointerEvent = (): void => {
    if (window.document.querySelectorAll('iframe').length !== 0) {
        window.document.querySelectorAll('iframe').forEach((item: HTMLIFrameElement) => {
            (item.style as any)['pointer-events'] = 'none';
        });
    }
};
export const recoverIframePointerEvent = (): void => {
    if (window.document.querySelectorAll('iframe').length !== 0) {
        window.document.querySelectorAll('iframe').forEach((item: HTMLIFrameElement) => {
            (item.style as any)['pointer-events'] = 'auto';
        });
    }
};
