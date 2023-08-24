/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { Checkbox, Divider, Select, Pagination } from 'antd';
import { DragDirection, useDraggableContainer } from '../utils/useDraggableContainer';
import { optionDataType, VoidFunction } from '../utils/interface';
import type { EChartsType } from 'echarts';

export const Label = (props: {name: string;style?: object }): JSX.Element => {
    return <span style={{ margin: '0 10px', ...(props.style ?? {}) }}>{props.name} :</span>;
};

export const Space = (props: {length: string | number }): JSX.Element => {
    return <span style={{ display: 'inline-block', marginRight: `${props.length}px` }}/>;
};

export const Container = (props: {title?: JSX.Element | string; content?: JSX.Element;
    style?: any;type?: String;titleClassName?: string;}): JSX.Element => {
    if (props.type === 'headerfixed') {
        return <div className={'header-fixed-content-scroll'} style={{ ...(props.style ?? {}) }}>
            <div className={'container-header'}>{props.title}</div>
            <div>{props.content}</div>
        </div>;
    }
    return <div className={'container-box'} style={{ height: '100%', ...(props.style ?? {}) }}>
        <div className={props.titleClassName ?? 'container-header'} >{props.title}</div>
        <div className={'container-body'}
            style={{ height: 'calc(100% - 20px)', overflow: 'auto' }}>{props.content}</div>
    </div>;
};

export const Tan = (props: {position: string;main: JSX.Element;drag: JSX.Element;id: string;dragSize?: number;style?: object}): JSX.Element => {
    const [view] = useDraggableContainer({ draggableWH: props.dragSize ?? 300, dragDirection: DragDirection.left, open: true });
    const { style = {} } = props;
    return <div style={{ display: 'block', userSelect: 'text', ...style }}>
        <div style={{ display: 'flex', height: '100%', overflow: 'auto' }} className={'tan-box'}>
            {view({
                mainContainer: props.main,
                draggableContainer: props.drag,
                id: props.id,
            })}
        </div>
    </div>;
};

export const MultiSelectWithAll = (props: any): JSX.Element => {
    const { onChange, options = [] } = props;
    const [ checked, setChecked ] = useState(false);
    useEffect(() => {
        if (props.value.length > 0 && options.length > 0) {
            setChecked(props.value.length === options.length);
        }
    }, [ props.value, props.options ]);
    return (
        <Select
            {...props}
            mode="multiple"
            dropdownRender={menu => (
                <div>
                    {menu}
                    <Divider style={{ margin: '2px 0' }} />
                    <div style={{ padding: '4px 8px 8px 8px' }}>
                        <Checkbox
                            checked={checked}
                            onChange={event => {
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
    const [ page, setPage ] = useState({ current: 1, pageSize: 10, total: 0 });
    useEffect(() => {
        props.handlePageChange(page);
    }, [ page.current, page.pageSize ]);
    useEffect(() => {
        setPage({ current: 1, pageSize: 10, total: props.total });
    }, [props.total]);

    return <Pagination
        {...page}
        defaultCurrent={1}
        pageSizeOptions= {[ 10, 20, 50, 100 ] }
        showTotal={(total: number) => (<div style={{ marginRight: '10px' }}>Total {total} items</div>)}
        onChange={(current, pageSize) => { setPage({ ...page, current, pageSize }); }}
        showQuickJumper={page.total / page.pageSize > 5}
        style={{ float: 'right', marginTop: '10px' }}
    />;
};
export const GetPageConfigWhithPageData = (page: { current: number; pageSize: number; total: number },
    setPage: VoidFunction): object => {
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

export const GetPageConfigWhithAllData = (total: number): object => {
    return {
        total,
        showSizeChanger: total > 10,
        pageSizeOptions: [ 10, 20, 50, 100 ],
        showTotal: (total: number) => (<div style={{ marginRight: '10px' }}>Total {total} items</div>),
        hideOnSinglePage: false,
    };
};

let index = 0;
const maxIndex = 3;
const intervalTime = 100;
export const doRequest = (func: (...rest: any[]) => any): void => {
    if (window.request !== undefined) {
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

export function notNullObj(obj: {[prop: string]: any}): boolean {
    let flag = true;
    Object.keys(obj).forEach(key => {
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
    return (
        [
            date.getFullYear(),
            padTo2Digits(date.getMonth() + 1),
            padTo2Digits(date.getDate()),
        ].join('-') +
        ' ' +
        [
            padTo2Digits(date.getHours()),
            padTo2Digits(date.getMinutes()),
            padTo2Digits(date.getSeconds()),
        ].join(':')
    );
}

export function addResizeEvent(echart: EChartsType): void {
    window.addEventListener('resize', function () {
        if (checkDomDisplay(echart.getDom())) {
            echart.resize();
        }
    });
}

export const checkDomDisplay = (dom: HTMLElement): boolean => {
    return dom.offsetParent !== null;
};

export const COLOR = {
    BrightBlue: '#7df7ff',
    Grey20: 'rgb(202,202,202)',
    Grey40: 'rgb(152,152,152)',
    Grey50: 'rgb(123,122,122)',
};
