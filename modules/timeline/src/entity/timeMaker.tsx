/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import type { TimeStamp } from './common';

export interface TimelineAxisFlag {
    uid: string;
    timeStamp: number;
    timeDisplay: string;
    color: string;
    colorCache: string;
    description: string;
    descriptionCache: string;
    type: MarkerType;
    anotherTimeStamp?: number;
};

export type MarkerType = 'point' | 'range';

export interface TimeLineMaker {
    timelineFlagList: TimelineAxisFlag[];
    selectedFlag?: TimelineAxisFlag;
    timelineFlagColorList: string[];
    refreshTrigger: number;
    oldMarkedRange?: [TimeStamp, TimeStamp];
};

export const TIME_MAKER_DEFAULT: TimeLineMaker = {
    timelineFlagList: [],
    timelineFlagColorList: ['#D92C21', '#6456E6', '#1D978A'],
    refreshTrigger: 0,
    selectedFlag: undefined,
    oldMarkedRange: undefined,
};
