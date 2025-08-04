/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React from 'react';
import type { ColumnType } from 'antd/es/table';
import { Input, Button } from '../components/index';
import { SearchOutlined } from '@ant-design/icons';
import type { FilterDropdownProps, Key } from 'antd/es/table/interface';
import { limitInput } from '../utils/Common';
import i18n from '../i18n';
import { ButtonGroup } from './ColumnFilterWithSelection';
import { ColumnFilterIcon } from '../icon/Icon';

const state = {
    searchText: '',
    searchedColumn: '',
    searchInput: null,
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

export function fetchColumnFilterProps(columnDataIndex: string, columnTitle: string): ColumnType<any> {
    const getColumnFilterProps = (dataIndex: string): ColumnType<any> => ({
        filterDropdown: ({ setSelectedKeys, selectedKeys, confirm, clearFilters }): React.ReactNode => (
            <div style={{ padding: 8 }} onKeyDown={(e): void => e.stopPropagation()}>
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
            </div>
        ),
        filterIcon: (filtered: boolean) => (
            <ColumnFilterIcon/>
        ),
        onFilter: (value, record) =>
            record[dataIndex]
                .toString()
                .toLowerCase()
                .includes((value as string).toLowerCase()),
        onFilterDropdownOpenChange: (visible): void => {
            if (visible) {
                limitInput();
            }
        },
    });
    return getColumnFilterProps(columnDataIndex);
}
