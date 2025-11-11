/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React, { useEffect, useState } from 'react';
import { Button, Input } from '../components/index';
import { limitInput } from '../utils';
import type { ColumnType } from 'antd/es/table';
import type { ColumnFilterItem, FilterDropdownProps } from 'antd/es/table/interface';
import type { CheckItem, ValueType } from '../utils/VirtualUl';
import VirtalUl from '../utils/VirtualUl';
import i18n from '../i18n';
import styled from '@emotion/styled';
import { ColumnFilterIcon } from '../icon/Icon';

export const ButtonGroup = styled.div`
    display: flex;
    justify-content: center;
`;

function FilterDropdown({ setSelectedKeys, confirm, filters: originFilters }: FilterDropdownProps): JSX.Element {
    const [filters, setFilters] = useState<ColumnFilterItem[]>(originFilters ?? []);
    const [searchText, setSearchText] = useState('');
    const [selection, setSelection] = useState<React.Key[]>([]);

    const handleSelectedChange = (values: ValueType[]): void => {
        setSelection(values);
    };
    const reset = (): void => {
        const newFilters = [...filters];
        setFilters(newFilters);
        setSearchText('');
        setSelectedKeys([]);
        confirm();
    };

    useEffect(() => {
        setFilters(originFilters ?? []);
    }, [originFilters]);

    return (
        <div style={{ padding: 8, maxWidth: '400px', overflow: 'auto' }} onKeyDown={(e): void => e.stopPropagation()}>
            <div><Input allowClear maxLength={200} value={searchText} onChange={(e): void => setSearchText(e.target.value)} />
            </div>
            {filters.length > 0 && <VirtalUl items={filters as CheckItem[]} onChange={handleSelectedChange} searchText={searchText}/>}
            <ButtonGroup>
                <Button type="primary" size="small" style={{ marginRight: 8 }} onClick={(): void => {
                    setSelectedKeys(selection);
                    confirm();
                }}>{i18n.t('buttonText:Search')}</Button>
                <Button size="small" onClick={(): void => { reset(); }} >{i18n.t('buttonText:Reset')}</Button>
            </ButtonGroup>
        </div>
    );
}
export const getColumnSearchProps = <T extends Record<string, unknown>>(clearFilters?: () => void): ColumnType<T> => {
    return ({
        filterDropdown: (props: FilterDropdownProps): React.ReactNode => (
            <FilterDropdown {...props} clearFilters={clearFilters}/>),
        filterIcon: () => (
            <ColumnFilterIcon />
        ),
        onFilterDropdownOpenChange: (visible: boolean): void => {
            if (visible) {
                limitInput();
            }
        },
    });
};
