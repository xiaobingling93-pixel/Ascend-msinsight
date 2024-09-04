/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { useTheme } from '@emotion/react';
import styled from '@emotion/styled';
import { Tooltip, Select } from 'ascend-components';
import { runInAction } from 'mobx';
import { observer } from 'mobx-react';
import React, { useEffect, useRef, useState } from 'react';
import { FilterIcon } from 'ascend-icon';
import type { Session } from '../entity/session';
import { CustomButton } from './base/StyledButton';
import type { InsightUnit } from '../entity/insight';
import type { CardMetaData, ProcessMetaData } from '../entity/data';
import { useTranslation } from 'react-i18next';
import i18n from 'ascend-i18n';

const ChildrenContainer = styled.div`
    color: ${(props): string => props.theme.textColorPrimary};
    font-size: 12px;
    user-select: none;

    > div {
      padding: 5px 11px;
      cursor: pointer;
      &:hover{
        background: ${(props): string => props.theme.primaryColor};
      }
    }
`;

const CustomDiv = styled.div`
    display: flex;
    align-items: center;
    border-radius: 2px;
    padding: 0 8px;
    background: ${(props): string => props.theme.bgColorLight};
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

interface CompleteOptionProps {
    label: string;
    value: string;
}

type UseAutoCompleteHandlesReturnType = [
    selectValue: string | null,
    dropdownRenderr: () => JSX.Element,
    isOpen: boolean,
    setIsOpen: React.Dispatch<React.SetStateAction<boolean>>,
    completeOptions: CompleteOptionProps[],
    handleChange: (value: string[]) => void,
];
const useAutoCompleteHandles = (session: Session): UseAutoCompleteHandlesReturnType => {
    const [selectValue, setSelectValue] = useState<string | null>(null);
    const [isOpen, setIsOpen] = useState<boolean>(false);
    const { cardNames, unitNames } = useUnitsNameSet(session);
    const [completeOptions, setCompleteOptions] = useState<CompleteOptionProps[]>([]);

    useEffect(() => {
        handleSearch('');
    }, [selectValue]);

    function dropdownRender(): JSX.Element {
        return (
            <ChildrenContainer>
                <div key={'file'} onClick={(): void => {
                    setSelectValue('Card Filter');
                    setIsOpen(false);
                }}>{i18n.t('Card Filter', { ns: 'timeline' })}</div>
                <div key={'folder'} onClick={(): void => {
                    setSelectValue('Units Filter');
                    setIsOpen(false);
                }}>{i18n.t('Units Filter', { ns: 'timeline' })}</div>
            </ChildrenContainer>
        );
    };

    const handleSearch = (value: string): void => {
        const result: CompleteOptionProps[] = [];
        let targetSet = new Set<string>();
        if (selectValue === 'Units Filter') {
            targetSet = unitNames;
        }
        if (selectValue === 'Card Filter') {
            targetSet = cardNames;
        }
        targetSet.forEach((cardName) => {
            if (cardName.toLowerCase().includes(value.toLowerCase())) {
                result.push({ label: cardName, value: cardName });
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
    }, [completeOptions, session.doReset]);

    return (
        <CustomDiv theme={theme}>
            <Select
                value={selectValue === null ? null : i18n.t(selectValue, { ns: 'timeline' })}
                dropdownRender={dropdownRender}
                onDropdownVisibleChange={(open: boolean): void => setIsOpen(open)}
                open={isOpen}
                placeholder={i18n.t('Select filter type', { ns: 'timeline' })}
                height={32} width={120}>
            </Select>
            <Select
                mode="multiple"
                allowClear
                options={completeOptions}
                width={280}
                height={32}
                value={selection}
                onChange={(val: string[]): void => { setSelection(val); handleChange(val); }}
            >
            </Select>
        </CustomDiv>
    );
};

const startFilter = (session: Session, inputValue: string[], selectValue: string | null): void => {
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
        default:
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
                unit.children?.forEach(unitChild => {
                    unitChild.isDisplay = true;
                    unitChild.children?.forEach(processUnit => {
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
    const [customButtonProps, updateCustomButtonProps] = useState({
        isEmphasize: false,
        isSuspend: false,
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
            overlayInnerStyle={{ borderRadius: 2 }}
            title={CategorySearchContent(session)}
            trigger="click"
            onOpenChange={onTooltipVisibleChange}
            overlayClassName="insight-category-search-overlay"
            align={{ offset: [-8, 3] }}
            zIndex={1040}
        >
            <CustomButton tooltip={t('tooltip:filter')} icon={FilterIcon as any} { ...customButtonProps } ref={ref}/>
        </Tooltip>
    );
});
