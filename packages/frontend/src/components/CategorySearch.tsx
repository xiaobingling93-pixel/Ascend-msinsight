import { useTheme } from '@emotion/react';
import styled from '@emotion/styled';
import { Dropdown, Menu, Pagination, Select, Tooltip } from 'antd';
import type { InputRef } from 'antd';
import { PaginationProps } from 'antd/lib/pagination';
import { runInAction } from 'mobx';
import { observer } from 'mobx-react';
import React, { ChangeEvent, Dispatch, SetStateAction, useEffect, useMemo, useRef, useState } from 'react';
import { ReactComponent as SearchIcon } from '../assets/images/insights/ic_search_lined.svg';
import { ReactComponent as CloseIcon } from '../assets/images/insights/ic_close_filled.svg';
import { Session } from '../entity/session';
import { platform } from '../platforms';
import { CustomButton } from './base/StyledButton';
import { StyledInput } from './base/StyledInput';
import { Logger } from '../utils/Logger';
import { InsightUnit, MenuType } from '../entity/insight';
import { preOrderFlatten } from '../entity/common';
import { isPinned } from './ChartContainer/unitPin';
import { FrameSearchResultType, ThreadInfo } from '../entity/data';
import { EventHandler, EventType, useEventBus } from '../utils/eventBus';
import { MenuClickEventHandler } from 'rc-menu/lib/interface';

interface TaskData {
    type: string;
    startTime: number;
    duration: number;
    depth: number;
    pid: number;
    tid: number;
    taskName: string;
    cookie: number;
};

type SelectOptionType = { value: string; label: string };

type ResultType = {
    key: string;
    showKey: string | undefined;
    value: string | number;
    showValue: string;
    mode: 'input' | 'keyValue';
    isTrigger?: boolean;
    options?: SelectOptionType[];
    type?: 'number' | 'string';
};

type UseDataType = {
    result: ResultType[];
    setResult: Dispatch<SetStateAction<ResultType[]>>;
    visible: { icon: boolean; input: boolean };
    setVisible: Dispatch<SetStateAction<{ icon: boolean; input: boolean }>>;
    MenuJSX: JSX.Element;
    isDisableMenu: boolean;
    curInput: MenuType | undefined;
    paginationData: { current: number; total: number };
    updatePaginationData: React.Dispatch<React.SetStateAction<{current: number; total: number}>>;
    searchContent: string;
    setSearchContent: React.Dispatch<React.SetStateAction<string>>;
    time: { startTimeAll: number; endTimeAll: number; startRecordTime: number };
    setTime: React.Dispatch<React.SetStateAction<{ startTimeAll: number; endTimeAll: number; startRecordTime: number }>>;
    inputRef: React.RefObject<InputRef>;
};

type UseMenuType = {
    menuList: MenuType[];
    result: ResultType[];
    visible: { icon: boolean; input: boolean };
    setVisible: Dispatch<SetStateAction<{ icon: boolean; input: boolean }>>;
    clickItem: MenuClickEventHandler;
};

type UseMenuRet = {
    MenuJSX: JSX.Element;
    isDisableMenu: boolean;
    curInput: MenuType | undefined;
};

type DomEventType = React.MouseEvent<HTMLElement> | React.KeyboardEvent<HTMLElement>;

type ChooseResultType = {
    session: Session;
    result: ResultType[];
    setResult: Dispatch<SetStateAction<ResultType[]>>;
    visible: { icon: boolean; input: boolean };
    setVisible: React.Dispatch<React.SetStateAction<{ icon: boolean; input: boolean }>>;
    setTime: React.Dispatch<React.SetStateAction<{ startTimeAll: number; endTimeAll: number; startRecordTime: number }>>;
    updatePaginationData: React.Dispatch<React.SetStateAction<{current: number; total: number}>>;
    setSearchContent: React.Dispatch<React.SetStateAction<string>>;
};

type PressEnterType = {
    searchContent: string;
    setSearchContent: React.Dispatch<React.SetStateAction<string>>;
    result: ResultType[];
    setResult: Dispatch<SetStateAction<ResultType[]>>;
    curInput: MenuType | undefined;
};

type SelectResultItemType = {
    curValue: string;
    option: SelectOptionType;
    result: ResultType[];
    setResult: Dispatch<SetStateAction<ResultType[]>>;
    visible: { icon: boolean; input: boolean };
    setVisible: React.Dispatch<React.SetStateAction<{ icon: boolean; input: boolean }>>;
    setSearchContent: React.Dispatch<React.SetStateAction<string>>;
};

