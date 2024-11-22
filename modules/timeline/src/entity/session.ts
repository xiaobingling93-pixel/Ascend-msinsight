/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { makeAutoObservable, runInAction, when } from 'mobx';
import type { FC } from 'react';
import { type Caches } from '../cache/cache';
import { toLocalTimeString } from '../utils/humanReadable';
import { type TimeStamp } from './common';
import { Domain } from './domain';
import type { InsightUnit, UnitMatcher, LinkLines, LinkDataDesc } from './insight';
import { type TimeLineMaker, TIME_MAKER_DEFAULT } from './timeMaker';
import { omit } from 'lodash';
import { platform } from '../platforms';
import i18n from 'ascend-i18n';
import { type Phase, stateTexts } from '../utils/constant';
import { SimpleCache } from '../cache/simplecache';
import { InsightUnitSet } from '../utils/PageSetting';

export interface SelectedParams {
    baseRawId?: number;
    curRawId?: number;
}

export type SelectedData = Record<string, unknown> & {
    sourceUnit?: InsightUnit;
};

export type DataWithHeight<T> = T & { height?: number };
export interface LinkedDataWithHeight<T> {
    data: DataWithHeight<T>;
    datas: Array<DataWithHeight<T>>;
};
export type ValidSession = Session & { startRecordTime: TimeStamp; phase: Exclude<Phase, 'configuring'> };

export function isValidSession(session?: Session): session is ValidSession {
    return !(session === undefined || session.phase === 'configuring' || session.startRecordTime === undefined);
}
export type LinkDataType<T extends Record<string, unknown> = Record<string, unknown>> = T & { startTime: number; height: number; duration: number };
export type DataMatcher = (unit: InsightUnit) => boolean;
export interface LinkData {
    target: {
        data: LinkDataType;
        matcher: DataMatcher;
    };
    sources: Array<{ data: LinkDataType; matcher: DataMatcher }>;
};
export interface ContextMenu {
    isVisible: boolean;
    zoomHistory: Array<{ domainStart: TimeStamp; domainEnd: TimeStamp }>;
};

interface UnitsConfig {
    jsAllocationUsage: {
        isRecordStackTraces: boolean;
    };
    nativeConfig: {
        filterSize: number;
        maxStackDepth: number;
    };
    offsetConfig: {
        timestampOffset: Record<string, number>;
    };
    filterConfig: {
        pythonFunction: Record<string, boolean>;
    };
}

export class Session {
    language: 'zhCN' | 'enUS' = 'enUS';
    id = '';
    remoteAttrs: Map<string, Record<string, unknown>> = new Map();
    singleLinkLine: LinkLines = {};
    linkLineCategories: string[] = [];
    // 是否等待解析
    isPending: boolean = false;
    // 是否loading解析按钮
    isParserLoading: boolean = false;
    // 是否是算子仿真图
    isSimulation: boolean = false;
    // 是否隐藏了算子调优flag事件
    areFlagEventsHidden: boolean = false;
    isCluster: boolean = false;
    // 页面可视范围的Card的CardId
    viewedCardIdSet: Set<string> = new Set<string>();
    selectedMultiSlice: string = '';
    isFullDb: boolean = false;
    // 是否是ipynb文件
    isIpynb: boolean = false;
    ipynbUrl: string = '';
    // context menu state
    contextMenu: ContextMenu = {
        isVisible: false,
        zoomHistory: [],
    };

    pinnedUnits: InsightUnit[] = [];
    icon?: JSX.Element;
    caches: Caches | null = null;
    simpleCache: SimpleCache;

    // Frontend start time of recording.
    startRecordTime: TimeStamp;

    // Any data out of max duration would be dropped, Number.MAX_SAFE_INTEGER means unlimited
    maxDuration = Number.MAX_SAFE_INTEGER;

    // should use ns time unit
    isNsMode: boolean = false;

    // some params for selected value which is not a range.
    selectedParams: SelectedParams = { baseRawId: undefined, curRawId: undefined };
    selectedRange?: [ TimeStamp, TimeStamp ];
    scrollTop: number = 0;
    expandedUnitKeys: string[] | [] = [];
    selectedUnits: InsightUnit[] = []; // redundant for reducing extra computation
    selectedDetailKeys: [string] | [] = [];
    selectedDetails: [Record<string, unknown>] | [] = []; // redundant for reducing extra computation
    unitsConfig: UnitsConfig = {
        jsAllocationUsage: {
            isRecordStackTraces: false,
        },
        nativeConfig: {
            filterSize: 4096,
            maxStackDepth: 10,
        },
        offsetConfig: {
            timestampOffset: {},
        },
        filterConfig: {
            pythonFunction: {},
        },
    };

    searchData?: { [x: string]: unknown; content: string; isMatchCase: boolean; isMatchExact: boolean };
    doContextSearch?: boolean;
    showEvent?: boolean;
    linkData?: LinkData;
    linkLines: LinkLines = {};

    totalHeight: number = 0;
    renderTrigger: boolean = true;
    /**
     * 页面是否有m快捷键的遮罩m
     */
    mKeyRender: boolean = false;
    /**
     * m快捷键遮罩范围
     */
    mMaskRange: number[] = [];

    linkFlow?: Record<string, unknown>;
    linkDetail?: LinkDataDesc<Record<string, unknown>>;
    buttons: Array<FC<{ session: Session }>>;

    // set this field with a new matcher to trigger jump-to-target-lane
    locateUnit?: UnitMatcher;

    timer: ReturnType<typeof setInterval> | undefined;

    sharedState: Record<string, unknown> = {}; // used for sharing state across different units

    // timeline flag data source
    timelineMaker: TimeLineMaker = TIME_MAKER_DEFAULT;

