/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
*/
import React from 'react';
import { Col, Row, message, Tooltip, Button } from 'ascend-components';
import { Modal } from 'antd';
import { DownOutlined, ExclamationCircleOutlined, InfoCircleOutlined, QuestionCircleOutlined } from '@ant-design/icons';
import { type ArgsProps } from 'antd/lib/message';
import { useTranslation } from 'react-i18next';
import i18n from 'ascend-i18n';
import type { Theme } from '@emotion/react';
import type { TFunction } from 'i18next';
export const Label = (props: {name: string;style?: object }): JSX.Element => {
    return <span style={{ margin: '0 10px', ...(props.style ?? {}) }}>{props.name ? `${props.name} :` : ''} </span>;
};
export const HeaderFixedContainer = (
    { style, header, body, headerStyle, bodyStyle, headerProps, bodyProps, ...restProps }:
    {
        header?: JSX.Element | string; body?: JSX.Element;
        style?: React.CSSProperties;
        headerStyle?: React.CSSProperties;
        bodyStyle?: React.CSSProperties;
        headerProps?: object;
        bodyProps?: object;
        id?: string;
    },
): JSX.Element => {
    return <div style={{
        height: ' 100%',
        display: 'flex',
        flexDirection: 'column',
        width: '100%',
        ...(style ?? {}),
    }}
    {...restProps}
    >
        <div
            style={{ ...(headerStyle ?? {}) }}
            {...headerProps ?? {}}
        >{header}</div>
        <div
            style={{
                display: 'block',
                flex: '1 1',
                overflow: 'auto',
                ...(bodyStyle ?? {}),
            }}
            {...bodyProps ?? {}}
        >{body}</div>
    </div>;
};

export const LeftRightContainer = ({ style, left, right, headerStyle, bodyStyle, type, leftProps, rightProps }:
{
    left?: JSX.Element | string;
    right?: JSX.Element;
    style?: React.CSSProperties;
    headerStyle?: React.CSSProperties;
    bodyStyle?: React.CSSProperties;
    type?: string;
    leftProps?: object;
    rightProps?: object;
}): JSX.Element => {
    if (type === 'simple') {
        return <Row><Col span={12}>{left}</Col><Col span={12}>{right}</Col></Row>;
    }
    return <div
        style={{
            height: ' 100%',
            display: 'flex',
            flexDirection: 'row',
            width: '100%',
            ...(style ?? {}),
        }}>
        <div style={{
            flex: '0 0 50%',
            height: ' 100%',
            overflow: 'auto',
            ...(headerStyle ?? {}),
        }}
        {...leftProps ?? {}}>
            {left}
        </div>
        <div style={{
            display: 'block',
            flex: '0 0 50%',
            overflowX: 'auto',
            height: ' 100%',
            overflow: 'auto',
            ...(bodyStyle ?? {}),
        }} {...rightProps ?? {}}>{right}</div>
    </div>;
};

export const Loading = ({ size = 20, style = {} }: {size?: number;style?: object}): JSX.Element => {
    return (<div className={'loading'}
        style={{ width: `${size}px`, height: `${size}px`, ...style }}></div>);
};

export function notNull(val: unknown): boolean {
    return val !== undefined && val !== null && val !== '';
}

export function isNull(val: unknown): boolean {
    return val === undefined || val === null || val === '';
}

export function isViewable(dom: Element, fixed?:
{ fixedtop?: number; fixedbottom?: number; fixedleft?: number; fixedright?: number }): boolean {
    const { fixedtop = 0, fixedbottom = 0, fixedleft = 0, fixedright = 0 } = fixed ?? {};
    const height = window.innerHeight || document.documentElement.clientHeight;
    const width = window.innerWidth || document.documentElement.clientWidth;
    const { top, right, bottom, left } = dom.getBoundingClientRect();
    return !(bottom <= fixedtop || top >= height - fixedbottom || right <= fixedleft || left >= width - fixedright);
}