const CustomDiv = styled.div<{isDisableMenu: boolean}>`
    display: flex;
    align-items: center;
    justify-content: space-between;
    border-radius: 18px;
    padding: 1px 7px 1px 10px;
    min-width: 600px;
    background: ${props => props.theme.tooltipBGColor};
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
            color: ${props => props.theme.deviceProcessContentFontColor};
            background: ${props => props.theme.multiSelectBgColor};
            border-radius: 12px;
            margin: 0 5px;
            .icon {
                width: 12px;
                height: 12px;
                margin-left: 5px;
                fill: ${props => props.theme.buttonColor.enableClickColor};
            }
            & > div {
                padding-left: 8px;
            }
            .ant-select {
                color: ${props => props.theme.deviceProcessContentFontColor};
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
                    color: ${props => props.theme.deviceProcessContentFontColor};
                }
                // 去掉下拉三角按钮
                .ant-select-arrow {
                    display: none;
                }
                .ant-select-dropdown {
                    width: 200px!important;
                    border-radius: 16px;
                    left: -7px !important;
                    background: ${props => props.theme.deviceProcessBackgroundColor};
                    .rc-virtual-list-holder-inner {
                        & > div {
                            border-radius: 10px;
                            color: ${props => props.theme.filterColor};
                        }
                        .ant-select-item-option-active {
                            color: ${props => props.theme.filterColor} !important;
                            background-color: ${props => props.theme.filterSelectActiveBgColor} !important;
                        }
                    }
                }
            }
        }
    }
    .popup {
        display: flex;
        align-items: center;
        width: ${props => props.isDisableMenu ? 0 : '100px'};
        height: 32px;
        padding-left: ${props => props.isDisableMenu ? 0 : '5px'};
        color: ${props => props.theme.filterTipColor};
        flex-grow: ${props => props.isDisableMenu ? 0 : 1};
        cursor: pointer;
        .ant-dropdown {
            width: 200px;
            min-width: 0px !important;
            animation-duration: 0s !important;
            .ant-dropdown-menu, .ant-dropdown-menu-item {
                background: ${props => props.theme.deviceProcessBackgroundColor};
                color: ${props => props.theme.filterColor};
                border-radius: 10px;
                .ant-dropdown-menu-item-active {
                    color: ${props => props.theme.filterColor};
                    background-color: ${props => props.theme.filterSelectActiveBgColor};
                }
            }
        }
        &::after {
            content: ${props => props.isDisableMenu ? '' : '\'Please select\''};
            font-size: ${props => props.isDisableMenu ? '' : '14px'};
        }
    }
    .searchResult {
        color: ${(props) => props.theme.svgBackgroundColor};
        font-size: 12px;
        white-space: nowrap;
    }
`;

const StylePagination = styled((props: PaginationProps) => <Pagination {...props} />)`
    .ant-pagination-item-link {
        color: ${(props) => props.theme.fontColor};
        display: inline-block;
    }
    .ant-pagination-simple-pager {
        color: ${(props) => props.theme.fontColor};
        input {
            color: rgba(0, 0, 0, 0.9);
        }
    }
`;

const findUnitNameByResult = (result: ResultType[]): { unitName: string; resultObj: Record<string, unknown> } => {
    const resultObj: Record<string, unknown> = {};
    result.forEach(item => {
        const value = item.value;
        if ((typeof value === 'string' && value.trim().length > 0) || typeof value === 'number') {
            Object.assign(resultObj, { [item.key]: value });
        }
    });
    return { unitName: resultObj.unit as string, resultObj };
};

// 获取搜索的结果数量
const queryDataCount = async (session: Session, result: ResultType[],
    setTime: React.Dispatch<React.SetStateAction<{ startTimeAll: number; endTimeAll: number; startRecordTime: number }>>): Promise<number> => {
    // const { unitName, resultObj } = findUnitNameByResult(result);
    // if (unitName === undefined) {
    //     return 0;
    // }
    // const [ startTimeAll, endTimeAll ] = getRelativeAll(session as ValidSession);
    // const time = { startRecordTime: session.startRecordTime ?? 0, startTimeAll, endTimeAll };
    // setTime(time);
    // return (await engine.fetchData({ session, params: { globalSearchCount: { ...time, ...resultObj } } })).globalSearchCount ?? 0;
    return 0;
};

