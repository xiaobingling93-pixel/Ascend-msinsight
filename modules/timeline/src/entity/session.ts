/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { makeAutoObservable, when } from 'mobx';
import type { FC } from 'react';
import type { Theme } from '@emotion/react';
import { debounce, omit } from 'lodash';
import i18n from 'ascend-i18n';
import { type Caches } from '../cache/cache';
import { toLocalTimeString } from '../utils/humanReadable';
import { type TimeStamp } from './common';
import { Domain, DomainRange } from './domain';
import type { InsightUnit, UnitMatcher, LinkLines } from './insight';
import { type TimeLineMaker, TIME_MAKER_DEFAULT } from './timeMaker';
import { platform } from '../platforms';
import { type Phase, stateTexts } from '../utils/constant';
import { SimpleCache } from '../cache/simplecache';
import { InsightUnitSet } from '../utils/PageSetting';
import { getTimeOffsetKey } from '../insight/units/utils';
import { CardMetaData, SliceData, SliceMeta, ThreadMetaData, ThreadTrace, ThreadTraceRequest } from './data';
import { CardRankInfo } from '../api/interface';
import { getRootUnit } from '../utils';
import { getAutoKey } from '../utils/dataAutoKey';

export const MAX_ZOOM_COUNT = 10000;

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
    zoomHistory: DomainRange[];
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

export interface SelectedDataType extends Pick<ThreadTrace, 'duration' | 'startTime' | 'name'> {
    id?: SliceData['id'];
    type?: string;
    depth?: ThreadTrace['depth'];
    threadId: SliceMeta['threadId'] | SliceData['tid'];
    processId: SliceMeta['processId'];
    cardId?: SliceMeta['cardId'];
    dbPath?: string;
    metaType?: ThreadMetaData['metaType'];
    color?: keyof Theme['colorPalette'] | Array<[ number, keyof Theme['colorPalette'] ]>;
    startRecordTime?: number;
    showSelectedData?: boolean;
    showDetail?: boolean;
}

export type TimelineScale = (x: number) => number;

interface ScaleBag {
    timelineMarkerTimeScale: TimelineScale | null;
    timelineMarkerXScale: TimelineScale | null;
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
    // 是否是服务化场景
    isIE: boolean = false;
    // 是否隐藏了算子调优flag事件
    areFlagEventsHidden: boolean = false;
    // 快捷键对齐触发渲染
    alignRender: boolean = false;
    isCluster: boolean = false;
    isMultiCluster: boolean = false;
    // 页面可视范围的打开的Card的CardId
    viewedExpandedCardIdSet: Set<string> = new Set<string>();
    selectedMultiSlice: string = '';
    isMultiDevice: boolean = false; // 判断项目是否是单Host多Device
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
    scrollTop: number = 0;
    // Timeline模块键盘滚动区域
    scrollArea: string = '';
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
    /**
     * 框选的范围是否已经被锁定
     */
    selectedRangeIsLock: boolean = false;

    /**
     * 锁定的框选范围
     */
    lockRange?: [ TimeStamp, TimeStamp ];

    /**
     * 锁定的泳道个数
     */
    lockUnitCount: number = 0;

    /**
     * 锁定的泳道
     */
    lockUnit: InsightUnit[] = [];

    /**
     * 表格点击的连线类型
     */
    ridLineType: string = '';
    buttons: Array<FC<{ session: Session }>>;

    // set this field with a new matcher to trigger jump-to-target-lane
    locateUnit?: UnitMatcher;

    timer: ReturnType<typeof setInterval> | undefined;

    sharedState: Record<string, unknown> = {}; // used for sharing state across different units

    // timeline flag data source
    timelineMaker: TimeLineMaker = TIME_MAKER_DEFAULT;

    zoom?: {zoomCount: number; zoomPoint?: number };
    doReset: boolean = false;
    eventUnits: InsightUnit[] = [];
    projectName?: string;
    pageSetting: Record<string, {
        domainRange: DomainRange;
        units: InsightUnitSet[];
    } | undefined> = {};

