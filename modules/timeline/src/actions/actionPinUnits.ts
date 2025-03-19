/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { register } from './register';
import type { Session } from '../entity/session';
import { runInAction } from 'mobx';
import { isPinned, switchPinned } from '../components/ChartContainer/unitPin';
import type { InsightUnit } from '../entity/insight';
import type { ThreadMetaData } from '../entity/data';
import { getAutoKey } from '../utils/dataAutoKey';
import { preOrderFlatten } from '../entity/common';
import { getRootUnit } from '../utils';

interface SelectedUnitStatus {
    isThreadNameStartWithGroup: boolean;
    isGroupCommunicationUnit: boolean;
    groupNameValue: string;
    isPinned: boolean;
}

interface SelectedUnitListStatus {
    isAllThreadNameStartWithGroup: boolean;
    isAllGroupCommunicationUnit: boolean;
    isSameGroupNameValue: boolean;
    groupNameValue: string;
    isAllPinned: boolean;
    isAllUnpinned: boolean;
}

function calculateSelectedUnitStatus(selectedUnit: InsightUnit): SelectedUnitStatus {
    const hasStringValue = (str: string = ''): boolean => {
        return str !== '';
    };
    const metadata = selectedUnit.metadata as ThreadMetaData;
    return {
        isThreadNameStartWithGroup: metadata?.threadName?.includes('Group') ?? false,
        isGroupCommunicationUnit: hasStringValue(metadata?.groupNameValue),
        groupNameValue: (selectedUnit.metadata as ThreadMetaData)?.groupNameValue ?? '',
        isPinned: isPinned(selectedUnit),
    };
}

export function calculateSelectedUnitListStatus(selectedUnits: InsightUnit[]): SelectedUnitListStatus {
    const selectedUnitStatuses = selectedUnits.map(calculateSelectedUnitStatus);
    const isAllThreadNameStartWithGroup = selectedUnitStatuses
        .every(({ isThreadNameStartWithGroup }) => isThreadNameStartWithGroup);
    const isAllGroupCommunicationUnit = selectedUnitStatuses
        .every(({ isGroupCommunicationUnit }) => isGroupCommunicationUnit);

    const isAllPinned = selectedUnitStatuses.every((status) => status.isPinned);
    const isAllUnpinned = selectedUnitStatuses.every((status) => !status.isPinned);

    const groupNameValue = selectedUnitStatuses?.[0]?.groupNameValue ?? '';
    const isSameGroupNameValue = isAllGroupCommunicationUnit && selectedUnitStatuses
        .reduce((acc, curr) => acc && curr.groupNameValue === groupNameValue, true);
    return {
        isAllThreadNameStartWithGroup,
        isAllGroupCommunicationUnit,
        isSameGroupNameValue,
        groupNameValue,
        isAllPinned,
        isAllUnpinned,
    };
}

function unpinAll(session: Session): void {
    const { pinnedUnits } = session;
    runInAction(() => {
        session.pinnedUnits = [];
        pinnedUnits.forEach((item): void => switchPinned(item));
    });
}

function getSameGroupNameUnits(session: Session): InsightUnit[] {
    if (session.selectedUnits.length === 0) {
        return [];
    }
    const selectedGroupNameValue = (session.selectedUnits[0].metadata as ThreadMetaData).groupNameValue;
    const flattenUnits = preOrderFlatten(getRootUnit(session.units), 0);
    return flattenUnits
        .filter(({ metadata }) => (metadata as ThreadMetaData)?.groupNameValue === selectedGroupNameValue);
}

function pinAllSameGroupNameValue(session: Session): void {
    if (session.selectedUnits.length === 0) {
        return;
    }
    const sameGroupNameValueUnits = getSameGroupNameUnits(session);
    if (sameGroupNameValueUnits.length === 0) {
        return;
    }
    const pinnedUnitKeys = session.pinnedUnits.map((item) => getAutoKey(item));
    const addUnits = sameGroupNameValueUnits.reduce<InsightUnit[]>((acc, curr): InsightUnit[] => {
        const key = getAutoKey(curr);
        if (pinnedUnitKeys.includes(key)) {
            return acc;
        }
        acc.push(curr);
        pinnedUnitKeys.push(key);
        return acc;
    }, []);
    runInAction(() => {
        session.pinnedUnits = [...session.pinnedUnits, ...addUnits];
        addUnits.forEach((item): void => switchPinned(item));
    });
}

function unpinAllSameGroupNameValue(session: Session): void {
    const sameGroupNameValueUnits = getSameGroupNameUnits(session);
    if (sameGroupNameValueUnits.length === 0) {
        return;
    }
    const { pinnedUnits } = session;
    const pinnedUnitKeys = pinnedUnits.map((item) => getAutoKey(item));
    const subtractUnits = sameGroupNameValueUnits.reduce<InsightUnit[]>((acc, curr): InsightUnit[] => {
        const key = getAutoKey(curr);
        const findIdx = pinnedUnitKeys.findIndex((pinnedKey: string): boolean => pinnedKey === key);
        if (findIdx < 0) {
            return acc;
        }
        acc.push(curr);
        pinnedUnits.splice(findIdx, 1);
        pinnedUnitKeys.splice(findIdx, 1);
        return acc;
    }, []);
    runInAction(() => {
        session.pinnedUnits = [...pinnedUnits];
        subtractUnits.forEach((item): void => switchPinned(item));
    });
}

export const actionUnpinAll = register({
    name: 'unpinAll',
    label: 'timeline:contextMenu.Unpin All',
    visible: (session) => {
        // 为多选 unit 功能做准备
        const selectedUnitListStatus = calculateSelectedUnitListStatus(session.selectedUnits);
        return selectedUnitListStatus.isAllPinned;
    },
    perform: (session): void => {
        // 为多选 unit 功能做准备
        const selectedUnitListStatus = calculateSelectedUnitListStatus(session.selectedUnits);
        if (!selectedUnitListStatus.isAllPinned) {
            return;
        }
        unpinAll(session);
    },
});

export const actionPinByGroupNameValue = register({
    name: 'pinByGroupNameValue',
    label: '',
    perform: (session): void => {
        const selectedUnitListStatus = calculateSelectedUnitListStatus(session.selectedUnits);
        if (!selectedUnitListStatus.isAllUnpinned) {
            return;
        }
        pinAllSameGroupNameValue(session);
    },
});

export const actionUnpinByGroupNameValue = register({
    name: 'unpinByGroupNameValue',
    label: '',
    perform: (session): void => {
        const selectedUnitListStatus = calculateSelectedUnitListStatus(session.selectedUnits);
        if (!selectedUnitListStatus.isAllPinned) {
            return;
        }
        unpinAllSameGroupNameValue(session);
    },
});
