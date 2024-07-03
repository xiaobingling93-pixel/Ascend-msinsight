import { useTheme } from '@emotion/react';
import styled from '@emotion/styled';
import { Tooltip, Select } from 'antd';
import { runInAction } from 'mobx';
import { observer } from 'mobx-react';
import React, { useEffect, useRef, useState } from 'react';
import { ReactComponent as AntdFilterIcon } from '../assets/images/insights/FunnelIcon.svg';
import { Session } from '../entity/session';
import { CustomButton } from './base/StyledButton';
import { InsightUnit } from '../entity/insight';
import { SvgType } from './base/rc-table/types';
import { StyledSelect } from './base/StyledSelect';
import { CardMetaData, ProcessMetaData } from '../entity/data';
import { useTranslation } from 'react-i18next';

const FilterIcon = AntdFilterIcon as SvgType;

const ChildrenContainer = styled.div`
    color: ${(props): string => props.theme.fontColor};
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
    background: ${(props): string => props.theme.tooltipBGColor};
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
            color: ${(props): string => props.theme.deviceProcessContentFontColor};
            background: ${(props): string => props.theme.multiSelectBgColor};
            border-radius: 12px;
            margin: 0 5px;
            .icon {
                width: 12px;
                height: 12px;
                margin-left: 5px;
                fill: ${(props): string => props.theme.buttonColor.enableClickColor};
            }
            & > div {
                padding-left: 8px;
            }
            .ant-select {
                color: ${(props): string => props.theme.deviceProcessContentFontColor};
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
                    color: ${(props): string => props.theme.deviceProcessContentFontColor};
                }
                // 去掉下拉三角按钮
                .ant-select-arrow {
                    display: none;
                }
                .ant-select-dropdown {
                    width: 200px!important;
                    border-radius: 16px;
                    left: -7px !important;
                    background: ${(props): string => props.theme.deviceProcessBackgroundColor};
                    .rc-virtual-list-holder-inner {
                        & > div {
                            border-radius: 10px;
                            color: ${(props): string => props.theme.filterColor};
                        }
                        .ant-select-item-option-active {
                            color: ${(props): string => props.theme.filterColor} !important;
                            background-color: ${(props): string => props.theme.filterSelectActiveBgColor} !important;
                        }
                    }
                }
            }
        }
    }
    .searchResult {
        color: ${(props): string => props.theme.svgBackgroundColor};
        font-size: 12px;
        white-space: nowrap;
    }
`;

