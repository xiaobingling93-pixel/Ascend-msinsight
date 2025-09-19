/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { useTheme } from '@emotion/react';
import styled from '@emotion/styled';
import { Tooltip, Select, Tabs } from 'ascend-components';
import { runInAction } from 'mobx';
import { observer } from 'mobx-react';
import React, { useEffect, useRef, useState, useMemo } from 'react';
import { FilterIcon, HelpIcon } from 'ascend-icon';
import type { Session } from '../entity/session';
import { CustomButton } from './base/StyledButton';
import type { InsightUnit } from '../entity/insight';
import type { CardMetaData, ProcessMetaData, ThreadMetaData } from '../entity/data';
import { useTranslation } from 'react-i18next';

const DEFAULT_FILTER_KEY = 'Card';

const CustomDiv = styled.div`
    display: flex;
    align-items: center;
    border-radius: 2px;
    padding: 0 8px;
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

const TabsWrapper = styled.div`
    position: relative;
`;

const FixedTooltipWrapper = styled.div`
    position: absolute;
    top: 6px;
    right: 40px;
`;

interface CompleteOptionProps {
    label: string;
    value: string;
}

const startFilter = (session: Session, cardSelection: string[], unitSelection: string[]): void => {
    setAllUnitsDisplay(session);
    if (cardSelection.length === 0 && unitSelection.length === 0) {
        return;
    }

    if (cardSelection.length !== 0) {
        doCardFilter(session.units, cardSelection);
    }
    if (unitSelection.length !== 0) {
        doUnitsFilter(session.units, unitSelection);
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
            unit.children.forEach(childUnit => {
                let isUnitMatch = false;
                if (childUnit.name === 'Thread') {
                    isUnitMatch = selectValues.includes((childUnit.metadata as ThreadMetaData).threadName.replace(/\(\d{0,12}\)/, '').trim());
                } else {
                    isUnitMatch = selectValues.includes((childUnit.metadata as ProcessMetaData).processName.replace(/\(\d{0,12}\)/, '').trim());
                }
                childUnit.isDisplay = isUnitMatch || findMatchUnit(childUnit, selectValues);
                hasMatchUnit = hasMatchUnit || childUnit.isDisplay;
            });
            if (!hasMatchUnit) {
                unit.isDisplay = false;
            }
        });

        const setUnitDisplay = (unit: InsightUnit, loopIndex = 0): void => {
            const MaxLoop = 100;
            if (loopIndex > MaxLoop) {
                return;
            }
            unit.isDisplay = true;
            if (unit.children) {
                for (const child of unit.children) {
                    setUnitDisplay(child, loopIndex++);
                }
            }
        };
        flattenUnits.forEach(unit => {
            if (!unit.children) {
                return;
            }
            if (unit.name === 'Thread') {
                if (selectValues.includes(((unit.metadata as ThreadMetaData).threadName))) {
                    setUnitDisplay(unit);
                }
            } else {
                if (selectValues.includes(((unit.metadata as ProcessMetaData).processName))) {
                    setUnitDisplay(unit);
                }
            }
        });
    });
};

const findMatchUnit = (unit: InsightUnit, selectValues: string[]): boolean => {
    let hasMatchUnit = false;
    unit.children?.forEach(childUnit => {
        let isUnitMatch = false;
        if (childUnit.name === 'Thread') {
            isUnitMatch = selectValues.includes((childUnit.metadata as ThreadMetaData).threadName);
        } else {
            isUnitMatch = selectValues.includes((childUnit.metadata as ProcessMetaData).processName);
        }
        childUnit.isDisplay = isUnitMatch || findMatchUnit(childUnit, selectValues);
        hasMatchUnit = hasMatchUnit || childUnit.isDisplay;
    });
    return hasMatchUnit;
};

const setAllUnitsDisplay = (session: Session): void => {
    const setUnitDisplay = (unit: InsightUnit, loopIndex = 0): void => {
        const MaxLoop = 100;
        if (loopIndex > MaxLoop) {
            return;
        }
        runInAction(() => {
            unit.isDisplay = true;
        });
        if (unit.children) {
            for (const child of unit.children) {
                setUnitDisplay(child, loopIndex++);
            }
        }
    };
    for (const unit of session.units) {
        setUnitDisplay(unit);
    }
};

const useUnitsNameSet = (session: Session): { cardNames: Set<string>; unitNames: Set<string> } => {
    const cardNames = new Set<string>();
    const unitNames = new Set<string>();

    const visitUnit = (unit: InsightUnit, loopIndex = 0): void => {
        const MaxLoop = 100;
        if (loopIndex > MaxLoop) {
            return;
        }
        if (unit.name === 'Card' && !unit.isMultiDeviceHidden) {
            cardNames.add((unit.metadata as CardMetaData).cardName ?? '');
        }
        if (unit.name === 'Process' || unit.name === 'Label') {
            const metaDataName = (unit.metadata as ProcessMetaData).processName;
            if (!metaDataName.toLowerCase().includes('process') && !metaDataName.toLowerCase().includes('thread')) {
                unitNames.add(metaDataName.replace(/\(\d{0,12}\)/, '').trim());
            }
        }
        if (unit.name === 'Thread') {
            const metaDataName = (unit.metadata as ThreadMetaData).threadName;
            if (metaDataName && metaDataName !== '' && ['pytorch', 'mstx', 'python gc', 'cann', 'os runtime api'].includes(metaDataName.toLowerCase())) {
                unitNames.add(metaDataName.replace(/\(\d{0,12}\)/, '').trim());
            }
        }
        if (unit.children) {
            for (const child of unit.children) {
                visitUnit(child, loopIndex++);
            }
        }
    };

    session.units.forEach(unit => {
        visitUnit(unit);
    });
    return { cardNames, unitNames };
};

type UseAutoCompleteHandlesReturnType = [
    setSelectValue: React.Dispatch<React.SetStateAction<string | null>>,
    completeOptions: CompleteOptionProps[],
    handleSearch: (value: string, label?: string | null) => void,
    cardSelection: string[],
    unitSelection: string[],
    setCardSelection: React.Dispatch<React.SetStateAction<string[]>>,
    setUnitSelection: React.Dispatch<React.SetStateAction<string[]>>,
];
const useAutoCompleteHandles = (session: Session): UseAutoCompleteHandlesReturnType => {
    const [selectValue, setSelectValue] = useState<string | null>(null);
    const { cardNames, unitNames } = useUnitsNameSet(session);
    const [completeOptions, setCompleteOptions] = useState<CompleteOptionProps[]>([]);
    const [cardSelection, setCardSelection] = useState<string[]>([]);
    const [unitSelection, setUnitSelection] = useState<string[]>([]);

    // 切换项目时 session.projectName 改变，触发 selectValue 重置
    useEffect(() => {
        setSelectValue(null);
        setCardSelection([]);
        setUnitSelection([]);
    }, [session.projectName]);

    useEffect(() => {
        handleSearch('');
    }, [selectValue]);

    useEffect(() => {
        startFilter(session, cardSelection, unitSelection);
    }, [cardSelection, unitSelection]);

    const handleSearch = (value: string, key = selectValue): void => {
        const result: CompleteOptionProps[] = [];
        let targetSet = new Set<string>();
        if (key === 'Unit') {
            targetSet = unitNames;
        }
        if (key === 'Card') {
            targetSet = cardNames;
        }
        targetSet.forEach((cardName) => {
            if (cardName.toLowerCase().includes(value.toLowerCase())) {
                result.push({ label: cardName, value: cardName });
            }
        });
        setCompleteOptions(result);
    };

    return [setSelectValue, completeOptions, handleSearch, cardSelection, unitSelection, setCardSelection, setUnitSelection];
};

const CategorySearchContent = (session: Session): JSX.Element => {
    const theme = useTheme();
    const [setSelectValue, completeOptions, handleSearch, cardSelection, unitSelection, setCardSelection, setUnitSelection] = useAutoCompleteHandles(session);
    const [activeTabKey, setActiveTabKey] = useState<string>(DEFAULT_FILTER_KEY);
    const { t } = useTranslation();

    useEffect(() => {
        setActiveTabKey(DEFAULT_FILTER_KEY);
        handleTabChange(DEFAULT_FILTER_KEY);
    }, [session.projectName]);

    const filterTabItems = useMemo(() => [
        {
            label: t('Card Filter', { ns: 'timeline' }),
            key: 'Card',
            children: (
                <Select
                    id={'select-card-filter-content'}
                    mode="multiple"
                    allowClear
                    placeholder={t('Please select card', { ns: 'timeline' })}
                    options={completeOptions}
                    width={280}
                    height={32}
                    value={cardSelection}
                    onChange={(val: string[]) => setCardSelection(val)}
                >
                </Select>
            ),
        },
        {
            label: t('Units Filter', { ns: 'timeline' }),
            key: 'Unit',
            children: (
                <Select
                    id={'select-unit-filter-content'}
                    mode="multiple"
                    allowClear
                    placeholder={t('Please select unit', { ns: 'timeline' })}
                    options={completeOptions}
                    width={280}
                    height={32}
                    value={unitSelection}
                    onChange={(val: string[]) => setUnitSelection(val)}
                >
                </Select>
            ),
        },
    ], [completeOptions, cardSelection, unitSelection, t]);

    const handleTabChange = (activeKey: string): void => {
        setActiveTabKey(activeKey);
        const tab = filterTabItems.find(item => item.key === activeKey);
        if (tab) {
            const key = String(tab.key);
            setSelectValue(key);
            handleSearch('', key);
        }
    };

    return (
        <CustomDiv theme={theme}>
            <TabsWrapper>
                <Tabs
                    type="card"
                    size="small"
                    tabBarGutter={4}
                    activeKey={activeTabKey}
                    onChange={handleTabChange}
                    items={filterTabItems}
                >
                </Tabs>
                <FixedTooltipWrapper>
                    <Tooltip placement="bottom" title={t('Filter ToolTip', { ns: 'timeline' })}>
                        <HelpIcon style={{ cursor: 'pointer' }} height={20} width={20} />
                    </Tooltip>
                </FixedTooltipWrapper>
            </TabsWrapper>
        </CustomDiv>
    );
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
            <CustomButton data-testid={'tool-filter'} tooltip={t('tooltip:filter')} icon={FilterIcon as any} { ...customButtonProps } ref={ref}/>
        </Tooltip>
    );
});