export function addClass(dom: Element, className: string): void {
    dom.classList.add(className);
}
export function removeClass(dom: Element, className: string): void {
    dom.classList.remove(className);
}
export function syncScroller(...args: Array<HTMLElement | null>): () => void {
    const nodes: HTMLElement[] = Array.prototype.filter.call(args, item => item instanceof HTMLElement);
    const max = nodes.length;
    if (!max || max === 1) {
        return () => {};
    }
    let sign = 0; // 用于标注

    function event(this: HTMLElement): void {
        if (!sign) { // 标注为 0 时 表示滚动起源
            sign = max - 1;
            const top: number = this.scrollTop;
            nodes.forEach((node: HTMLElement) => {
                if (node !== this) { // 同步所有除自己以外节点
                    node.scrollTo(node.scrollLeft, top);
                }
            });
        } else {
            --sign;
        } // 其他节点滚动时 标注减一
    }

    nodes.forEach((ele, index) => {
        ele.addEventListener('scroll', event);
    });

    return () => {
        nodes.forEach((ele, index) => {
            ele.removeEventListener('scroll', event);
        });
    };
}

type MessageType = 'info' | 'success' | 'error' | 'warn' | 'warning' | 'confirm';

export const confrimMessage = (type: MessageType, title: string): void => {
    const icon = type === 'error' ? <ExclamationCircleOutlined /> : <InfoCircleOutlined />;
    Modal[type]({
        title,
        icon,
        okText: 'confirm',
    });
};

type MessageLevelType = 'success' | 'warning' | 'error' | 'loading' | 'info' ;
export function showMessage(type: MessageLevelType, param: string | ArgsProps): void {
    if (typeof param === 'string') {
        message[type]({
            content: param,
            duration: 5,
        });
    } else {
        message[type](param);
    }
};
let logindex: number = 0;
const logRecord: Record<number, unknown> = {};
export function log(...param: unknown[]): void {
    logRecord[logindex++ % 1000] = param;
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

export const GetPageConfigWhithPageData = (page: { current: number; pageSize: number; total: number },
    setPage: unknown, pageSizeOptions?: number[],
): object => {
    return {
        ...page,
        showSizeChanger: page.total > 10,
        pageSizeOptions: pageSizeOptions ?? [10, 20, 50, 100],
        showTotal: (total: number): React.ReactNode => (<div style={{ marginRight: '10px' }}>{i18n.t('PaginationTotal', { total })}</div>),
        hideOnSinglePage: false,
        onChange: (current: number, pageSize: number): void => {
            (setPage as (value: unknown) => void)({ ...page, current, pageSize });
        },
        showQuickJumper: page.pageSize !== 0 && page.total / page.pageSize > 5,
    };
};

export const useHit = (): JSX.Element => {
    const { t } = useTranslation('details');
    return (<Tooltip title={t('NAHit')}>
        <QuestionCircleOutlined style={{ cursor: 'pointer', margin: '0 3px' }}/>
    </Tooltip>);
};

export const getFormatNum = (str?: string | number): number | string => {
    const num = Number(str);
    if (!isNaN(num) && num !== 0) {
        // 保留两位有效数字
        let decimal = Math.log10(Math.abs(num));
        decimal = decimal >= 0 ? 0 : -decimal;
        return Number(num.toFixed(2 + decimal));
    }
    return str ?? '';
};

export const getFormatNumReturnEmpty = (str?: string | number): number | string => {
    if (str === '') {
        return '';
    }
    return getFormatNum(str);
};

export const getContextElement = (text: string, record: any, theme: Theme): JSX.Element => {
    if (isNaN(Number(text))) {
        return <div>{text}</div>;
    }
    if (record.source === 'Difference') {
        return <div style={{ color: Number(text) >= 0 ? theme.successColor : theme.dangerColor }}>{text}</div>;
    } else {
        return <div>{Number(text)}</div>;
    }
};

export const renderExpandColumn = (record: any, setExpandedKeys: React.Dispatch<React.SetStateAction<string[]>>, t: TFunction): JSX.Element => {
    return record.source === 'Difference'
        ? (<Button type="link"
            onClick={(): void => {
                setExpandedKeys((pre: any) => {
                    const list = [...pre];
                    const keyIndex = list.indexOf(record.key);
                    if (keyIndex === -1) {
                        list.push(record.key);
                    } else {
                        list.splice(keyIndex, 1);
                    }
                    return list;
                });
            }}>{t('See More')}<DownOutlined/></Button>)
        : <></>;
};
