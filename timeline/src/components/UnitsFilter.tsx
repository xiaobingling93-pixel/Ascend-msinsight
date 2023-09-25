import { useTheme } from '@emotion/react';
import styled from '@emotion/styled';
import { Tooltip } from 'antd';
import { computed, runInAction } from 'mobx';
import { observer } from 'mobx-react';
import React, { useEffect, useRef, useState } from 'react';
import { ReactComponent as AntdFilterIcon } from '../assets/images/insights/FunnelIcon.svg';
import { ReactComponent as AntdCloseIcon } from '../assets/images/insights/ic_close_filled.svg';
import { Session } from '../entity/session';
import { CustomButton } from './base/StyledButton';
import { StyledInput } from './base/StyledInput';
import { InsightUnit } from '../entity/insight';
import { SvgType } from './base/rc-table/types';
import { StyledSelect } from './base/StyledSelect';
import { CardMetaData, ProcessMetaData } from '../entity/data';
import { preOrderFlatten } from '../entity/common';
import { isPinned } from './ChartContainer/unitPin';
import { StyledAutoComplete } from './base/rc-table/StyledAutoComplete';
import i18n from 'i18next';

const FilterIcon = AntdFilterIcon as SvgType;
const CloseIcon = AntdCloseIcon as SvgType;

const ChildrenContainer = styled.div`
    color: ${props => props.theme.fontColor};
    text-align: left;
    padding-left: 20px;
    user-select: none;
`;

const CustomDiv = styled.div`
    display: flex;
    align-items: center;
    justify-content: space-between;
    border-radius: 18px;
    padding: 1px 7px 1px 10px;
    min-width: 600px;
    background: ${props => props.theme.tooltipBGColor};
    .chooseResult {
        display: flex;
        margin-bottom: 0;
        list-style: none;
        padding: 0;
        li {
            display: flex;
            align-items: center;
            height: 24px;
            padding-right: 8px;
            font-size: 14px;
            white-space: nowrap;
            color: ${props => props.theme.deviceProcessContentFontColor};
            background: ${props => props.theme.multiSelectBgColor};
            border-radius: 12px;
            margin: 0 5px;
            .icon {
                width: 12px;
                height: 12px;
                margin-left: 5px;
                fill: ${props => props.theme.buttonColor.enableClickColor};
            }
            & > div {
                padding-left: 8px;
            }
            .ant-select {
                color: ${props => props.theme.deviceProcessContentFontColor};
                .ant-select-selector {
                    height: 24px;
                    border: none;
                    padding: 0 5px;
                    background-color: transparent;
                    .ant-select-selection-search input {
                        height: 24px;
                    }
                    .ant-select-selection-item {
                        display: flex;
                        align-items: center;
                        height: 24px;
                        padding-right: 0;
                    }
                }
                // select禁用的时候颜色不变
                &.ant-select-disabled .ant-select-selector {
                    color: ${props => props.theme.deviceProcessContentFontColor};
                }
                // 去掉下拉三角按钮
                .ant-select-arrow {
                    display: none;
                }
                .ant-select-dropdown {
                    width: 200px!important;
                    border-radius: 16px;
                    left: -7px !important;
                    background: ${props => props.theme.deviceProcessBackgroundColor};
                    .rc-virtual-list-holder-inner {
                        & > div {
                            border-radius: 10px;
                            color: ${props => props.theme.filterColor};
                        }
                        .ant-select-item-option-active {
                            color: ${props => props.theme.filterColor} !important;
                            background-color: ${props => props.theme.filterSelectActiveBgColor} !important;
                        }
                    }
                }
            }
        }
    }
    .searchResult {
        color: ${(props) => props.theme.svgBackgroundColor};
        font-size: 12px;
        white-space: nowrap;
    }
`;

