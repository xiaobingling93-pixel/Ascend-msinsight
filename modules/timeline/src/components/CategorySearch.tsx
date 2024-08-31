/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */
import { useTheme } from '@emotion/react';
import styled from '@emotion/styled';
import { Tooltip, Button, Input } from 'ascend-components';
import { message } from 'antd';
import { observer } from 'mobx-react';
import React, { type ChangeEvent, useEffect, useState } from 'react';
import { SearchIcon } from 'ascend-icon';
import { ReactComponent as AntdCloseIcon } from '../assets/images/insights/ic_close_filled.svg';
import type { Session } from '../entity/session';
import { CustomButton, StyledButton } from './base/StyledButton';
import type { SvgType } from './base/rc-table/types';
import { action, runInAction } from 'mobx';
import { ThreadUnit } from '../insight/units/AscendUnit';
import { useTranslation } from 'react-i18next';
import { LeftOutlined, RightOutlined } from '@ant-design/icons';
import type { ThreadMetaData } from '../entity/data';
import { generateFlowParam } from '../insight/units/details';
import { getTimeOffset } from '../insight/units/utils';

const CloseIcon = AntdCloseIcon as SvgType;

const RANGE_MULTIPLE = 10;

const CustomDiv = styled.div`
    display: flex;
    align-items: center;
    justify-content: space-between;
    border-radius: 18px;
    padding: 1px 7px 1px 10px;
    min-width: 300px;
    height: 32px;
    background: ${(props): string => props.theme.bgColorLight};
    .searchResult {
        font-size: 12px;
        white-space: nowrap;
    }
    button.ant-btn.ant-btn-default.ant-btn-icon-only {
        border: none;
        background-color: ${(props): string => props.theme.bgColorLight};
        color: ${(props): string => props.theme.textColorPrimary};
    }
    button.ant-btn.ant-btn-default.ant-btn-icon-only:hover {
        color: #007aff;
    }
    input.ant-input.ant-input-sm {
        width: 50px;
        border-radius: 5px;
        height: 22px;
        font-size: 12px;
    }
    button.ant-btn.ant-btn-default {
        font-size: 12px;
    }
`;

const SearchContainer = styled.div`
    display: flex;
    align-items: center;
    margin-left: 10px;
`;

interface RankCount {
    rankId: string;
    count: number;
};

interface RemoteCount {
    dataSource: DataSource;
    countList: RankCount[];
};

interface SliceData {
    rankId: string;
    pid: string;
    tid: number;
    id?: string;
    startTime: number;
    duration: number;
    depth: number;
};

let remoteCntArray: RemoteCount[] = [];

// 获取搜索的结果数量
const queryDataCount = async (session: Session, searchContent: string, isMatchCase: boolean, isMatchExact: boolean): Promise<number> => {
    if (searchContent === undefined || searchContent === '') {
        return 0;
    }
    let totalCnt = 0;
    remoteCntArray = [];
    for (const unit of session.units) {
        if (!unit.isDisplay) {
            continue;
        }
        const metadata = unit.metadata as any;
        const res = await window.request(metadata.dataSource, { command: 'search/count', params: { rankId: metadata.cardId, searchContent, isMatchCase, isMatchExact } });
        if (res.totalCount === 0) {
            continue;
        }
        totalCnt += res.totalCount;
        remoteCntArray.push({ dataSource: metadata.dataSource, countList: res.countList });
    }
    return totalCnt;
};

// 跳转函数
const jumpSlice = async (session: Session, searchContent: string, index: number, isMatchCase: boolean, isMatchExact: boolean): Promise<void> => {
    let finalDataSource;
    let finalRankId;
    let flag = false;
    let currentIndex = index;
    for (const remoteCount of remoteCntArray) {
        if (flag) {
            break;
        }
        for (const rankCount of remoteCount.countList) {
            if (currentIndex <= rankCount.count) {
                finalRankId = rankCount.rankId;
                finalDataSource = remoteCount.dataSource;
                flag = true;
                break;
            }
            currentIndex -= rankCount.count;
        }
    }
    const slice: SliceData = await window.request(finalDataSource as DataSource, { command: 'search/slice', params: { rankId: finalRankId, searchContent, index: Math.max(1, currentIndex), isMatchCase, isMatchExact } });
    doJumpSlice(session, slice, currentIndex === 0);
};

