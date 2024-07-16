/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { useTheme } from '@emotion/react';
import styled from '@emotion/styled';
import { Checkbox, Input, Row, Select, Switch } from 'antd';
import { runInAction } from 'mobx';
import { observer } from 'mobx-react';
import React, { useState } from 'react';
import { ReactComponent as PullDownIcon } from '../assets/images/insights/PullDownIcon.svg';
import { ReactComponent as SearchIcon } from '../assets/images/insights/SearchIcon.svg';
import { ReactComponent as CancelIcon } from '../assets/images/insights/CancelIcon.svg';
import type { Theme } from '@emotion/react';
import { FilterType, type OptionType, type TabState } from '../entity/tabDependency';
import { Abbreviature } from './details/base/Abbreviature';

interface SingleSelectProps {
    theme: Theme;
    tabState: TabState;
    optionVal: unknown;
    options: OptionType[];
    style: React.CSSProperties;
    origin: Origin;
};

interface MultipleSelectProps {
    theme: Theme;
    tabState: TabState;
    dftVal: string[];
    options: OptionType[];
    style: React.CSSProperties;
    mode?: 'multiple';
};

enum Origin {
    'OPTION1' = 0,
    'OPTION2' = 1,
}

export const FILTER_HEIGHT = 31;

const Container = styled.div`
    display: flex;
    align-items: center;
    width: 100%;
    height: ${FILTER_HEIGHT}px;
    background-color: ${(props): string => props.theme.contentBackgroundColor};
    border-top: 1px solid ${(p): string => p.theme.dividerColor};

    .ant-select {
        height: 24px;
        margin-left: 8px;
        border-radius: 8px;
        border: 1px solid ${(props): string => props.theme.solidLine};
        color: ${(props): string => props.theme.deviceProcessContentFontColor};
        text-align: left;
        font-size: 1rem;
        .ant-select-arrow {
            color: white;
            svg {
                width: 8px;
            }
        }
        .ant-select-selector {
            display: flex;
            align-items: center;
            height: 24px;
            padding: 0 6px;
            border: none;
            background-color: ${(props): string => props.theme.deviceProcessBackgroundColor};
            .ant-select-selection-overflow {
                margin-top: -6px;
            }
            .ant-select-selection-placeholder {
                padding-left: 3px;
                color: ${(props): string => props.theme.filterColor};
            }
            .ant-select-selection-overflow-item .ant-select-selection-item {
                max-width: 80px;
                border-radius: 8px;
                border: 1px solid ${(props): string => props.theme.solidLine};
                background: transparent;
                .ant-select-selection-item-content {
                    color: ${(props): string => props.theme.filterColor};
                }
                .ant-select-selection-item-remove svg {
                    fill: ${(props): string => props.theme.devicePullDown};
                }
            }
            .ant-select-selection-search .ant-select-selection-search-input {
                height: 0px;
            }
            .ant-select-selection-item {
                color: ${(props): string => props.theme.filterColor};
            }
        }
        .ant-select-selection-item {
            padding-left: 5px;
        }
        .ant-select-item {
            padding-left: 7px!important;
        }
        .ant-select-dropdown {
            border-radius: 16px;
            padding: 4px;
            .rc-virtual-list-holder-inner {
                & > div {
                    border-radius: 10px;
                    font-size: 1rem;
                    background: ${(props): string => props.theme.deviceProcessBackgroundColor};
                    color: ${(props): string => props.theme.filterColor};
                }
                .ant-select-item-option-active {
                    color: ${(props): string => props.theme.filterColor} !important;
                    background-color: ${(props): string => props.theme.filterSelectActiveBgColor} !important;
                    svg {
                        g {
                            fill: ${(props): string => props.theme.deviceProcessActiveFontColor};
                        }
                    }
                }
                .ant-select-item-option-selected .ant-select-item-option-state {
                    display: none;
                }
            }
        }
        .ant-select-item-option-content {
            font-weight: 500;
        }
        .rc-virtual-list-scrollbar-thumb {
            background: rgba(95, 95, 95, 0.9) !important;
        }

    }

    /* 多选列表 */
    .multiSelect {
        .ant-select-selection-item {
            display: flex;
            max-width: 105px!important;
            height: 20px;
            align-items: center;
            border: none!important;
            background: ${(props): string => props.theme.multiSelectBgColor} !important;
            .ant-checkbox {
                display: none;
                & + span {
                    padding: 0;
                }
            }
        }
        .multiSelectRow {
            .ant-checkbox{
                &.ant-checkbox-checked .ant-checkbox-inner {
                    background-color: #317AF7;
                    border: 1px solid #317AF7;
                    border-radius: 4px;
                }
                .ant-checkbox-inner {
                    border-radius: 4px;
                    background-color: ${(props): string => props.theme.multiSelectUnCheckedBgColor};
                    border: 1px solid ${(props): string => props.theme.multiSelectUnCheckedBorderColor};
                }
                &.ant-checkbox-checked::after {
                    border-radius: 4px;
                }
            }
            div {
                color: ${(props): string => props.theme.filterColor};
            }
        }
    }

    .pullDownIcon g use {
        fill: ${(props): string => props.theme.devicePullDown};
    }

    /* 搜索框 */
    .searchContainer {
        display: flex;
        height: 24px;
        align-items: center;
        background-color: ${(props): string => props.theme.searchBackgroundColor};
        border-radius: 20px;
        margin-left: 8px;
        padding: 0 10px;
        input {
            width: 245px;
            background-color: transparent;
            border: none;
            font-size: 1rem;
            caret-color: ${(props): string => props.theme.searchInputCaretColor};
            color: ${(props): string => props.theme.filterColor};
            &.ant-input:focus, &.ant-input:hover {
                border-color: ${(props): string => props.theme.solidLine};
                box-shadow: none;
            }
            &::placeholder {
                color: ${(props): string => props.theme.filterTipColor};
            }
        }
        .searchIcon {
            g use {
                fill: ${(props): string => props.theme.searchIconBackgroundColor};
            }
        }
        .cancalIcon {
            cursor: pointer;
            g, g path {
                fill: ${(props): string => props.theme.cancelIconBackgroundColor};
            }
        }
    }

    .switch {
        display: flex;
        height: 100%;
        align-items: center;
        // 开关关闭
        .ant-switch {
            background-color: ${(props): string => props.theme.switchClose};
        }
        // 开关开启
        .ant-switch-checked {
            background-color: ${(props): string => props.theme.switchOpen};
        }
        .ant-switch-checked:focus {
            box-shadow: none;
        }
        button {
            margin-left: 8px;
        }
        span {
            margin-left: 8px;
            font-size: 1rem;
        }
    }
`;