const calculateDomainRange = (session: Session, startTime: number, duration: number): [ number, number ] => {
    let rangeStart = startTime - duration * 9;
    rangeStart = rangeStart > 0 ? rangeStart : 0;
    const rangeEnd = Math.min(startTime + duration * 10, session.endTimeAll ?? Number.MAX_SAFE_INTEGER);
    return [ rangeStart, rangeEnd ];
};

const doJumpCpuSlice = (session: Session, cpu?: number, startTime?: number, duration?: number): void => {
    if (cpu === undefined || startTime === undefined || duration === undefined) {
        Logger('doJumpCpuSlice', `cpu: ${cpu}, startTime: ${startTime}, duration: ${duration}, some of them is undefined.`, 'warn');
        return;
    }
    // runInAction(() => {
    //     session.locateUnit = {
    //         target: (unit) => unit instanceof CPUSliceUnit && unit.metadata === cpu,
    //         onSuccess: (unit) => {
    //             const [ rangeStart, rangeEnd ] = calculateDomainRange(session, startTime, duration);
    //             session.domainRange = { domainStart: rangeStart, domainEnd: rangeEnd };
    //             session.selectedData = { cpuCoreId: cpu, duration, startTime };
    //         },
    //     };
    // });
};

const doJumpSystraceSlice = (session: Session, taskData?: TaskData): void => {
    if (taskData === undefined) {
        Logger('doJumpSystraceSlice', 'taskData is undefined.', 'warn');
        return;
    }
    runInAction(() => {
        if (taskData.type === 'threadTrace') {
            // session.locateUnit = {
            //     target: (unit) => unit instanceof CPUThreadTraceUnit && (unit.metadata as ThreadTraceMetadataChildren).tid === taskData.tid,
            //     onSuccess: (unit) => {
            //         const [ rangeStart, rangeEnd ] = calculateDomainRange(session, taskData.startTime, taskData.duration);
            //         session.domainRange = { domainStart: rangeStart, domainEnd: rangeEnd };
            //         session.selectedData = {
            //             threadId: taskData.tid,
            //             duration: taskData.duration,
            //             startTime: taskData.startTime,
            //             depth: taskData.depth,
            //             name: taskData.taskName,
            //         };
            //     },
            // };
        } else if (taskData.type === 'userTrace') {
            // session.locateUnit = {
            //     target: (unit) => unit instanceof CPUUserTraceUnit && (unit.metadata as UserTraceMetadataChildren).pid === taskData.pid && (unit.metadata as UserTraceMetadataChildren).taskName === taskData.taskName,
            //     onSuccess: (unit) => {
            //         const [ rangeStart, rangeEnd ] = calculateDomainRange(session, taskData.startTime, taskData.duration);
            //         session.domainRange = { domainStart: rangeStart, domainEnd: rangeEnd };
            //         session.selectedData = {
            //             name: taskData.cookie,
            //             taskName: taskData.taskName,
            //             timestamp: taskData.startTime,
            //             duration: taskData.duration,
            //             depth: taskData.depth,
            //         };
            //     },
            // };
        }
    });
};

// 帧跳转
const doJumpFrameSlice = (session: Session, frameData?: FrameSearchResultType): void => {
    const { processId, startTime, endTime, depth, isJank } = frameData ?? {};
    if (frameData === undefined) {
        Logger('doJumpFrameSlice', `processId: ${processId}, startTime: ${startTime}, endTime: ${endTime},
        depth: ${depth}, isJank: ${isJank} some of them is undefined.`);
        return;
    }
    // runInAction(() => {
    //     session.locateUnit = {
    //         target: (unit) => {
    //             const res = unit instanceof FrameLeafUnit && unit.metadata.processId === processId;
    //             return res;
    //         },
    //         onSuccess: (unit) => {
    //             const duration = endTime - startTime;
    //             const [ rangeStart, rangeEnd ] = calculateDomainRange(session, startTime, duration);
    //             session.domainRange = { domainStart: rangeStart, domainEnd: rangeEnd };
    //             session.selectedData = { processId, startTime, endTime, depth, isJank, duration };
    //         },
    //     };
    // });
};