const useAutoCompleteHandles = (session: Session): [ selectValue: string, dropdownRenderr: () => JSX.Element, isOpen: boolean,
    setIsOpen: React.Dispatch<React.SetStateAction<boolean>>, completeOptions: Array<{value: string}>, handleSearch: Function,
    handleSearch: (value: string) => void, handleSelect: (value: string) => void ] => {
    const [ selectValue, setSelectValue ] = useState<string>('Filter');
    const [ isOpen, setIsOpen ] = useState<boolean>(false);
    const { cardNames, unitNames } = useUnitsNameSet(session);
    const [ completeOptions, setCompleteOptions ] = useState<Array<{value: string}>>([]);

    useEffect(() => {
        handleSearch('');
    }, [selectValue]);

    function dropdownRender(): JSX.Element {
        return (
            <ChildrenContainer>
                <div key={'file'} onClick={() => {
                    setSelectValue('Card Filter');
                    setIsOpen(false);
                }}>Card Filter</div>
                <div key={'folder'} onClick={() => {
                    setSelectValue('Units Filter');
                    setIsOpen(false);
                }}>Units Filter</div>
            </ChildrenContainer>
        );
    };

    const handleSearch = (value: string): void => {
        const result: Array<{value: string}> = [];
        let targetSet = new Set<string>();
        if (selectValue === 'Units Filter') {
            targetSet = unitNames;
        }
        if (selectValue === 'Card Filter') {
            targetSet = cardNames;
        }
        targetSet.forEach((cardName) => {
            if (cardName.toLowerCase().includes(value.toLowerCase())) {
                result.push({ value: cardName });
            }
        });
        setCompleteOptions(result);
    };

    const handleChange = (value: string): void => {
        if (value === '') {
            startFilter(session, value, selectValue);
        }
    };

    const handleSelect = (value: string): void => {
        startFilter(session, value, selectValue);
    };

    return [ selectValue, dropdownRender, isOpen, setIsOpen, completeOptions, handleSearch, handleChange, handleSelect ];
};

const CategorySearchContent = (session: Session): JSX.Element => {
    const theme = useTheme();

    const [ selectValue, dropdownRender, isOpen, setIsOpen, completeOptions, handleSearch, handleChange, handleSelect ] = useAutoCompleteHandles(session);
    return (
        <CustomDiv theme={theme}>
            <StyledSelect
                style={{ top: 2 }}
                value={selectValue}
                dropdownRender={dropdownRender}
                onDropdownVisibleChange={open => setIsOpen(open)}
                open={isOpen}
                height={24} width={120} itemPaddingLeft={20}>
            </StyledSelect>

            <StyledAutoComplete
                type={'text'} options={completeOptions} style={{ top: 2 }}
                onSearch={handleSearch} onChange={handleChange} onSelect={handleSelect} getPopupContainer={(triggerNode: { parentNode: any }) => triggerNode.parentNode}>
                <StyledInput allowClear={{ clearIcon: <CloseIcon fill={theme.buttonColor.enableClickColor}/> }}
                    minwidth={450} height={24} isshow={Number(selectValue !== 'Filter')} type={'text'}>
                </StyledInput>
            </StyledAutoComplete>
        </CustomDiv>
    );
};

const startFilter = (session: Session, inputValue: string, selectValue: string): void => {
    setAllUnitsDisplay(session);
    if (inputValue === '') {
        return;
    }
    const flattenUnits = computed(() => preOrderFlatten(session.units, 0,
        { when: unit => unit.isExpanded, bypass: unit => unit.type === 'transparent', exclude: unit => unit.pinType === 'move' && isPinned(unit) })).get();
    switch (selectValue) {
        case 'Card Filter':
            doCardFilter(flattenUnits, inputValue);
            break;
        case 'Units Filter':
            doUnitsFilter(flattenUnits, inputValue);
            break;
    }
};

