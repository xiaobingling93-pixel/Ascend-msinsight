/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import React, { useEffect, useState } from 'react';
import { Checkbox, Divider, Select, Pagination, Tooltip } from 'antd';
import type { optionDataType, VoidFunction } from '../utils/interface';
import type { EChartsType } from 'echarts';
import i18n from '../i18n';
import type { TooltipProps } from 'antd/lib/tooltip';
import { useTheme } from '@emotion/react';

export const Label = (props: {name: string;style?: object }): JSX.Element => {
    return <span style={{ margin: '0 10px', ...(props.style ?? {}) }}>{props.name ? `${props.name} :` : ''} </span>;
};

export const Space = (props: {length: string | number }): JSX.Element => {
    return <span style={{ display: 'inline-block', marginRight: `${props.length}px` }}/>;
};

export const Container = (props: {title?: JSX.Element | string; content?: JSX.Element;
    style?: any;type?: string;titleClassName?: string;
    headerStyle?: object;bodyStyle?: object;
}): JSX.Element => {
    if (props.type === 'headerfixed') {
        return <div className={'header-fixed-content-scroll'} style={{ ...(props.style ?? {}) }}>
            <div className={'container-header'}>{props.title}</div>
            <div className={'container-body'}>{props.content}</div>
        </div>;
    }
    return <div className={'container-box'} style={{ height: '100%', ...(props.style ?? {}) }}>
        <div className={props.titleClassName ?? 'container-header'} style={props.headerStyle ?? {}}>{props.title}</div>
        <div className={'container-body'}
            style={{ height: 'calc(100% - 20px)', overflow: 'auto', ...props.bodyStyle ?? {} }}>{props.content}</div>
    </div>;
};

export const Loading = ({ size = 20, style = {} }: {size?: number;style?: object}): JSX.Element => {
    return (<div className={'loading'}
        style={{ width: `${size}px`, height: `${size}px`, ...style }}></div>);
};

export const MultiSelectWithAll = (props: any): JSX.Element => {
    const { onChange, options = [] } = props;
    const [checked, setChecked] = useState(false);
    useEffect(() => {
        if (props.value.length > 0 && options.length > 0) {
            setChecked(props.value.length === options.length);
        }
    }, [props.value, props.options]);
    return (
        <Select
            {...props}
            mode="multiple"
            dropdownRender={(menu): JSX.Element => (
                <div>
                    {menu}
                    <Divider style={{ margin: '2px 0' }} />
                    <div style={{ padding: '4px 8px 8px 8px' }}>
                        <Checkbox
                            checked={checked}
                            onChange={(event): void => {
                                setChecked(event.target.checked);
                                if (event?.target.checked) {
                                    onChange(options.map((item: optionDataType) => item.value));
                                } else {
                                    onChange([]);
                                }
                            }
                            }>All</Checkbox>
                    </div>
                </div>
            )}
        />);
};

export const PaginationWhithPgaeData = (props: any): JSX.Element => {
    const [page, setPage] = useState({ current: 1, pageSize: 10, total: 0 });
    useEffect(() => {
        props.handlePageChange(page);
    }, [page.current, page.pageSize]);
    useEffect(() => {
        setPage({ current: 1, pageSize: 10, total: props.total });
    }, [props.total]);

    return <Pagination
        {...page}
        defaultCurrent={1}
        pageSizeOptions= {[10, 20, 50, 100] }
        showTotal={(total: number): React.ReactElement => (<div style={{ marginRight: '10px' }}>{i18n.t('PaginationTotal', { total })}</div>)}
        onChange={(current, pageSize): void => { setPage({ ...page, current, pageSize }); }}
        showQuickJumper={page.total / notZero(page.pageSize) > 5}
        style={{ float: 'right', marginTop: '10px' }}
    />;
};
export const getPageConfigWithPageData = (page: { current: number; pageSize: number; total: number },
    setPage: VoidFunction): object => {
    return {
        ...page,
        showSizeChanger: page.total > 10,
        pageSizeOptions: [10, 20, 50, 100],
        showTotal: (total: number) => (<div style={{ marginRight: '10px' }}>{i18n.t('PaginationTotal', { total })}</div>),
        hideOnSinglePage: false,
        onChange: (current: number, pageSize: number): void => { setPage({ ...page, current, pageSize }); },
        showQuickJumper: page.total / notZero(page.pageSize) > 5,
    };
};

export const getPageConfigWithAllData = (total: number): object => {
    return {
        total,
        showSizeChanger: total > 10,
        pageSizeOptions: [10, 20, 50, 100],
        showTotal: (totalNum: number): React.ReactElement => (<div style={{ marginRight: '10px' }}>{i18n.t('PaginationTotal', { total: totalNum })}</div>),
        hideOnSinglePage: false,
    };
};