// 单选，选择了对应value
const selectVal = (value: any, tabState: TabState, origin: Origin): void => {
    runInAction(() => {
        if (tabState === undefined) {
            return;
        }
        // 改变选项
        if (tabState.trigger !== undefined && (origin === Origin.OPTION1 || origin === Origin.OPTION2)) {
            const key = origin === Origin.OPTION1 ? 'option1' : 'option2';
            tabState.trigger[key] = value;
        }
    });
};

// 多选,选择了某个item
const handleChange = (value: unknown, tabState: TabState, setDirty: React.Dispatch<React.SetStateAction<boolean>>): void => {
    runInAction(() => {
        if (tabState.filter === undefined || tabState.filter.filterKeys === undefined) {
            return;
        }
        setDirty(true);
        tabState.filter.filterKeys.length = 0;
        if (String(value).length > 0) {
            const valArr = String(value).split(',');
            tabState.filter.filterKeys.push(...valArr);
        }
    });
};

// 单选列表
const SingleSelect = ({ theme, tabState, optionVal, options, style, origin }: SingleSelectProps): JSX.Element => {
    return <Select
        value={optionVal}
        placement={'topLeft'}
        suffixIcon={<PullDownIcon className="pullDownIcon"/>}
        dropdownStyle={{ background: theme.deviceProcessBackgroundColor }}
        bordered={false}
        onSelect={(value: any): void => selectVal(value, tabState, origin)}
        getPopupContainer={(trigger: HTMLElement): ParentNode | null => trigger.parentNode}
        options={options}
        style={style}>
    </Select>;
};

// 多选列表
const MultiSelect = ({ theme, tabState, dftVal, options, style, mode }: MultipleSelectProps): JSX.Element => {
    const [dirty, setDirty] = React.useState(false);
    return <Select
        className="multiSelect"
        defaultValue={dftVal}
        placement={'topLeft'}
        suffixIcon={<PullDownIcon className="pullDownIcon"/>}
        dropdownStyle={{ background: theme.deviceProcessBackgroundColor }}
        bordered={false}
        getPopupContainer={(trigger: HTMLElement): ParentNode | null => trigger.parentNode}
        style={style}
        mode={mode}
        onChange={(value: unknown): void => handleChange(value, tabState, setDirty)}
        onMouseDown={(e: React.MouseEvent): void => { setDirty(false); e.stopPropagation(); }}
        maxTagCount="responsive"
        placeholder="Click to choose">
        { options.map(item =>
            <Select.Option key={item.value}>
                <Row className={'multiSelectRow'}>
                    <Checkbox value={item.value} onClick={(e: React.MouseEvent): void => { if (dirty) { e.stopPropagation(); } setDirty(false); }}
                        checked={tabState?.filter?.filterKeys.includes(String(item.value))}>
                        <div style={{ width: '135px' }}><Abbreviature content={ item.label } placement={'right'} availableWidth={135} /></div>
                    </Checkbox>
                </Row>
            </Select.Option>,
        )}
    </Select>;
};