const doCardFilter = (flattenUnits: InsightUnit[], inputValue: string): void => {
    let targetUnit: InsightUnit | undefined;
    flattenUnits.forEach(unit => {
        runInAction(() => {
            const isTargetUnit = (unit.metadata as CardMetaData).cardName === inputValue;
            unit.isDisplay = (unit.metadata as CardMetaData).cardName === inputValue;
            if (isTargetUnit) {
                targetUnit = unit;
            }
        });
    });
    if (targetUnit === undefined) {
        return;
    }
    targetUnit.children?.forEach(unit => {
        unit.isDisplay = true;
        unit.children?.forEach(processUnit => {
            processUnit.isDisplay = true;
        });
    });
};

const doUnitsFilter = (flattenUnits: InsightUnit[], inputValue: string): void => {
    flattenUnits.forEach(unit => {
        runInAction(() => {
            if (!unit.children) {
                unit.isDisplay = false;
                return;
            }
            let hasMatchUnit = false;
            unit.children.forEach(processUnit => {
                const isProcessUnitMatch = (processUnit.metadata as ProcessMetaData).processName === inputValue;
                hasMatchUnit = hasMatchUnit || isProcessUnitMatch;
                if (!isProcessUnitMatch) {
                    processUnit.isDisplay = false;
                } else {
                    processUnit.isDisplay = true;
                }
            });
            if (!hasMatchUnit) {
                unit.isDisplay = false;
            }
        });
    });
    const setUnitDislay = (unit: InsightUnit): void => {
        runInAction(() => {
            unit.isDisplay = true;
        });
        if (unit.children) {
            for (const child of unit.children) {
                setUnitDislay(child);
            }
        }
    };
    flattenUnits.forEach(unit => {
        if (!unit.children) {
            return;
        }
        if ((unit.metadata as ProcessMetaData).processName === inputValue) {
            setUnitDislay(unit);
        }
    });
};

const setAllUnitsDisplay = (session: Session): void => {
    const setUnitDislay = (unit: InsightUnit): void => {
        runInAction(() => {
            unit.isDisplay = true;
        });
        if (unit.children) {
            for (const child of unit.children) {
                setUnitDislay(child);
            }
        }
    };
    for (const unit of session.units) {
        setUnitDislay(unit);
    }
};

const useUnitsNameSet = (session: Session): { cardNames: Set<string>; unitNames: Set<string> } => {
    const cardNames = new Set<string>();
    const unitNames = new Set<string>();

    const visitUnit = (unit: InsightUnit): void => {
        if (unit.name === 'Card') {
            cardNames.add((unit.metadata as CardMetaData).cardName ?? '');
        } else if (unit.name === 'Process') {
            unitNames.add((unit.metadata as ProcessMetaData).processName);
        }
        if (unit.children) {
            for (const child of unit.children) {
                visitUnit(child);
            }
        }
    };

    session.units.forEach(unit => {
        visitUnit(unit);
    });
    return { cardNames, unitNames };
};

export const UnitsFilter = observer(({ session }: { session: Session}): JSX.Element | null => {
    const theme = useTheme();
    const [ customButtonProps, updateCustomButtonProps ] = useState({
        isEmphasize: false,
        isSuspend: false,
        icon: FilterIcon,
    });
    // tooltip显隐控制悬浮效果
    const onTooltipVisibleChange = (visible: boolean): void => {
        updateCustomButtonProps({ ...customButtonProps, isSuspend: visible });
    };
    const ref = useRef<HTMLButtonElement>(null);
    return (
        <Tooltip overlayStyle={{ maxWidth: 1000 }}
            title={CategorySearchContent(session)}
            trigger="click"
            placement="right"
            onVisibleChange={onTooltipVisibleChange}
            color={theme.tooltipBGColor}
            overlayInnerStyle={{ color: theme.tooltipFontColor, padding: 0, borderRadius: 20 }}
            overlayClassName={'insight-category-search-overlay'} align={{ offset: [ -8, 3 ] }}>
            <CustomButton tooltip={i18n.t('tooltip:filter')} { ...customButtonProps } ref={ref}/>
        </Tooltip>
    );
});
