/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React from 'react';
import { Col, Row } from 'antd';
import type { EChartsType } from 'echarts';

export const Label = (props: {name: string;style?: object }): JSX.Element => {
    return <span style={{ margin: '0 10px', ...(props.style ?? {}) }}>{props.name ? props.name + ' :' : ''} </span>;
};

export const Container = (props: {title?: JSX.Element | string; content?: JSX.Element;
    style?: any;type?: String;titleClassName?: string;}): JSX.Element => {
    if (props.type === 'headerfixed') {
        return <div className={'header-fixed-content-scroll'} style={{ ...(props.style ?? {}) }}>
            <div className={'container-header'}>{props.title}</div>
            <div className={'container-body'}>{props.content}</div>
        </div>;
    }
    return <div className={'container-box'} style={{ height: '100%', ...(props.style ?? {}) }}>
        <div className={props.titleClassName ?? 'container-header'} >{props.title}</div>
        <div className={'container-body'}
            style={{ height: 'calc(100% - 36px)', overflow: 'auto' }}>{props.content}</div>
    </div>;
};
export const HeaderFixedContainer = (
    { style, header, body, headerStyle, bodyStyle, headerProps, bodyProps, ...restProps }:
    {header?: JSX.Element | string; body?: JSX.Element; style?: any;headerStyle?: any;bodyStyle?: any;leftProps?: any;
        rightProps?: any;[props: string]: any;}): JSX.Element => {
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
    style?: any;
    headerStyle?: any;
    bodyStyle?: any;
    type?: string;
    leftProps?: any;
    rightProps?: any;
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

export const GetPageConfigWhithPageData = (page: { current: number; pageSize: number; total: number },
    setPage: Function): object => {
    return {
        ...page,
        showSizeChanger: page.total > 10,
        pageSizeOptions: [ 10, 20, 50, 100 ],
        showTotal: (total: number) => (<div style={{ marginRight: '10px' }}>Total {total} items</div>),
        hideOnSinglePage: false,
        onChange: (current: number, pageSize: number) => { setPage({ ...page, current, pageSize }); },
        showQuickJumper: page.total / page.pageSize > 5,
    };
};

export const checkDomDisplay = (dom: HTMLElement): boolean => {
    return dom?.offsetParent !== null;
};
export function addResizeEvent(echart: EChartsType): void {
    window.addEventListener('resize', function () {
        if (checkDomDisplay(echart.getDom())) {
            echart.resize();
        }
    });
}

class DomVisibilityListener {
    private _visible: boolean = false;
    private readonly _target: HTMLElement | null;

    private _listener: any;

    private readonly _onVisibleChange: Function | undefined;
    constructor(dom: string, onVisibleChange?: Function) {
        this._target = document.getElementById(dom);
        this.visible = this._target?.offsetParent !== null;
        this._onVisibleChange = onVisibleChange;
        this.add();
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

    get visible(): boolean {
        return this._visible;
    }

    set visible(value: boolean) {
        this._visible = value;
    }
}

export const chartVisbilityListener = (dom: string, onVisibleChange?: Function): DomVisibilityListener => {
    return new DomVisibilityListener(dom, (status: boolean) => {
        if (status && onVisibleChange !== undefined) {
            onVisibleChange();
        }
    });
};
