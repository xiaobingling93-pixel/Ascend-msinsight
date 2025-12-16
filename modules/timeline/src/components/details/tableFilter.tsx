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

import React from 'react';
import styled from '@emotion/styled';
import type { FilterConfirmProps } from '../base/rc-table/interface';
import { ReactComponent as CancelIcon } from '../../assets/images/insights/CancelIcon.svg';
import { Input } from '@insight/lib/components';
import { ColumnFilterIcon } from '@insight/lib/icon';
import i18n from '@insight/lib/i18n';

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
            placeholder={i18n.t('Filter by field content')}
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