let index = 0;
const maxIndex = 3;
const intervalTime = 100;
export const doRequest = (func: (...rest: any[]) => any): void => {
    if (window.requestData !== undefined) {
        index = 0;
        func();
    } else if (index < maxIndex) {
        index++;
        setTimeout(() => {
            doRequest(func);
        }, intervalTime);
    } else {
        func();
    }
};

export function notNull(val: any): boolean {
    return val !== undefined && val !== null && val !== '';
}

export function isNull(val: any): boolean {
    return val === undefined || val === null || val === '';
}

export function notNullObj(obj: {[prop: string]: any}, keys?: string[]): boolean {
    let flag = true;
    const keylist = keys ?? Object.keys(obj);
    keylist.forEach(key => {
        if (isNull(obj[key])) {
            flag = false;
        }
    });
    return flag;
}

function padTo2Digits(num: number): string {
    return num.toString().padStart(2, '0');
}

export function formatDate(date: Date): string {
    const dateStr = [date.getFullYear(), padTo2Digits(date.getMonth() + 1), padTo2Digits(date.getDate())].join('-');
    const timeStr = [padTo2Digits(date.getHours()), padTo2Digits(date.getMinutes()), padTo2Digits(date.getSeconds())].join(':');
    return `${dateStr} ${timeStr}`;
}

export function addResizeEvent(echart: EChartsType): void {
    window.addEventListener('resize', (): void => {
        if (checkDomDisplay(echart.getDom())) {
            echart.resize();
        }
    });
    window.addEventListener('load', (): void => {
        if (checkDomDisplay(echart.getDom())) {
            echart.resize();
        }
    });
}

export const checkDomDisplay = (dom: HTMLElement): boolean => {
    return dom?.offsetParent !== null;
};

export const COLOR = {
    BrightBlue: '#7df7ff',
    Grey20: '#cacaca',
    Grey40: '#989898',
    Grey50: '#7b7a7a',
    Band0: '#f82d18',
    Band1: '#eac299',
    Band2: '#c7eef5',
    Band3: '#0177ff',
};

interface AnyFunction {
    (...rest: any[]): any;
}

const listenerMap: {[props: string]: any} = {};

class DomVisibilityListener {
    private _visible: boolean = false;
    private readonly _target: HTMLElement | null;

    private _listener: any;

    private readonly _onVisibleChange: AnyFunction | undefined;
    constructor(dom: string, onVisibleChange?: AnyFunction) {
        if (listenerMap[dom] !== undefined && listenerMap[dom] !== null) {
            listenerMap[dom].clear();
            listenerMap[dom] = null;
        }
        listenerMap[dom] = this;
        this._target = document.getElementById(dom);
        this.visible = this._target?.offsetParent !== null;
        this._onVisibleChange = onVisibleChange;
        this.add();
    }

    get visible(): boolean {
        return this._visible;
    }

    set visible(value: boolean) {
        this._visible = value;
    }

    add(): void {
        this._listener = setTimeout(() => {
            const newStatus = this._target?.offsetParent !== null;
            if (newStatus !== this.visible && this._onVisibleChange !== undefined) {
                if (this._onVisibleChange !== undefined) {
                    this._onVisibleChange(newStatus);
                }
            }
            this.visible = newStatus;
            this.add();
        }, 100);
    }

    clear(): void {
        if (this._listener !== null) {
            clearTimeout(this._listener);
        }
    }
}

export const chartVisbilityListener = (dom: string, onVisibleChange?: AnyFunction): DomVisibilityListener => {
    return new DomVisibilityListener(dom, (status: boolean) => {
        if (status && onVisibleChange !== undefined) {
            onVisibleChange();
        }
    });
};

export const getDecimalCount = (num: number): number => {
    const numStr = num.toString();
    const arr = numStr.split('.');
    if (arr[1]?.length) {
        return arr[1].length;
    }
    return 0;
};

export const notZero = (num: number, replace = 1): number => {
    const replaceNum = replace === 0 ? 1 : replace;
    return num === 0 ? replaceNum : num;
};

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

export const commonEchartsOptions: {
    tooltip: any;
    splitLineY: any;
} = {
    tooltip: {
        trigger: 'axis',
        axisPointer: {
            type: 'cross',
            crossStyle: {
                color: COLOR.BrightBlue,
                type: 'solid',
            },
        },
    },
    splitLineY: {
        lineStyle: {
            color: COLOR.Grey20,
            type: 'dashed',
        },
    },
};

export const StyledTooltip: React.FC<TooltipProps> = ({ children, overlayInnerStyle, ...props }: TooltipProps) => {
    const theme = useTheme();
    return <Tooltip
        color={theme.tooltipBGColor}
        overlayInnerStyle={
            {
                borderRadius: '12px',
                color: theme.tooltipFontColor,
                boxShadow: theme.tooltipBoxShadow,
                whiteSpace: 'pre-wrap',
                fontSize: 14,
            }
        }
        mouseEnterDelay={0.3}
        {...props}
    >
        {children}
    </Tooltip>;
};
