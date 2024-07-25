/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React, { useEffect, useState, useMemo, type CSSProperties } from 'react';
import { Checkbox } from '../components/index';
import type { CheckboxChangeEvent } from 'antd/es/checkbox';
import i18n from '../i18n';
import { useWatchVirtualRender } from './VirtualRenderUtils';

export interface CheckItem {
    text: React.ReactNode;
    value: ValueType;
    checked?: boolean;
    index?: number;
}

export type ValueType = string | number;

interface IProps {
    items?: CheckItem[];
    height?: number;
    itemHeight?: number;
    searchText?: string;
    style?: CSSProperties;
    onChange?: (values: ValueType[]) => void;
}

const ITEM_HEIGHT = 22;
const BOX_HEIGHT = 400;
const ALL_SELECTED_INDEX = -1;
export default function VirtalUl({ items = [], height = BOX_HEIGHT, itemHeight = ITEM_HEIGHT, searchText = '', style, onChange }:
IProps): JSX.Element {
    const [allSelected, setAllSelected] = useState(false);
    const [curChange, setCurChange] = useState<{ curIndex?: number; checked?: boolean }>({ });
    const [checkItems, setCheckItems] = useState<CheckItem[]>([]);
    const displayItems = useMemo(() => {
        if (searchText === '') {
            return checkItems;
        } else {
            return checkItems.filter(item => String(item.text).includes(String(searchText)));
        }
    }, [checkItems, searchText]);
    const { data, boxRef, targetRef } = useWatchVirtualRender({
        visibleHeight: height,
        itemHeight,
        dataSource: displayItems,
    });

    useEffect(() => {
        const newCheckItems = items.map((item, index) => ({ ...item, index, checked: false }));
        setCheckItems(newCheckItems);
        setAllSelected(false);
    }, [items]);

    useEffect(() => {
        const { curIndex, checked } = curChange;
        if (curIndex === undefined || checked === undefined) {
            return;
        }
        let newCheckItems;
        if (curIndex === ALL_SELECTED_INDEX) {
            newCheckItems = checkItems.map((item, index) => {
                if (searchText === '' || (searchText !== '' && String(item.text).includes(String(searchText)))) {
                    return { ...item, checked: checked ?? item.checked };
                } else {
                    return { ...item };
                }
            });
        } else {
            newCheckItems = [...checkItems];
            newCheckItems[curIndex].checked = checked;
        }
        setCheckItems(newCheckItems);

        if (curIndex !== ALL_SELECTED_INDEX) {
            const isAllSelected = checked && checkItems.findIndex(item => item.index !== curIndex && !item.checked) < 0;
            setAllSelected(isAllSelected);
        }
    }, [curChange]);

    useEffect(() => {
        if (onChange !== undefined) {
            const values = checkItems.reduce<ValueType[]>((pre, cur) => {
                if (cur.checked) {
                    return [...pre, cur.value];
                }
                return pre;
            }, []);
            onChange(values);
        }
    }, [checkItems]);

    return <div style={{ padding: '8px 0 8px 16px', textAlign: 'left', ...style ?? {} }}>
        <div>
            <Checkbox checked={allSelected} onChange={(e: CheckboxChangeEvent): void => {
                setAllSelected(e.target.checked);
                setCurChange({ curIndex: ALL_SELECTED_INDEX, checked: e.target.checked });
            }}>{i18n.t('translation:All')}</Checkbox>
        </div>
        <div ref={boxRef} style={{ maxHeight: `${height}px`, overflow: 'auto' }}>
            <ul ref={targetRef}
                style={{ listStyleType: 'none', paddingInlineStart: '1rem' }}>
                {
                    data.map(item => (
                        <li key={item.index}>
                            <Checkbox
                                checked={item.checked}
                                onChange={(e: CheckboxChangeEvent): void => {
                                    setCurChange({ curIndex: item.index, checked: e.target.checked });
                                }}>
                                {item.text}
                            </Checkbox>
                        </li>
                    ))
                }
            </ul>
        </div>
    </div>;
}