// callStack 范围框选
const doSelectRange = (session: Session, data?: { startTime: number; endTime: number }, threadId?: number): void => {
    if (data === undefined) {
        Logger('doSelectRange', 'taskData is undefined.');
        return;
    }
    const { startTime, endTime } = data;
    // const data: ((unit: InsightUnit) => boolean) = threadId === undefined
    //     ? (unit) => unit instanceof JsCpuTime
    //     : (unit) => unit instanceof ThreadCpuTime && (unit.metadata as ThreadInfo).tid === Number(threadId);
    // runInAction(() => {
    //     session.locateUnit = {
    //         target: data,
    //         onSuccess: (unit) => {
    //             const duration = endTime - startTime;
    //             const [ rangeStart, rangeEnd ] = calculateDomainRange(session, startTime, duration);
    //             session.domainRange = { domainStart: rangeStart, domainEnd: rangeEnd };
    //             session.selectedRange = [ startTime, endTime ];
    //         },
    //     };
    // });
};

// 跳转分发函数
const jumpSlice = async (session: Session, result: ResultType[],
    time: { startTimeAll: number; endTimeAll: number; startRecordTime: number }, index: number): Promise<void> => {
    const { unitName, resultObj } = findUnitNameByResult(result);
    if (unitName === undefined) { return; }

    switch (unitName) {
        case 'frame': {
            // const frame = (await engine.fetchData({ session, params: { frameSearchResult: { ...resultObj, startTimeAll: time.startTimeAll, endTimeAll: time.endTimeAll, index } } })).frameSearchResult;
            doJumpFrameSlice(session, undefined);
            break;
        }
        case 'CPUCore': {
            // const CPUCore = (await engine.fetchData({ session, params: { CPUCoreSearchResult: { ...resultObj, endTimeAll: time.endTimeAll, index } } })).CPUCoreSearchResult;
            // CPUCore.cpu, CPUCore.startTime, CPUCore.duration
            doJumpCpuSlice(session, undefined, undefined, undefined);
            break;
        }
        case 'process': {
            // const process = (await engine.fetchData({ session, params: { processSearchResult: { ...resultObj, endTimeAll: time.endTimeAll, index } } })).processSearchResult;
            doJumpSystraceSlice(session, undefined);
            break;
        }
        case 'arkTSCallstack': {
            // const arkTS = (await engine.fetchData({ session, params: { arkTSSearchResult: { ...resultObj, ...time, index } } })).arkTSSearchResult;
            doSelectRange(session, undefined);
            break;
        }
        case 'nativeCallstack': {
            // const native = (await engine.fetchData({ session, params: { nativeSearchResult: { ...resultObj, ...time, index } } })).nativeSearchResult;
            doSelectRange(session, undefined, resultObj.threadId as number);
            break;
        }
    }
};

// 请求搜索结果的count
const searchCount = async (session: Session, result: ResultType[],
    setTime: React.Dispatch<React.SetStateAction<{ startTimeAll: number; endTimeAll: number; startRecordTime: number }>>,
    setVisible: React.Dispatch<React.SetStateAction<{icon: boolean; input: boolean}>>,
    updatePaginationData: React.Dispatch<React.SetStateAction<{current: number; total: number}>>): Promise<void> => {
    setVisible({ icon: false, input: false });
    const totalCnt = await queryDataCount(session, result, setTime).catch(() => 0);
    updatePaginationData({ current: 0, total: totalCnt });
};

// 生成下一步的菜单选项
const useMenu = ({ menuList, result, visible, setVisible, clickItem }: UseMenuType): UseMenuRet => {
    const menuArr = useMemo(() => {
        // 根据list计算出MenuList
        let temp: MenuType[] | undefined = menuList;
        let tempObj: MenuType | undefined;
        let curInput: MenuType | undefined;
        let climbOverValue = '';
        for (let i = 0; i < result.length; i++) {
            const curResult = result[i];
            tempObj = temp?.find(item => {
                if (item.mode === 'keyValue') {
                    return item.key === curResult.key && item.value === curResult.value;
                }
                climbOverValue = (item.mode === 'key' && curResult.key === item.key && curResult.value !== '') ? curResult.value as string : '';
                return curResult.key === item.key;
            });
            tempObj = climbOverValue !== '' ? tempObj?.children?.find(item => item.value === climbOverValue) : tempObj;
            if ((tempObj?.children?.length ?? 0) === 0 || (tempObj?.mode === 'input' && curResult.value === '')) {
                curInput = tempObj;
                break;
            }
            temp = tempObj?.children;
        }
        const isDisableMenu = curInput !== undefined;
        setVisible({ ...visible, input: isDisableMenu && result.some(item => item.value === '') });
        // 为修改选项做准备
        let options: SelectOptionType[];
        if (temp?.[0]?.mode === 'value' || temp?.[0]?.mode === 'keyValue') {
            options = temp.map(item => ({ value: item.value as string, label: item.showValue as string }));
        }
        return {
            MenuJSX: temp !== undefined && temp.length > 0
                ? <Menu onClick={clickItem} key="menu" >
                    { temp.map((item: MenuType) => (
                        <Menu.Item key={item.value ?? item.key} data-mode={item.mode}
                            data-dt={JSON.stringify({ showKey: item.showKey, key: item.key, showValue: item.showValue, value: item.value, isTrigger: item.isTrigger })}
                            data-child={ item.mode !== 'input' && item?.children?.length === 1 ? JSON.stringify(item.children[0]) : '' }
                            data-options={item.mode === 'value' || item.mode === 'keyValue' ? JSON.stringify(options) : 'null'}>
                            {item.showValue ?? item.showKey}</Menu.Item>
                    ))}
                </Menu>
                : <Menu></Menu>,
            isDisableMenu,
            curInput,
        };
    }, [ menuList, result ]);
    return menuArr;
};

