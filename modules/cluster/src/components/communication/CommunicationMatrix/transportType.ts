/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import type { PiecewiseVisualMapOption } from 'echarts/types/dist/shared';
import { COLOR } from '../../Common';
import { MatrixType } from './Filter';
import type { TFunction } from 'i18next';
import { baseSerie, getTooltip, type HeatmapData, HeatmapDataIndex } from '../CommunicationMatrix';

enum CompareRes {
    SAME = 0,
    DIFFERENT = 1,
}

export const allTransporType = ['HCCS', 'PCIE', 'RDMA', 'LOCAL', 'SIO'];
export const getTransporTypeNumber = (item: HeatmapData, isCompare: boolean): number => {
    if (isCompare) {
        const compareData = item[HeatmapDataIndex.DATA];
        return compareData.compare.value === compareData.baseline.value ? CompareRes.SAME : CompareRes.DIFFERENT;
    }
    const type = item[HeatmapDataIndex.VALUE] as string;
    return allTransporType.indexOf(type);
};
export const getTransportTypeName = (item: HeatmapData, isCompare: boolean, showAll?: boolean): string => {
    const num = item[HeatmapDataIndex.VALUE] as number;
    if (isCompare) {
        const compareData = item[HeatmapDataIndex.DATA];
        return num === CompareRes.DIFFERENT || showAll ? `${compareData.baseline.value}->${compareData.compare.value}` : `${compareData.compare.value}`;
    }
    return allTransporType[num] ?? '';
};

export const transportTypeVisualMap: PiecewiseVisualMapOption = {
    type: 'piecewise',
    orient: 'horizontal',
    left: 'center',
    bottom: '0',
    splitNumber: 1,
    textStyle: { color: COLOR.GREY_40 },
    dimension: 2,
};

export const getTransportTypeVisualMap = (isCompare: boolean, t: TFunction): PiecewiseVisualMapOption => {
    return {
        ...transportTypeVisualMap,
        pieces: isCompare
            ? [
                { value: 0, label: t('Same'), color: COLOR.BAND_4 },
                { value: 1, label: t('Different'), color: COLOR.BAND_0 },
            ]
            : [
                { value: 0, label: allTransporType[0], color: COLOR.BAND_0 },
                { value: 1, label: allTransporType[1], color: COLOR.BAND_1 },
                { value: 2, label: allTransporType[2], color: COLOR.BAND_2 },
                { value: 3, label: allTransporType[3], color: COLOR.BAND_3 },
                { value: 4, label: allTransporType[4], color: COLOR.BAND_4 },
            ],
    };
};

export const getTransportTypeSerie = ({ data, rankIds, type, isCompare, t }: {
    data: HeatmapData[];
    rankIds: number[];
    type: MatrixType;
    isCompare: boolean;
    t: TFunction;
}): any => {
    const numData = data.map(item => [...item.slice(0, HeatmapDataIndex.VALUE),
        getTransporTypeNumber(item, isCompare),
        ...item.slice(HeatmapDataIndex.VALUE + 1)]);
    return {
        ...baseSerie,
        data: numData,
        label: {
            show: rankIds.length <= 16,
            formatter: (params: {value: HeatmapData}): string => getTransportTypeName(params.value, isCompare),
        },
        tooltip: getTooltip({ t, type, isCompare }),
    };
};
