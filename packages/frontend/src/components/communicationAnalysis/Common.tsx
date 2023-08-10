/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { Checkbox, Divider, Select, Pagination } from 'antd';
import { DragDirection, useDraggableContainer } from '../../utils/useDraggableContainer';
import { optionDataType } from '../../utils/interface';

export const Label = (props: {name: string;style?: object }): JSX.Element => {
    return <span style={{ margin: '0 10px', ...(props.style ?? {}) }}>{props.name} :</span>;
};

export const Space = (props: {length: string | number }): JSX.Element => {
    return <span style={{ display: 'inline-block', marginRight: `${props.length}px` }}/>;
};

export const Container = (props: {title?: JSX.Element | string; content?: JSX.Element;style?: any}): JSX.Element => {
    return <div style={{ height: '100%', margin: '10px', ...(props.style ?? {}) }}>
        <div style={{
            fontWeight: 'bold', background: '#fafafa', height: '3rem', lineHeight: '3rem', fontSize: '1.5rem', paddingLeft: '10px',
        }}>{props.title}</div>
        <div style={{ height: 'calc(100% - 20px', overflow: 'auto' }}>{props.content}</div>
    </div>;
};

export const Tan = (props: {position: string;main: JSX.Element;drag: JSX.Element;id: string;dragSize?: number;style?: object}): JSX.Element => {
    const [view] = useDraggableContainer({ draggableWH: props.dragSize ?? 300, dragDirection: DragDirection.left, open: true });
    const { style = {} } = props;
    return <div style={{ display: 'block', ...style }}>
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
                                if (event.target.checked) {
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
        pageSizeOptions= {[ 2, 10, 20, 50, 100 ] }
        showTotal={(total: number) => (<div style={{ marginRight: '10px' }}>Total {total} items</div>)}
        onChange={(current, pageSize) => { setPage({ ...page, current, pageSize }); }}
        showQuickJumper={page.total / page.pageSize > 5}
        style={{ float: 'right', marginTop: '10px' }}
    />;
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