const useData = (session: Session, menuList: MenuType[]): UseDataType => {
    const [ paginationData, updatePaginationData ] = useState({ current: 0, total: 0 });
    const [ searchContent, setSearchContent ] = useState('');
    const [ result, setResult ] = useState<ResultType[]>([]);
    const [ visible, setVisible ] = useState({ icon: true, input: false });
    const inputRef = useRef<InputRef>(null);
    // 保证请求结果数量和请求跳转的时间是一致的
    const [ time, setTime ] = useState({ startTimeAll: 0, endTimeAll: 0, startRecordTime: 0 });

    useEffect(() => {
        setVisible({ ...visible, icon: true });
        setResult([]);
        setSearchContent('');
        updatePaginationData({ current: 0, total: 0 });
    }, [session]);

    useEffect(() => { visible.input && inputRef.current?.focus(); }, [visible.input]);

    const clickItem = ({ item, domEvent: e }: {item: React.ReactInstance & {props?: Record<string, unknown>}; domEvent: DomEventType }): void => {
        if (item?.props === undefined) { return; }
        const props = item.props;
        const propsDt = JSON.parse(props['data-dt']);
        const mode = props['data-mode'];
        const options = JSON.parse(props['data-options']);
        let updateResult;
        if (mode !== 'value') {
            const current = { ...propsDt, mode: mode === 'input' ? 'input' : 'keyValue' };
            (mode === 'key' || mode === 'input') && Object.assign(current, { value: '', showValue: '' });
            mode === 'keyValue' && Object.assign(current, { options });
            updateResult = [ ...result, current ];
        } else {
            updateResult = [ ...result.slice(0, -1), { ...result[result.length - 1], ...propsDt, options } ];
        }
        if (props['data-child'] !== '') {
            updateResult.push(Object.assign(JSON.parse(props['data-child']), { value: '', showValue: '' }));
        }
        setResult(updateResult);
        modifyInput = false;
        e.stopPropagation();
    };

    const { MenuJSX, isDisableMenu, curInput } = useMenu({ menuList, result, visible, setVisible, clickItem });
    return { result, setResult, visible, setVisible, paginationData, updatePaginationData, searchContent, setSearchContent, time, setTime, MenuJSX, isDisableMenu, curInput, inputRef };
};

// 选择下拉列表的标签选项
const selectResultItem = ({ curValue, option, result, setResult, visible, setVisible, setSearchContent }: SelectResultItemType): void => {
    if (curValue === option.value) { return; }
    const updateResult = [];
    for (let i = 0; i < result.length; i++) {
        if (result[i].value === curValue) {
            updateResult.push({ ...result[i], value: option.value, showValue: option.label });
            break;
        }
        updateResult.push({ ...result[i] });
    }
    setResult(updateResult);
    setVisible({ ...visible, icon: true });
    setSearchContent('');
    modifyInput = false;
};

// 当前状态是否是修改模式, 修改模式下修改完不会自动生成下一个item的输入框，因为有可能修改的是中间的input选项
let modifyInput: boolean = false;

