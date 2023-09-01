import React from 'react';
import { ReactComponent as FunnelIcon } from '../../assets/images/insights/FunnelIcon.svg';
import styled from '@emotion/styled';
import { FilterConfirmProps } from '../base/rc-table/interface';
import { ReactComponent as CancelIcon } from '../../assets/images/insights/CancelIcon.svg';
import Input from 'antd/lib/input';

const FilterContainer = styled.div`
    width: 230px;
    height: 50px;
    padding: 10px;
    border-radius: 16px;
    background-color: ${props => props.theme.selectBackgroundColor};
    color: ${props => props.theme.fontColor};
    align-items: center;
    justify-content: space-evenly;
    input {
        width: 180px;
        height: 30px;
        border: none;
        font-size: .875rem;
        border-radius: 6px;
        background-color: ${props => props.theme.searchBackgroundColor};
        caret-color: ${props => props.theme.searchInputCaretColor};
        color: ${props => props.theme.tableHeadFontColor};
    }
    input:focus {
        box-shadow: none;
    }
    .cancalIcon {
        cursor: pointer;
        margin-left: 11px;
        g, g path {
            fill: ${props => props.theme.cancelIconBackgroundColor};
        }
    }
}
`;

type FilterDropdownType = {
    setSelectedKeys: (selectedKeys: React.Key[]) => void;
    selectedKeys: React.Key[];
    confirm: (param?: FilterConfirmProps | undefined) => void;
    clearFilters: (() => void) | undefined;
};

// 过滤视图
const setFiltersContent = ({ setSelectedKeys, selectedKeys, confirm, clearFilters }: FilterDropdownType): JSX.Element => {
    return <FilterContainer>
        <Input
            placeholder="Filter by field content"
            value={selectedKeys[0]}
            onChange={e => setSelectedKeys(e.target.value ? [e.target.value] : [])}
            onPressEnter={() => confirm()}
        />
        <CancelIcon className="cancalIcon" onClick={() => { clearFilters?.(); confirm(); }} />
    </FilterContainer>;
};

const onFilterFunc = <T extends Record<string, unknown>>(dataIndex: keyof T | Array<keyof T>): typeof onFilter => {
    const onFilter = (value: string | number | boolean, record: T): boolean => {
        if (typeof dataIndex === 'string') {
            return filterIndex(String(record[dataIndex]), String(value));
        } else if (dataIndex instanceof Array) {
            const filteredDataIndex = dataIndex.filter(item => record[item] !== undefined);
            return filteredDataIndex.some(index => filterIndex(String(record[index]), String(value)));
        }
        return false;
    };
    return onFilter;
};

const filterIndex = (data: string, value: string): boolean => {
    return data.toLowerCase().includes(value.toLowerCase());
};

export const parseFilterDef = <T extends Record<string, unknown>>(dataIndex?: keyof T | Array<keyof T>): Record<string, unknown> => dataIndex === undefined
    ? {}
    : ({
        filterIcon: <FunnelIcon className="filterIcon" />,
        filterDropdown: setFiltersContent,
        onFilter: onFilterFunc(dataIndex),
    });
