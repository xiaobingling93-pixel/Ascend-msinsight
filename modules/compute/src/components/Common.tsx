/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */
import React, { useMemo, useState } from 'react';
import { message, Tooltip, Button } from '@insight/lib/components';
import { Modal } from 'antd';
import { DownOutlined, ExclamationCircleOutlined, InfoCircleOutlined, QuestionCircleOutlined } from '@ant-design/icons';
import { type ArgsProps } from 'antd/lib/message';
import { useTranslation } from 'react-i18next';
import i18n from '@insight/lib/i18n';
import type { Theme } from '@emotion/react';
import type { TFunction } from 'i18next';
import { Resizor } from '@insight/lib/resize';
import styled from '@emotion/styled';
import { useWatchDomResize } from '@insight/lib/utils';
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

const LeftRightBox = styled.div`
  height: 100%;
  width: 100%;
  .left {
    float: left;
  }
  .right {
    float: right;
  }
  .left, .right {
    display: inline-block;
    position: relative;
    height: calc(100% - 6px);
  }
  .resizor {
    cursor: col-resize;
  }
`;

export const LeftRightContainer = ({
    left, right, style, headerStyle, bodyStyle, flexStyle, leftPercent, className,
    leftWidth: initLeftWidth, leftProps = {}, rightProps = {}, minWidth = 50, flex = false,
}:
{
    left?: React.ReactNode;
    right?: React.ReactNode;
    style?: React.CSSProperties;
    headerStyle?: React.CSSProperties;
    bodyStyle?: React.CSSProperties;
    leftProps?: object;
    rightProps?: object;
    minWidth?: number;
    flex?: boolean;
    leftPercent?: number;
    leftWidth?: number;
    flexStyle?: React.CSSProperties;
    className?: string;
}): JSX.Element => {
    const [leftWidth, setLeftWidth] = useState(initLeftWidth ?? -1);
    const leftWidthStyle = useMemo(() => leftWidth > 0 ? `${leftWidth}px` : `${leftPercent ?? 50}%`, [leftWidth]);
    const rightWidthStyle = useMemo(() => leftWidth > 0 ? `calc(100% - ${leftWidth}px)` : `${100 - (leftPercent ?? 50)}%`, [leftWidth]);
    const [boxWidth, ref] = useWatchDomResize<HTMLDivElement>('width');

    const handleResizeEvent = (diff: number, width: number): void => {
        // 左DOM不小于最小宽度，不大于最大Box宽度
        const checkSize = width >= minWidth && width <= boxWidth - minWidth;
        if (checkSize) {
            setLeftWidth(width);
        }
    };

    return <LeftRightBox ref={ref} style={style} className={className}>
        <div className="left" style={{ ...headerStyle, width: leftWidthStyle }} {...leftProps}>
            {flex && <Resizor onResize={ handleResizeEvent} style={flexStyle}/>}
            {left}
        </div>
        <div className="right" style={{ ...bodyStyle, width: rightWidthStyle }} {...rightProps }>
            {right}
        </div>
    </LeftRightBox>;
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
export function syncScroller(args: Array<HTMLElement | null>): () => void {
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
    if (num === 0) {
        return '0';
    }
    return str ?? '';
};

export const getFormatNumReturnEmpty = (str?: string | number): number | string => {
    if (str === '') {
        return '';
    }
    return getFormatNum(str);
};

export const getContextElement = (text: string, record: any, theme: Theme, t: TFunction): JSX.Element => {
    if (isNaN(Number(text))) {
        return <div>{text}</div>;
    }
    if (record.source === t('Difference')) {
        return <div style={{ color: Number(text) >= 0 ? theme.successColor : theme.dangerColor }}>{text}</div>;
    } else {
        return <div>{Number(text)}</div>;
    }
};

export const renderExpandColumn = (record: any, setExpandedKeys: React.Dispatch<React.SetStateAction<string[]>>, t: TFunction): JSX.Element => {
    return record.source === t('Difference')
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
