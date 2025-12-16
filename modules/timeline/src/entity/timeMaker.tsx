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