    zoom?: {zoomCount: number; zoomPoint?: number };
    doReset: boolean = false;
    memoryRankIds: string[] = [];
    operatorRankIds: string[] = [];
    eventUnits: InsightUnit[] = [];
    projectName?: string;
    pageSetting: Record<string, {
        domainRange: { domainStart: TimeStamp; domainEnd: TimeStamp };
        units: InsightUnitSet[];
    } | undefined> = {};

    autoAdjustUnitHeight: boolean = false;

    private readonly _domain: Domain;
    private _selectedUnitKeys: string[] = [];
    // Relative to the startTimeOffset, which means that it will start from 0.
    private _endTimeAll: TimeStamp | undefined;
    private _name: string | null;
    private _phase: Phase = 'configuring';
    private _units: InsightUnit[] = [];
    private _availableUnits: InsightUnit[] = [];
    private _selectedData?: Record<string, unknown>;
    private _selectedRangeData?: Array<Record<string, unknown>>;
    private _interval: number;

    constructor(conf?: Partial<Session>) {
        makeAutoObservable(this, {
            timer: false,
            selectedUnits: false,
            selectedDetails: false,
            caches: false,
            isNsMode: false,
            printSessionInfo: false,
            linkLines: false,
        });
        this._name = conf?.name ?? this.id;
        this._interval = 100;
        this.startRecordTime = 0;
        if (conf) {
            Object.assign(this, conf);
        }
        this._domain = new Domain(this.isNsMode, this.endTimeAll);
        this.buttons = conf?.buttons ?? [];
        this.simpleCache = new SimpleCache();
        // 录制时长大于等于5min，建议结束录制
        const MAXTIME = this.isNsMode ? 5 * 60 * 1e9 : 5 * 60 * 1e3;
        when(
            () => this._endTimeAll !== undefined && this._endTimeAll >= MAXTIME,
            () => {
                const showTimeoutTip = window.localStorage.getItem('showTimeoutTip');
                if (showTimeoutTip === null) {
                    platform.notify(i18n.t('notify:5012'));
                    window.localStorage.setItem('showTimeoutTip', 'false');
                }
            },
        );
    }

    get endTimeAll(): TimeStamp | undefined {
        return this._endTimeAll;
    }

    get name(): string | null {
        return this._name;
    }

    get phase(): Phase {
        return this._phase;
    }

    get statusInfo(): string {
        if (this.phase === 'configuring') {
            return 'Idle';
        } else if (this.phase === 'download' && this.startRecordTime !== undefined) {
            return `Recorded at: ${toLocalTimeString(this.startRecordTime)}`;
        } else {
            return stateTexts[this.phase];
        }
    }

    get selectedUnitKeys(): (string[]) {
        return this._selectedUnitKeys;
    }

    get domainRange(): { domainStart: TimeStamp; domainEnd: TimeStamp } {
        const { domainStart, domainEnd } = this._domain.domainRange;
        return { domainStart, domainEnd };
    }

    get realTimeUpdate(): boolean {
        return this._domain.realTimeUpdate && this.phase === 'recording';
    }

    get domain(): Domain {
        return this._domain;
    }

    get interval(): number {
        return this._interval;
    }

    get units(): InsightUnit[] {
        return this._units;
    }

    get availableUnits(): InsightUnit[] {
        return this._availableUnits;
    }

    get selectedData(): Record<string, unknown> | undefined {
        return this._selectedData;
    }

    get selectedRangeData(): Array<Record<string, unknown>> | undefined {
        return this._selectedRangeData;
    }

    set endTimeAll(endTimeAll: TimeStamp | undefined) {
        this._endTimeAll = endTimeAll;
        this._domain !== undefined && (this._domain.endTimeAll = endTimeAll ?? this.domain.maxDuration);
    }

    set name(value: string | null) {
        this._name = value;
    }

    set phase(value: Phase) {
        this._phase = value;
    }

    set selectedUnitKeys(value: string[]) {
        this._selectedUnitKeys = value;
        // 'More' panel should be cleared when selected unit is changed
        runInAction(() => {
            this.selectedDetailKeys = [];
            this.selectedDetails = [];
        });
    }

    set domainRange(domainRange: { domainStart: TimeStamp; domainEnd: TimeStamp }) {
        this._domain.domainRange = domainRange;
    }

    set realTimeUpdate(realTime: boolean) {
        this._domain.realTimeUpdate = realTime && this.phase === 'recording';
    }

    set interval(value: number) {
        this._interval = value;
    }

    set units(units: InsightUnit[]) {
        this._units = units;
    }

    set availableUnits(availableUnits: InsightUnit[]) {
        this._availableUnits = availableUnits;
    }

    set selectedData(data: Record<string, unknown> | undefined) {
        this._selectedData = data;
        this.linkFlow = undefined;
        this.linkData = undefined;
        this.linkDetail = undefined;
    }

    set selectedRangeData(data: Array<Record<string, unknown>> | undefined) {
        this._selectedRangeData = data;
    }

    printSessionInfo(): string {
        return `${JSON.stringify({ ...omit(this, ['caches', 'sharedState', '_units']) })}`;
    }

    sortUnits(): void {
        const sorter = (a: InsightUnit, b: InsightUnit): number => {
            const aName = (a.metadata as any).cardId as string;
            const bName = (b.metadata as any).cardId as string;
            if (aName.includes('Host') || bName.includes('Host')) {
                return aName.includes('Host') ? -1 : 1;
            }
            if (aName.length === bName.length) {
                return aName.localeCompare(bName);
            }
            return aName.length - bName.length;
        };
        this.units.sort(sorter);
        this.units.forEach(unit => {
            unit.children?.sort(sorter);
        });
    }

    setSelectedUnitKeys(value: [string] | []): void {
        this._selectedUnitKeys = value;
    };
}