// 点击输入框的标签
const clickResultItem = (item: ResultType, result: ResultType[], setResult: Dispatch<SetStateAction<ResultType[]>>, setSearchContent: React.Dispatch<React.SetStateAction<string>>): void => {
    const { key, value } = item;
    if (result.some(item => item.value === '') || modifyInput) { return; }
    const updateResult = [];
    for (let i = 0; i < result.length; i++) {
        if (key === result[i].key && value === result[i].value) {
            setSearchContent(String(value));
            updateResult.push({ ...result[i], value: '', showValue: '' });
            modifyInput = true;
            continue;
        }
        if (!modifyInput || (modifyInput && result[i].mode === 'input')) {
            updateResult.push({ ...result[i] });
        }
    }
    setResult(updateResult);
};

// 生成已选列表的标签
const useChooseResult = ({ session, result, setResult, visible, setVisible, setTime, updatePaginationData, setSearchContent }: ChooseResultType): JSX.Element => {
    return useMemo(() => (
        <ul className="chooseResult">
            {
                result.map((item: ResultType, index, arr) => {
                    if (index === arr.length - 1 && item.isTrigger && !arr.some(item => item.value === '')) {
                        // 检查是否该触发搜索
                        searchCount(session, result, setTime, setVisible, updatePaginationData);
                    }
                    return <li key={item.key}>
                        {
                            item.mode === 'input'
                                ? <div onClick={() => clickResultItem(item, result, setResult, setSearchContent)} style={{ cursor: arr.some(item => item.value === '') ? 'default' : 'pointer' }}>{`${item.showKey} : ${item.showValue}`}</div>
                                : <Select options={item.options} value={item.showKey === undefined ? `${item.showValue}` : `${item.showKey} : ${item.showValue}`}
                                    getPopupContainer={trigger => trigger.parentNode} bordered={false} disabled={ item?.options === undefined }
                                    onSelect={(value: string, option: SelectOptionType) => selectResultItem({ curValue: item.value as string, option, result, setResult, visible, setVisible, setSearchContent })} />
                        }
                        { <CloseIcon className="icon" style={{ cursor: 'pointer' }}
                            onClick={() => { setResult(result.slice(0, index)); setVisible({ ...visible, icon: true }); modifyInput = false; (!(item.mode === 'input' && item.value !== '')) && setSearchContent(''); }} /> }
                    </li>;
                })
            }
        </ul>
    ), [result]);
};

const useSearchConfig = (session: Session): Array<MenuType | undefined> => {
    const menuList = useMemo(() => {
        const when = (unit: InsightUnit): boolean => true;
        // pinned-by-move-units should be excluded in the result
        const exclude = (unit: InsightUnit): boolean => unit.pinType === 'move' && isPinned(unit);
        const bypass = (unit: InsightUnit): boolean => unit.type === 'transparent';
        const flattenUnits = preOrderFlatten(session.units, 0, { bypass, when, exclude });
        const unitSet = new Set<Function>();

        return flattenUnits.map(unit => {
            if (unit.searchConfig === undefined || unitSet.has(unit.constructor)) {
                return undefined;
            }
            if (unit.searchConfig.key !== 'unit') {
                Logger('CategorySearch', `lane ${unit.name}'s searchConfig configurate error, name field is unit?`);
                return undefined;
            }
            unitSet.add(unit.constructor);
            return unit.searchConfig;
        }).filter(it => it !== undefined);
    }, [ session.phase, session.units ]);
    return menuList ?? [];
};

const onPageChange = (session: Session, current: number, result: ResultType[],
    time: { startTimeAll: number; endTimeAll: number; startRecordTime: number },
    updatePaginationData: React.Dispatch<React.SetStateAction<{ current: number; total: number }>>): void => {
    updatePaginationData(prevState => ({ current, total: prevState.total }));
    try {
        jumpSlice(session, result, time, current);
    } catch {
        Logger('CategorySearchContent', 'invoke jumpSlice occurred an exception');
    }
};