    autoAdjustUnitHeight: boolean = false;
    showBottomPanel: boolean | null = null;
    // 是否处于拖拽场景下
    isDragging: boolean = false;
    debouncedSetZoomingHistory;
    hoverMouseX: number | null = null; // 鼠标悬浮线的 x 轴坐标
    showCreateFlagMarkKey: boolean = false; // 是否显示创建旗帜标记快捷键图标
    scaleBag: ScaleBag = {
        timelineMarkerTimeScale: null,
        timelineMarkerXScale: null,
    };

    private _selectedRange?: [ TimeStamp, TimeStamp ];
    private readonly _domain: Domain;
    // Relative to the startTimeOffset, which means that it will start from 0.
    private _initEndTimeAll: TimeStamp | undefined;
    private _endTimeAll: TimeStamp | undefined;
    private _name: string | null;
    private _phase: Phase = 'configuring';
    private _units: InsightUnit[] = [];
    private readonly _rankCardInfoMap: Map<string, CardRankInfo> = new Map(); // rank key(clusterId+host+realRankId+deviceId) -> CardInfo
    private _availableUnits: InsightUnit[] = [];
    private _selectedData?: SelectedDataType;
    private _benchMarkData?: Record<string, unknown>;
    private _alignSliceData: SliceData[] = [];
    private _selectedRangeData?: Array<Record<string, unknown>>;
    private _interval: number;
    private _selectedUnitKeys: string[] = [];
    private _selectedUnits: InsightUnit[] = []; // redundant for reducing extra computation
    private _disableZoomingHistory: boolean = false; // 禁止生成缩放历史记录