const doJumpSlice = (session: Session, slice: SliceData, isGlobal: boolean): void => {
    if (slice === undefined) {
        // slice is undefined.
        return;
    }
    runInAction(() => {
        session.locateUnit = {
            target: (unit): boolean => {
                return unit instanceof ThreadUnit && (Boolean(unit.metadata.cardId.includes(slice.rankId))) &&
                    unit.metadata.processId === slice.pid && unit.metadata.threadId === slice.tid;
            },
            onSuccess: (unit): void => {
                if (isGlobal) {
                    session.domainRange = { domainStart: 0, domainEnd: session.endTimeAll ?? session.domain.defaultDuration };
                    session.selectedData = undefined;
                    session.linkFlow = undefined;
                } else {
                    const [rangeStart, rangeEnd] = calculateDomainRange(session,
                        slice.startTime - getTimeOffset(session, (unit.metadata as ThreadMetaData).cardId), slice.duration);
                    session.domainRange = { domainStart: rangeStart, domainEnd: rangeEnd };
                    session.selectedData = {
                        startTime: slice.startTime - getTimeOffset(session, (unit.metadata as ThreadMetaData).cardId),
                        duration: slice.duration,
                        depth: slice.depth,
                        threadId: slice.tid,
                        id: slice.id,
                        metaType: (unit.metadata as ThreadMetaData).metaType,
                    };
                    session.linkFlow = generateFlowParam(unit.metadata as ThreadMetaData, slice);
                }
            },
        };
    });
};

export const calculateDomainRange = (session: Session, startTime: number, duration: number): [ number, number ] => {
    const range = duration === 0 ? 1 : duration;
    let rangeStart = startTime - (range * (RANGE_MULTIPLE - 1));
    rangeStart = rangeStart > 0 ? rangeStart : 0;
    const rangeEnd = Math.min(startTime + (range * RANGE_MULTIPLE), session.endTimeAll ?? Number.MAX_SAFE_INTEGER);
    return [rangeStart, rangeEnd];
};

const ImgWithFallback = ({ className = '' }): JSX.Element => {
    const theme = useTheme();
    const PictureContainer = styled.picture`
        display: block;
        width: 25px;
    `;
    return (
        <PictureContainer>
            <div className={className} style={{
                borderColor: theme.buttonColor.enableClickColor,
                borderTopColor: 'transparent',
            }}></div>
        </PictureContainer>
    );
};

// eslint-disable-next-line max-lines-per-function
const CategorySearchContent = (session: Session): JSX.Element => {
    const { t } = useTranslation();
    const [messageApi, contextHolder] = message.useMessage();
    const theme = useTheme();
    // paginationData记录了当前搜索的算子的下标以及一共搜索到多少个算子、默认搜索第1个算子
    const [paginationData, updatePaginationData] = useState({ current: 1, total: 0 });
    const [searchIconVisible, setSearchIconVisible] = useState(true);
    const [searchContent, setSearchContent] = useState('');
    const [searchingStatus, setSearchingStatus] = useState(false);
    const [isMatchCase, setIsMatchCase] = useState(false);
    const [isMatchExact, setIsMatchExact] = useState(false);

    useEffect(action(() => {
        setSearchIconVisible(true); setSearchContent(''); setIsMatchCase(false); setIsMatchExact(false);
        updatePaginationData({ current: 1, total: 0 }); session.searchData = undefined;
    }), [session, session.units]);
    const onPageChange = (current: number): void => {
        updatePaginationData(prevState => ({ current, total: prevState.total }));
        jumpSlice(session, searchContent, current, isMatchCase, isMatchExact);
    };
    const onInputPressEnter = async (): Promise<void> => {
        if (searchContent === '') { return; }
        setSearchingStatus(true);
        const totalCnt = await queryDataCount(session, searchContent, isMatchCase, isMatchExact);
        if (totalCnt > 0) {
            updatePaginationData({ current: 1, total: totalCnt });
            jumpSlice(session, searchContent, 1, isMatchCase, isMatchExact);
            setSearchIconVisible(false);
        } else {
            messageApi.warning(t('notify:SearchEmpty'));
        }
        setSearchingStatus(false);
    };
    const onInputChange = action((e: ChangeEvent<HTMLInputElement>): void => {
        const inputContent = e.target.value;
        setSearchContent(inputContent);
        setSearchIconVisible(true);
        session.searchData = { content: inputContent, isMatchCase, isMatchExact };
    });

    function changeMatchCaseStatus(): void {
        const status = !isMatchCase;
        setIsMatchCase(status);
        runInAction(() => {
            if (session.searchData !== null && session.searchData !== undefined) {
                session.searchData.isMatchCase = status;
            }
        });
    }

    function changeMatchExactStatus(): void {
        const status = !isMatchExact;
        setIsMatchExact(status);
        runInAction(() => {
            if (session.searchData !== null && session.searchData !== undefined) {
                session.searchData.isMatchExact = status;
            }
        });
    }
    const doSearchList = (): void => {
        runInAction(() => {
            if (session.searchData !== null && session.searchData !== undefined) {
                session.searchData = { ...session.searchData, content: searchContent, isMatchCase, isMatchExact };
            }
            session.doContextSearch = true;
        });
        setTimeout(() => {
            runInAction(() => {
                session.doContextSearch = false;
            });
        });
    };

    let dom;
    if (searchingStatus) {
        dom = <ImgWithFallback className={'loading'} />;
    } else {
        if (searchIconVisible) {
            dom = <SearchContainer>
                <Tooltip title={t('Match case', { ns: 'tooltip' })}>
                    <StyledButton icon={isMatchCase
                        ? <div className={'icon_selected_case_match'}/>
                        : <div className={'icon_case_match'}/>}
                    onClick={(): void => changeMatchCaseStatus()}></StyledButton>
                </Tooltip>
                <Tooltip title={t('Words', { ns: 'tooltip' })}>
                    <StyledButton icon={isMatchExact
                        ? <div className={'icon_selected_exact_match'}/>
                        : <div className={'icon_exact_match'}/>} onClick={(): void => changeMatchExactStatus()}></StyledButton>
                </Tooltip>
                <CustomButton icon={SearchIcon as any} onClick={onInputPressEnter}></CustomButton>
            </SearchContainer>;
        } else {
            dom = <SearchContainer>
                <StylePagination {...paginationData} onChange={onPageChange} />
                <Button type={'primary'} size="small" onClick={doSearchList}>{t('Open in Find Window', { ns: 'buttonText' })}</Button>
            </SearchContainer>;
        }
    }

    return (
        <CustomDiv theme={theme} onClick={(e): void => { e.stopPropagation(); }}>
            { contextHolder}
            <Input allowClear={{ clearIcon: <CloseIcon fill={theme.buttonColor.enableClickColor} /> }} disabled={searchingStatus} maxLength={200}
                size="large" value={searchContent} onChange={onInputChange} onPressEnter={onInputPressEnter} ></Input>
            <div className="searchResult">
                {dom}
            </div>
        </CustomDiv>
    );
};