// 多选,生成选项及过滤条件
const checkMutilSelect = (tabState: TabState): any[] => {
    const temp: any[] = [];
    const field = tabState?.filter?.field;
    const data = tabState?.data;
    if (tabState.filter !== undefined && data?.length > 1 && field !== undefined) {
        data.forEach((item: any) => {
            if (!temp.includes(item[field])) {
                temp.push(item[field]);
            }
        });
        const multiOptions = temp.map((item: string) => ({ value: item, label: item }));
        const filterKeys = tabState.filter?.filterKeys ?? [];
        tabState.filter.options = multiOptions;
        return [multiOptions, filterKeys];
    }
    return [];
};

// 搜索框按回车启动搜索
const handleSearch = (tabState: TabState, e?: React.KeyboardEvent<HTMLInputElement> | React.MouseEvent<SVGSVGElement, MouseEvent>): void => {
    runInAction(() => {
        if (tabState?.search?.content !== undefined) {
            tabState.search.content = e !== undefined ? (e.target as HTMLInputElement).value : '';
        }
    });
};

const switchChange = (checked: boolean, tabState: TabState): void => {
    runInAction(() => {
        if (tabState?.switch?.checked !== undefined) {
            tabState.switch.checked = checked;
        }
    });
};

const SearchContainer = observer(({ initialContent, fieldName, tabState }: { initialContent: string; fieldName: string; tabState: TabState }) => {
    const [content, setContent] = useState(initialContent);

    const handleInput = (e: React.ChangeEvent<HTMLInputElement>): void => { setContent(e.target.value); };

    return <div className="searchContainer">
        <SearchIcon className="searchIcon"/>
        <Input
            placeholder={`Involves ${fieldName}`}
            onPressEnter={(e: React.KeyboardEvent<HTMLInputElement>): void => handleSearch(tabState, e)}
            key={fieldName}
            value={content}
            onChange={handleInput}
        />
        <CancelIcon className="cancalIcon" onClick={(): void => { setContent(''); handleSearch(tabState); }} />
    </div>;
});

export const FilterContainer = observer(({ tabState }: {tabState: TabState}) => {
    const theme = useTheme();
    if (tabState === undefined) {
        return null;
    }
    // 单选
    const { option1, option2 } = tabState?.trigger ?? {};
    const { options1, options2 } = tabState;
    // 多选，通过选项过滤
    const [multiOptions, filterKeys] = tabState?.filter?.type === FilterType.MULTI_FILTER ? checkMutilSelect(tabState) : [];
    // 搜索内容
    const [fieldName, content] = tabState?.search !== undefined ? [tabState?.search?.fieldName, tabState?.search?.content] : [];
    // switch开关
    const { checked, tips, changeCallBack } = tabState?.switch ?? {};

    return <Container>
        {
            options1 !== undefined && option1 !== undefined &&
            <SingleSelect theme={theme} tabState={tabState} optionVal={option1} options={options1} style={{ width: '100px' }} origin={Origin.OPTION1} />
        }
        {
            options2 !== undefined && option2 !== undefined &&
            <SingleSelect theme={theme} tabState={tabState} optionVal={option2} options={options2} style={{ width: '135px' }} origin={Origin.OPTION2} />
        }
        {
            multiOptions !== undefined &&
            <MultiSelect theme={theme} tabState={tabState} dftVal={filterKeys} options={multiOptions} style={{ width: '180px' }} mode="multiple" />
        }
        {
            fieldName !== undefined && content !== undefined &&
            <SearchContainer initialContent={content} fieldName={fieldName} tabState={tabState} />
        }
        {
            checked !== undefined && tips !== undefined &&
            <div className="switch">
                <span>{tips}</span>
                <Switch defaultChecked={checked} onChange={(newCheckedState: boolean): void => { switchChange(newCheckedState, tabState); changeCallBack?.(newCheckedState); }} size="small"/>
            </div>
        }
    </Container>;
});
