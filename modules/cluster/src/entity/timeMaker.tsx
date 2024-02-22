import { TimeStamp } from './common';

export type TimelineAxisFlag = {
    uid: string;
    timeStamp: number;
    timeDisplay: string;
    color: string;
    colorCache: string;
    description: string;
    descriptionCache: string;
    type: MarkerType;
    anotherTimeStamp: number | undefined;
};

export type MarkerType = 'point' | 'range';

export type TimeLineMaker = {
    timelineFlagList: TimelineAxisFlag[];
    selectedFlag: TimelineAxisFlag | undefined;
    timelineFlagColorList: string[];
    refreshTrigger: number;
    oldMarkedRange: undefined | [TimeStamp, TimeStamp];
};

export const TIME_MAKER_DEFAULT: TimeLineMaker = {
    timelineFlagList: [],
    timelineFlagColorList: ['#EF372B', '#6456E6', '#24B0A0'],
    refreshTrigger: 0,
    selectedFlag: undefined,
    oldMarkedRange: undefined,
};
