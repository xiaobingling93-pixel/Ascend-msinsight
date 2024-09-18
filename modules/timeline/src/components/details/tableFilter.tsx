/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React from 'react';
import styled from '@emotion/styled';
import type { FilterConfirmProps } from '../base/rc-table/interface';
import { ReactComponent as CancelIcon } from '../../assets/images/insights/CancelIcon.svg';
import { Input } from 'ascend-components';
import { ColumnFilterIcon } from 'ascend-icon';
import i18n from 'ascend-i18n';

const FilterContainer = styled.div`
    width: 230px;
    height: 50px;
    padding: 10px;
    border-radius: 2px;
    background-color: ${(props): string => props.theme.bgColorLight};
    color: ${(props): string => props.theme.fontColor};
    align-items: center;
    justify-content: space-evenly;
    input {
        width: 180px;
        height: 30px;
        border: none;
        font-size: .875rem;
        border-radius: 6px;
        caret-color: ${(props): string => props.theme.searchInputCaretColor};
    }
    input:focus {
        box-shadow: none;
    }
    .cancalIcon {
        cursor: pointer;
        margin-left: 11px;
        g, g path {
            fill: ${(props): string => props.theme.cancelIconBackgroundColor};
        }
    }
}
`;

interface FilterDropdownType {
    setSelectedKeys: (selectedKeys: React.Key[]) => void;
    selectedKeys: React.Key[];
    confirm: (param?: FilterConfirmProps | undefined) => void;
    clearFilters: (() => void);
};

// 过滤视图
const setFiltersContent = ({ setSelectedKeys, selectedKeys, confirm, clearFilters }: FilterDropdownType): JSX.Element => {
    return <FilterContainer>
        <Input
            placeholder={`${i18n.t('Filter by field content')}`}
            value={selectedKeys[0]}
            onChange={(e): void => setSelectedKeys(e.target.value ? [e.target.value] : [])}
            onPressEnter={(): void => confirm()}
        />
        <CancelIcon className="cancalIcon" onClick={(): void => { clearFilters?.(); confirm(); }} />
    </FilterContainer>;
};

const onFilterFunc = <T extends Record<string, unknown>>(dataIndex: keyof T | Array<keyof T>): typeof onFilter => {
    const onFilter = (value: string | number | boolean, record: T): boolean => {
        if (typeof dataIndex === 'string') {
            return filterIndex(String(record[dataIndex]), String(value));
        } else if (dataIndex instanceof Array) {
            const filteredDataIndex = dataIndex.filter(item => record[item] !== undefined);
            return filteredDataIndex.some(index => filterIndex(String(record[index]), String(value)));
        } else {
            return false;
        }
    };
    return onFilter;
};

const filterIndex = (data: string, value: string): boolean => {
    return data.toLowerCase().includes(value.toLowerCase());
};

export const parseFilterDef = <T extends Record<string, unknown>>(dataIndex?: keyof T | Array<keyof T>): Record<string, unknown> => dataIndex === undefined
    ? {}
    : ({
        filterIcon: <ColumnFilterIcon/>,
        filterDropdown: setFiltersContent,
        onFilter: onFilterFunc(dataIndex),
    });