export const CategorySearch = observer(({ session }: { session: Session}): JSX.Element | null => {
    const { t } = useTranslation();
    const [customButtonProps, updateCustomButtonProps] = useState({
        isEmphasize: false,
        isDisabled: false,
        isSuspend: false,
    });
    const searchDataRef = React.useRef<Session['searchData']>();
    // tooltip显隐控制悬浮效果
    const onTooltipVisibleChange = (visible: boolean): void => {
        updateCustomButtonProps({ ...customButtonProps, isSuspend: visible });
        if (visible) {
            runInAction(() => { session.searchData = searchDataRef.current; });
        } else {
            searchDataRef.current = session.searchData;
        }
    };
    useEffect(() => {
        searchDataRef.current = undefined;
    }, [session.doReset]);

    return (
        <Tooltip overlayStyle={{ maxWidth: 1000 }}
            overlayInnerStyle={{ maxWidth: 800, borderRadius: 2 }}
            title={CategorySearchContent(session)}
            trigger="click"
            onOpenChange={onTooltipVisibleChange}
            overlayClassName={'insight-category-search-overlay'}
            align={{ offset: [-8, 3] }}>
            <CustomButton tooltip={t('tooltip:search')} icon={SearchIcon as any} { ...customButtonProps }/>
        </Tooltip>
    );
});

interface Props {
    onChange: (current: number) => void;
    current: number;
    total: number;
}
const StylePagination = ({ onChange, current, total }: Props): JSX.Element => {
    const [searchNumber, setSearchNumber] = useState(1);
    const [currentValue, setCurrentValue] = useState<string | number>(current);
    const handleSearch = (inputNumber: number): void => {
        setCurrentValue(inputNumber);
        onChange(inputNumber);
    };
    useEffect(() => {
        setCurrentValue(current);
    }, [current]);
    return (<div>
        <Button size="middle" disabled={current === 1} icon={<LeftOutlined />} style={{ minWidth: 'auto' }} onClick={(): void => onChange(current - 1) }/>
        <span><Input
            size="small"
            value={currentValue}
            onChange={(event: ChangeEvent<HTMLInputElement>): void => {
                const val = event.target.value.replace(/\D/g, '');
                if (val === '') {
                    setCurrentValue(val);
                    setSearchNumber(1);
                    return;
                }
                if (Number(val) > total || Number(val) < 1) {
                    return;
                }
                setCurrentValue(Number(val));
                setSearchNumber(Number(val));
            }}
            onPressEnter={(): void => handleSearch(searchNumber)}
        /></span> / <span>{total}</span>
        <Button size="middle" disabled={current === total} icon={<RightOutlined />} style={{ minWidth: 'auto' }} onClick={(): void => onChange(current + 1) }/>
    </div>);
};