    constructor(conf?: Partial<Session>) {
        makeAutoObservable(this, {
            timer: false,
            selectedUnits: false,
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
        this.debouncedSetZoomingHistory = debounce(this.setZoomingHistory.bind(this), 300);
        this._domain = new Domain(this.isNsMode, this.endTimeAll, this.debouncedSetZoomingHistory);
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

    get domainRange(): DomainRange {
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

    get rankCardInfoMap(): Map<string, CardRankInfo> {
        return this._rankCardInfoMap;
    }

    get availableUnits(): InsightUnit[] {
        return this._availableUnits;
    }

    get selectedData(): SelectedDataType | undefined {
        return this._selectedData;
    }

    get benchMarkData(): Record<string, unknown> | undefined {
        return this._benchMarkData;
    }

    get alignSliceData(): SliceData[] {
        return this._alignSliceData;
    }

    get selectedRangeData(): Array<Record<string, unknown>> | undefined {
        return this._selectedRangeData;
    }

    get selectedUnits(): InsightUnit[] {
        return this._selectedUnits;
    }

    get selectedRange(): [TimeStamp, TimeStamp] | undefined {
        return this._selectedRange;
    }

    get haveCreateFlagMarkPosition(): boolean {
        // 1. 有区间框选时，可以创建旗帜标记
        // 2. 没有区间框选，有鼠标悬浮线的坐标时，可以创建旗帜标记
        return this._selectedRange !== undefined || this.hoverMouseX !== null;
    }

    set endTimeAll(endTimeAll: TimeStamp | undefined) {
        this._initEndTimeAll = endTimeAll;
        this.updateEndTimeAll();
    }

    set name(value: string | null) {
        this._name = value;
    }

    set phase(value: Phase) {
        this._phase = value;
    }

    set selectedUnitKeys(value: string[]) {
        if (this.selectedRangeIsLock) {
            return;
        }
        this._selectedUnitKeys = value;
    }

    set domainRange(domainRange: DomainRange) {
        this._domain.domainRange = domainRange;
        if (!this._disableZoomingHistory) {
            this.debouncedSetZoomingHistory(domainRange);
        }
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

    set selectedData(data: SelectedDataType | undefined) {
        this._selectedData = data;
    }

    set benchMarkData(data: Record<string, unknown> | undefined) {
        this._benchMarkData = data;
    }

    set alignSliceData(data: SliceData[]) {
        this._alignSliceData = data;
    }

    set selectedRangeData(data: Array<Record<string, unknown>> | undefined) {
        this._selectedRangeData = data;
    }

    set selectedUnits(data: InsightUnit[]) {
        if (this.selectedRangeIsLock) {
            return;
        }
        this._selectedUnits = data;
        this._selectedUnitKeys = this._selectedUnits.map(getAutoKey);
    }

    set selectedRange(data: [TimeStamp, TimeStamp] | undefined) {
        if (this.selectedRangeIsLock) {
            console.warn('[WARN] selectedRange is locked, cannot set value.');
            return;
        }
        this._selectedRange = data;
    }

    updateEndTimeAll(): void {
        this._endTimeAll = this._initEndTimeAll === undefined ? undefined : (this._initEndTimeAll + this.getMaxRelativeOffset());
        this._domain !== undefined && (this._domain.endTimeAll = this._endTimeAll ?? this.domain.maxDuration);
    }

    setTimestampOffset(key: string, value: number): void {
        const prevObj = this.unitsConfig.offsetConfig.timestampOffset;
        this.unitsConfig.offsetConfig.timestampOffset = { ...prevObj, [key]: (value) };
        this.updateEndTimeAll();
    }

    setTimestampOffsetAll(offsetConfig: Record<string, number>): void {
        this.unitsConfig.offsetConfig.timestampOffset = { ...offsetConfig };
        this.updateEndTimeAll();
    }

    setTimestampOffsetByUnit(unit: InsightUnit, value: number, shouldUpdate: boolean = true): void {
        const prevObj = { ...this.unitsConfig.offsetConfig.timestampOffset };
        if (unit.children !== undefined && unit.children.length > 0) {
            for (const item of unit.children) {
                const key = getTimeOffsetKey(this, item.metadata as unknown as ThreadTraceRequest);
                prevObj[key] = value;
            }
        }
        this.unitsConfig.offsetConfig.timestampOffset = {
            ...prevObj,
            [(unit.metadata as unknown as CardMetaData).cardId]: (value),
        };
        if (shouldUpdate) { this.updateEndTimeAll(); }
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

    // 对于 Text 的单 Host 多 Device 场景，只保留一个卡，对于 db 的单 Host 多 Device 场景，只保留一个 host
    updateUnitsForMultiDevice(): void {
        const rootUnits = getRootUnit(this._units); // db 情况 this._units 不是根泳道，因此需要先获取根泳道
        if (!this.isMultiDevice) {
            return;
        }
        const len = rootUnits.length;
        for (let i = 1; i < len; ++i) {
            this.setUnitMultiDeviceHidden(rootUnits[i], true);
        }
    }

    setDomainWithoutHistory(domainRange: DomainRange): void {
        this._disableZoomingHistory = true;
        this.domainRange = domainRange;
        this._disableZoomingHistory = false;
    }

    setZoomingHistory(domainRange: DomainRange): void {
        this.contextMenu.zoomHistory.push(domainRange);
        if (this.contextMenu.zoomHistory.length > MAX_ZOOM_COUNT) {
            this.contextMenu.zoomHistory = this.contextMenu.zoomHistory.slice(-MAX_ZOOM_COUNT);
        }
    }

    // 用于更新 unit 下的全部泳道是否可见
    private setUnitMultiDeviceHidden(unit: InsightUnit, hidden: boolean): void {
        const visited = new Set<InsightUnit>();
        const setUnitDisplayable = (child: InsightUnit): void => {
            if (visited.has(child)) {
                return;
            }
            child.isMultiDeviceHidden = hidden;
            visited.add(child);
            if (child.children) {
                for (const item of child.children) {
                    setUnitDisplayable(item);
                }
            }
        };
        setUnitDisplayable(unit);
    }

    private getMaxRelativeOffset(): number {
        if (!Array.isArray(this._units) ||
            this.unitsConfig.offsetConfig.timestampOffset === undefined ||
            Object.keys(this.unitsConfig.offsetConfig.timestampOffset).length === 0) {
            return 0;
        }

        const getRelativeOffset = (unit: InsightUnit): number => {
            const initTimeOffset = unit.alignStartTimestamp ?? 0;
            const timeOffsetKey = getTimeOffsetKey(this, unit.metadata as { cardId?: string; processId?: string });
            const currentTimeOffset = this.unitsConfig.offsetConfig.timestampOffset[timeOffsetKey] ?? 0;
            return Math.max(0, initTimeOffset - currentTimeOffset);
        };

        const result = this._units.flatMap((unit: InsightUnit): number[] => {
            const relativeOffsets = Array.isArray(unit.children) ? unit.children.map(getRelativeOffset) : [];
            relativeOffsets.unshift(getRelativeOffset(unit));
            return relativeOffsets;
        });
        return Math.max(...result);
    }
}
