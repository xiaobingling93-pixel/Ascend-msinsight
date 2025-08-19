/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React from 'react';
import type { ColumnType } from 'antd/es/table';
import { Input, Button, InputNumber } from '../components/index';
import { SearchOutlined } from '@ant-design/icons';
import type { FilterDropdownProps, Key } from 'antd/es/table/interface';
import { limitInput } from '../utils/Common';
import i18n from '../i18n';
import { ButtonGroup } from './ColumnFilterWithSelection';
import { ColumnFilterIcon } from '../icon/Icon';
import { ValueType } from 'ascend-utils/VirtualUl';
interface FilterProps {
    setSelectedKeys: (selectedKeys: React.Key[]) => void;
    selectedKeys: React.Key[];
    confirm: () => void;
    clearFilters?: () => void;
    dataIndex: string;
}
const state = {
    searchText: '',
    searchedColumn: '',
    searchInput: null,
    searchMinInput: null,
    searchMaxInput: null,
    searchMin: '',
    searchMax: ''
};
const handleSearch = (
    selectedKeys: string[],
    confirm: FilterDropdownProps['confirm'],
    dataIndex: string,
    setSelectedKeys?: (selectedKeys: Key[]) => void,
): void => {
    const trimmed = selectedKeys[0]?.trim() ?? '';
    setSelectedKeys?.([trimmed]);
    confirm();
    state.searchText = trimmed;
    state.searchedColumn = dataIndex;
};
const handleReset = (
    clearFilters: () => void,
    confirm: FilterDropdownProps['confirm'],
    selectedKeys: string[],
    dataIndex: string,
): void => {
    state.searchText = '';
    clearFilters();
    handleSearch(selectedKeys as string[], confirm, dataIndex);
};
const handleRange = (
    selectedKeys: string[],
    confirm: FilterDropdownProps['confirm'],
    dataIndex: string,
    setSelectedKeys?: (selectedKeys: string[]) => void,
): void => {
    const min = selectedKeys[0] === undefined ? '0' : String(selectedKeys[0]);
    const max = selectedKeys[1] === undefined ? '0' : String(selectedKeys[1]);
    setSelectedKeys?.([min, max]);
    confirm();
    state.searchMin = '';
    state.searchMax = '';
    state.searchedColumn = dataIndex;
};
const handleRangeReset = (
    clearFilters: () => void,
    confirm: FilterDropdownProps['confirm'],
    selectedKeys: string[],
    dataIndex: string,
): void => {
    state.searchText = '';
    state.searchMin = '';
    state.searchMax = '';
    clearFilters();
    handleRange(selectedKeys as string[], confirm, dataIndex);
};
const filterSearch = (params: FilterProps, columnTitle: string) => {
    const { setSelectedKeys, selectedKeys, confirm, clearFilters, dataIndex } = params;
    return (
        < div style={{ padding: 8 }} onKeyDown={(e): void => e.stopPropagation()}>
            <Input
                ref={(node): void => {
                    state.searchInput = node as null;
                }}
                placeholder={`${i18n.t('buttonText:Search')} ${i18n.t(`filterColumnName:${columnTitle}`)}`}
                value={selectedKeys[0]}
                onChange={(e): void => setSelectedKeys(e.target.value ? [e.target.value] : [])}
                onPressEnter={(): void => handleSearch(selectedKeys as string[], confirm, dataIndex, setSelectedKeys)}
                style={{ marginBottom: 8, display: 'block' }}
            />
            <ButtonGroup>
                <Button
                    type="primary"
                    onClick={(): void => handleSearch(selectedKeys as string[], confirm, dataIndex, setSelectedKeys)}
                    icon={<SearchOutlined />} size="small" style={{ marginRight: 8 }}
                >{i18n.t('buttonText:Search')}</Button>
                <Button
                    onClick={(): void => clearFilters && handleReset(clearFilters, confirm, selectedKeys as string[], dataIndex)}
                    size="small"
                >{i18n.t('buttonText:Reset')}</Button>
            </ButtonGroup>
        </div >
    )
};
const rangeChange = (value: ValueType | null, selectedKeys: FilterProps['selectedKeys'], setSelectedKeys: FilterProps['setSelectedKeys'], key: number) => {
    const newSelectKeys = [...selectedKeys];
    if (value) {
        newSelectKeys[key] = String(value);
        setSelectedKeys(value ? newSelectKeys : []);
    } else {
        newSelectKeys[key] = '0';
        setSelectedKeys(newSelectKeys);
    }
}
const filterRange = (params: FilterProps) => {
    const { setSelectedKeys, selectedKeys, confirm, clearFilters, dataIndex } = params;
    return (<div style={{ padding: 8 }} onKeyDown={(e): void => e.stopPropagation()}>
        <InputNumber
            ref={(node): void => { state.searchMinInput = node as null; }}
            placeholder={i18n.t('buttonText:Min')}
            value={selectedKeys[0]}
            onChange={(value): void => { rangeChange(value, selectedKeys, setSelectedKeys, 0); }}
            onPressEnter={(): void => handleRange(selectedKeys as string[], confirm, dataIndex, setSelectedKeys)}
            size='small' style={{ marginBottom: 8, marginRight: 8 }}
            min={0} max={1000000000000}
            precision={0}
        />
        <InputNumber
            ref={(node): void => { state.searchMaxInput = node as null; }}
            placeholder={i18n.t('buttonText:Max')}
            value={selectedKeys[1]}
            onChange={(value): void => { rangeChange(value, selectedKeys, setSelectedKeys, 1); }}
            onPressEnter={(): void => handleRange(selectedKeys as string[], confirm, dataIndex, setSelectedKeys)}
            size='small' style={{ marginBottom: 8 }}
            min={0} max={1000000000000}
            precision={0}
        />
        <ButtonGroup>
            <Button
                type="primary"
                onClick={(): void => handleRange(selectedKeys as string[], confirm, dataIndex, setSelectedKeys)}
                size="small" style={{ marginRight: 8 }}
            >{i18n.t('buttonText:Search')}</Button>
            <Button
                onClick={(): void => clearFilters && handleRangeReset(clearFilters, confirm, selectedKeys as string[], dataIndex)}
                size="small"
            >{i18n.t('buttonText:Reset')}</Button>
        </ButtonGroup>
    </div>)
};

export function fetchColumnFilterProps(columnDataIndex: string, columnTitle: string, showRange: boolean = false): ColumnType<any> {
    const getColumnFilterProps = (dataIndex: string): ColumnType<any> => ({
        filterDropdown: ({ setSelectedKeys, selectedKeys, confirm, clearFilters }): React.ReactNode => {
            const params = { setSelectedKeys, selectedKeys, confirm, clearFilters, dataIndex };
            return showRange ? filterRange(params) : filterSearch(params, columnTitle);
        },
        filterIcon: (filtered: boolean) => (
            <ColumnFilterIcon />
        ),
        onFilter: (value, record) =>
            showRange ? record?.[dataIndex] : record?.[dataIndex] && record?.[dataIndex]
                .toString()
                .toLowerCase()
                .includes((value as string).toLowerCase())
        ,
        onFilterDropdownOpenChange: (visible): void => {
            if (visible) {
                limitInput();
            }
        },
    });
    return getColumnFilterProps(columnDataIndex);
}