// 输入框的回车事件
const onInputPressEnter = ({ searchContent, setSearchContent, result, setResult, curInput }: PressEnterType): void => {
    if (searchContent.trim() === '' || curInput === undefined || (curInput.type === 'number' && isNaN(Number(searchContent)))) { return; }
    platform.trace('searchData', {});
    const updateResult: ResultType[] = [];
    const content = curInput.type === 'number' ? Number(searchContent) : searchContent.trim();
    setSearchContent('');
    let extraAddRes;
    if (curInput?.children?.length === 1 && curInput.children[0].mode === 'input' && !modifyInput) {
        // 输入框的children一定是下一个输入框，那么就自动加入到列表，等待用户继续输入
        const child = curInput.children[0];
        extraAddRes = { value: '', key: child.key as string, showKey: child.showKey, showValue: '', mode: child.mode, isTrigger: child.isTrigger ?? false } as ResultType;
    }
    for (let i = 0; i < result.length; i++) {
        if (result[i].mode === 'input' && result[i].value === '') {
            updateResult.push({ ...result[i], value: content, showValue: String(content) });
            continue;
        }
        updateResult.push(result[i]);
    }
    extraAddRes !== undefined && updateResult.push(extraAddRes);
    setResult(updateResult);
    modifyInput = false;
};

const CategorySearchContent = (session: Session, menuList: MenuType[]): JSX.Element => {
    const theme = useTheme();
    const {
        result, setResult, visible, setVisible, paginationData, updatePaginationData, searchContent,
        setSearchContent, time, setTime, MenuJSX, isDisableMenu, curInput, inputRef,
    } = useData(session, menuList);

    const onInputChange = (e: ChangeEvent<HTMLInputElement>): void => {
        const inputContent = e.target.value;
        setSearchContent(inputContent);
        if (!visible.icon) {
            setVisible({ ...visible, icon: true });
        }
    };

    return (
        <CustomDiv theme={theme} isDisableMenu={isDisableMenu} onClick={(e) => { e.preventDefault(); e.stopPropagation(); }}
            onMouseMove={(e) => { e.stopPropagation(); }}>
            { useChooseResult({ session, result, setResult, visible, setVisible, setTime, updatePaginationData, setSearchContent }) }
            <Dropdown overlay={ MenuJSX } trigger={['click']} disabled={isDisableMenu} getPopupContainer={trigger => trigger} >
                <div className="popup"></div>
            </Dropdown>
            <StyledInput allowClear={{ clearIcon: <CloseIcon fill={theme.buttonColor.enableClickColor}/> }} ref={inputRef}
                minwidth={200} height={24} isshow={visible.input ? 1 : 0} type={curInput?.type === 'number' ? 'number' : 'text'}
                value={searchContent} onChange={onInputChange} placeholder={`Involves ${curInput?.showKey}`}
                onPressEnter={() => onInputPressEnter({ searchContent, setSearchContent, result, setResult, curInput })} >
            </StyledInput>
            <div className="searchResult">{ visible.icon
                ? <CustomButton icon={SearchIcon} onClick={() => onInputPressEnter({ searchContent, setSearchContent, result, setResult, curInput })}></CustomButton>
                : <StylePagination defaultCurrent={0} pageSize={1} { ...paginationData } onChange={(current: number) => onPageChange(session, current, result, time, updatePaginationData)} simple/> }
            </div>
        </CustomDiv>
    );
};

export const CategorySearch = observer(({ session }: { session: Session}): JSX.Element | null => {
    const menuList = useSearchConfig(session);
    const theme = useTheme();
    const [ customButtonProps, updateCustomButtonProps ] = useState({
        isEmphasize: false,
        isSuspend: false,
        icon: SearchIcon,
    });
    // 监听session.phase变化控制搜索按钮显示
    useEffect(() => {
        updateCustomButtonProps({ ...customButtonProps, isSuspend: false });
    }, [ session, session.phase ]);
    // tooltip显隐控制悬浮效果
    const onTooltipVisibleChange = (visible: boolean): void => {
        updateCustomButtonProps({ ...customButtonProps, isSuspend: visible });
    };
    // ctrl+f/F调出全局搜索输入框
    const ref = useRef<HTMLButtonElement>(null);
    const searchShortCutHandler = (): void => {
        ref.current?.click();
    };
    useEventBus(EventType.GLOBALSEARCH, searchShortCutHandler as EventHandler<unknown>);
    return (
        <Tooltip overlayStyle={{ maxWidth: 1000 }}
            title={CategorySearchContent(session, menuList as MenuType[])}
            trigger="click"
            placement="right"
            onVisibleChange={onTooltipVisibleChange}
            color={theme.tooltipBGColor}
            overlayInnerStyle={{ color: theme.tooltipFontColor, padding: 0, borderRadius: 20 }}
            overlayClassName={'insight-category-search-overlay'}
            align={{ offset: [ -8, 3 ] }}>
            <CustomButton { ...customButtonProps } ref={ref}/>
        </Tooltip>
    );
});