const useAutoCompleteHandles = (session: Session): [ selectValue: string, dropdownRenderr: () => JSX.Element, isOpen: boolean,
    setIsOpen: React.Dispatch<React.SetStateAction<boolean>>, completeOptions: Array<{value: string}>, handleChange: Function ] => {
    const [selectValue, setSelectValue] = useState<string>('Filter');
    const [isOpen, setIsOpen] = useState<boolean>(false);
    const { cardNames, unitNames } = useUnitsNameSet(session);
    const [completeOptions, setCompleteOptions] = useState<Array<{value: string}>>([]);

    useEffect(() => {
        handleSearch('');
    }, [selectValue]);

    function dropdownRender(): JSX.Element {
        return (
            <ChildrenContainer>
                <div key={'file'} onClick={(): void => {
                    setSelectValue('Card Filter');
                    setIsOpen(false);
                }}>Card Filter</div>
                <div key={'folder'} onClick={(): void => {
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

    const handleChange = (value: string[]): void => {
        startFilter(session, value, selectValue);
    };
    return [selectValue, dropdownRender, isOpen, setIsOpen, completeOptions, handleChange];
};

const CategorySearchContent = (session: Session): JSX.Element => {
    const theme = useTheme();
    const [selectValue, dropdownRender, isOpen, setIsOpen, completeOptions, handleChange] = useAutoCompleteHandles(session);
    const [selection, setSelection] = useState<string[]>([]);
    useEffect(() => {
        setSelection([]);
        handleChange([]);
    }, [completeOptions]);

    return (
        <CustomDiv theme={theme}>
            <StyledSelect
                style={{ top: 2 }}
                value={selectValue}
                dropdownRender={dropdownRender}
                onDropdownVisibleChange={(open): void => setIsOpen(open)}
                open={isOpen}
                height={24} width={120} itemPaddingLeft={20}>
            </StyledSelect>
            <Select
                size="small"
                mode="multiple"
                allowClear
                className="circle-border"
                options={completeOptions}
                style={{ top: '2px', width: '455px', height: '24px' }}
                value={selection}
                onChange={(val: string[]): void => { setSelection(val); handleChange(val); }}
            >
            </Select>
        </CustomDiv>
    );
};

const startFilter = (session: Session, inputValue: string[], selectValue: string): void => {
    setAllUnitsDisplay(session);
    if (inputValue.length === 0) {
        return;
    }
    switch (selectValue) {
        case 'Card Filter':
            doCardFilter(session.units, inputValue);
            break;
        case 'Units Filter':
            doUnitsFilter(session.units, inputValue);
            break;
    }
    session.singleLinkLine = {};
};

const doCardFilter = (flattenUnits: InsightUnit[], selectValues: string[]): void => {
    runInAction(() => {
        flattenUnits.forEach(unit => {
            const isTargetUnit = selectValues.includes((unit.metadata as CardMetaData).cardName as string);
            unit.isDisplay = isTargetUnit;
            if (isTargetUnit) {
                unit.children?.forEach(unit => {
                    unit.isDisplay = true;
                    unit.children?.forEach(processUnit => {
                        processUnit.isDisplay = true;
                    });
                });
            }
        });
    });
};

const doUnitsFilter = (flattenUnits: InsightUnit[], selectValues: string[]): void => {
    runInAction(() => {
        flattenUnits.forEach(unit => {
            if (!unit.children) {
                unit.isDisplay = false;
                return;
            }
            let hasMatchUnit = false;
            unit.children.forEach(processUnit => {
                const isProcessUnitMatch = selectValues.includes((processUnit.metadata as ProcessMetaData).processName.replace(/\(\d{0,12}\)/, ''));
                processUnit.isDisplay = isProcessUnitMatch || findMatchUnit(processUnit, selectValues);
                hasMatchUnit = hasMatchUnit || processUnit.isDisplay;
            });
            if (!hasMatchUnit) {
                unit.isDisplay = false;
            }
        });

        const setUnitDislay = (unit: InsightUnit): void => {
            unit.isDisplay = true;
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
            if (selectValues.includes(((unit.metadata as ProcessMetaData).processName))) {
                setUnitDislay(unit);
            }
        });
    });
};

const findMatchUnit = (unit: InsightUnit, selectValues: string[]): boolean => {
    let hasMatchUnit = false;
    unit.children?.forEach(processUnit => {
        const isProcessUnitMatch = selectValues.includes((processUnit.metadata as ProcessMetaData).processName);
        processUnit.isDisplay = isProcessUnitMatch || findMatchUnit(processUnit, selectValues);
        hasMatchUnit = hasMatchUnit || processUnit.isDisplay;
    });
    return hasMatchUnit;
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
        }
        if (unit.name === 'Process' || unit.name === 'Label') {
            unitNames.add((unit.metadata as ProcessMetaData).processName.replace(/\(\d{0,12}\)/, ''));
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
    const [customButtonProps, updateCustomButtonProps] = useState({
        isEmphasize: false,
        isSuspend: false,
        icon: FilterIcon,
    });
    // tooltip显隐控制悬浮效果
    const onTooltipVisibleChange = (visible: boolean): void => {
        updateCustomButtonProps({ ...customButtonProps, isSuspend: visible });

        setTimeout(() => {
            const inputs = document.querySelectorAll('input');
            inputs.forEach(input => {
                input.setAttribute('maxlength', '200');
            });
        });
    };
    const ref = useRef<HTMLButtonElement>(null);
    const { t } = useTranslation();
    return (
        <Tooltip overlayStyle={{ maxWidth: 1000 }}
            title={CategorySearchContent(session)}
            trigger="click"
            placement="right"
            onOpenChange={onTooltipVisibleChange}
            color={theme.tooltipBGColor}
            overlayInnerStyle={{ color: theme.tooltipFontColor, padding: 0, borderRadius: 20 }}
            overlayClassName={'insight-category-search-overlay'} align={{ offset: [-8, 3] }}>
            <CustomButton tooltip={t('tooltip:filter')} { ...customButtonProps } ref={ref}/>
        </Tooltip>
    );
});
