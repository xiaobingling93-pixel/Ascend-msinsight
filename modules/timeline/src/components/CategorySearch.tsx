import { useTheme } from '@emotion/react';
import styled from '@emotion/styled';
import { Tooltip, message, Button, Input } from 'antd';
import { observer } from 'mobx-react';
import React, { ChangeEvent, useEffect, useState } from 'react';
import { ReactComponent as AntdSearchIcon } from '../assets/images/insights/ic_search_lined.svg';
import { ReactComponent as AntdCloseIcon } from '../assets/images/insights/ic_close_filled.svg';
import { Session } from '../entity/session';
import { CustomButton } from './base/StyledButton';
import { StyledInput } from './base/StyledInput';
import { SvgType } from './base/rc-table/types';
import { action, runInAction } from 'mobx';
import { ThreadUnit } from '../insight/units/AscendUnit';
import i18n from 'i18next';
import { LeftOutlined, RightOutlined } from '@ant-design/icons';

const SearchIcon = AntdSearchIcon as SvgType;
const CloseIcon = AntdCloseIcon as SvgType;

const CustomDiv = styled.div`
    display: flex;
    align-items: center;
    justify-content: space-between;
    border-radius: 18px;
    padding: 1px 7px 1px 10px;
    min-width: 600px;
    height: 32px;
    background: ${(props): string => props.theme.tooltipBGColor};
    .searchResult {
        color: ${(props): string => props.theme.svgBackgroundColor};
        font-size: 12px;
        white-space: nowrap;
    }
    button.ant-btn.ant-btn-default.ant-btn-icon-only {
        border: none;
        background-color: ${(props): string => props.theme.tooltipBGColor};
        color: ${(props): string => props.theme.svgBackgroundColor};
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
`;

type RankCount = {
    rankId: string;
    count: number;
};

type RemoteCount = {
    dataSource: DataSource;
    countList: RankCount[];
};

type SliceData = {
    rankId: string;
    pid: string;
    tid: number;
    startTime: number;
    duration: number;
    depth: number;
};

let remoteCntArray: RemoteCount[] = [];

// 获取搜索的结果数量
const queryDataCount = async (session: Session, searchContent: string): Promise<number> => {
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
        const res = await window.request(metadata.dataSource, { command: 'search/count', params: { rankId: metadata.cardId, searchContent } });
        if (res.totalCount === 0) {
            continue;
        }
        totalCnt += res.totalCount;
        remoteCntArray.push({ dataSource: metadata.dataSource, countList: res.countList });
    }
    return totalCnt;
};

// 跳转函数
const jumpSlice = async (session: Session, searchContent: string, index: number): Promise<void> => {
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
    const slice: SliceData = await window.request(finalDataSource as DataSource, { command: 'search/slice', params: { rankId: finalRankId, searchContent, index: Math.max(1, currentIndex) } });
    doJumpSlice(session, slice, currentIndex === 0);
};

const doJumpSlice = (session: Session, slice: SliceData, isGlobal: boolean): void => {
    if (slice === undefined) {
        console.error('slice is undefined.');
    }
    runInAction(() => {
        session.locateUnit = {
            target: (unit) => {
                return unit instanceof ThreadUnit && (Boolean(unit.metadata.cardId.includes(slice.rankId))) && unit.metadata.processId === slice.pid && unit.metadata.threadId === slice.tid;
            },
            onSuccess: (unit) => {
                if (isGlobal) {
                    session.domainRange = { domainStart: 0, domainEnd: session.endTimeAll ?? session.domain.defaultDuration };
                    session.selectedData = undefined;
                } else {
                    const [rangeStart, rangeEnd] = calculateDomainRange(session, slice.startTime, slice.duration);
                    session.domainRange = { domainStart: rangeStart, domainEnd: rangeEnd };
                    session.selectedData = { startTime: slice.startTime, duration: slice.duration, depth: slice.depth, threadId: slice.tid };
                }
            },
        };
    });
};

export const calculateDomainRange = (session: Session, startTime: number, duration: number): [ number, number ] => {
    let rangeStart = startTime - duration * 9;
    rangeStart = rangeStart > 0 ? rangeStart : 0;
    const rangeEnd = Math.min(startTime + duration * 10, session.endTimeAll ?? Number.MAX_SAFE_INTEGER);
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

const CategorySearchContent = (session: Session): JSX.Element => {
    const [messageApi, contextHolder] = message.useMessage();
    const theme = useTheme();
    const [paginationData, updatePaginationData] = useState({ current: 0, total: 0 });
    const [searchIconVisible, setSearchIconVisible] = useState(true);
    const [searchContent, setSearchContent] = useState('');
    const [searchingStatus, setSearchingStatus] = useState(false);

    useEffect(action(() => {
        setSearchIconVisible(true); setSearchContent('');
        updatePaginationData({ current: 0, total: 0 }); session.searchData = undefined;
    }), [session]);
    const onPageChange = (current: number): void => {
        updatePaginationData(prevState => ({ current, total: prevState.total }));
        jumpSlice(session, searchContent, current);
    };
    const onInputPressEnter = async (): Promise<void> => {
        if (searchContent === '') { return; }
        setSearchingStatus(true);
        const totalCnt = await queryDataCount(session, searchContent);
        if (totalCnt > 0) {
            updatePaginationData({ current: 0, total: totalCnt });
            jumpSlice(session, searchContent, 0);
            setSearchIconVisible(false);
        } else { messageApi.warning(i18n.t('notify:SearchEmpty')); }
        setSearchingStatus(false);
    };
    const onInputChange = action((e: ChangeEvent<HTMLInputElement>): void => {
        const inputContent = e.target.value;
        setSearchContent(inputContent);
        setSearchIconVisible(true);
        session.searchData = { content: inputContent };
    });

    return (
        <CustomDiv theme={theme} onClick={(e) => { e.stopPropagation(); }}>
            { contextHolder}
            <StyledInput allowClear={{ clearIcon: <CloseIcon fill={theme.buttonColor.enableClickColor} /> }} disabled={searchingStatus} maxLength={200}
                minwidth={200} height={24} isshow={1} value={searchContent} onChange={onInputChange} onPressEnter={onInputPressEnter} ></StyledInput>
            <div className="searchResult">{searchingStatus
                ? <ImgWithFallback className={'loading'} />
                : searchIconVisible
                    ? <CustomButton icon={SearchIcon} onClick={onInputPressEnter}></CustomButton>
                    : <StylePagination {...paginationData} onChange={onPageChange} /> }
            </div>
        </CustomDiv>
    );
};

export const CategorySearch = observer(({ session }: { session: Session}): JSX.Element | null => {
    const theme = useTheme();
    const [customButtonProps, updateCustomButtonProps] = useState({
        isEmphasize: false,
        isDisabled: false,
        isSuspend: false,
        icon: SearchIcon,
    });
    const searchDataRef = React.useRef<Session['searchData']>();
    // tooltip显隐控制悬浮效果
    const onTooltipVisibleChange = (visible: boolean): void => {
        updateCustomButtonProps({ ...customButtonProps, isSuspend: visible });
        if (visible) {
            runInAction(() => { session.searchData = searchDataRef.current; });
        } else {
            searchDataRef.current = session.searchData;
            session.searchData = undefined;
        }
    };
    return (
        <Tooltip overlayStyle={{ maxWidth: 1000 }}
            title={CategorySearchContent(session)}
            trigger="click"
            placement="right"
            onVisibleChange={onTooltipVisibleChange}
            color={theme.tooltipBGColor}
            overlayInnerStyle={{ color: theme.tooltipFontColor, padding: 0, borderRadius: 20 }}
            overlayClassName={'insight-category-search-overlay'}
            align={{ offset: [-8, 3] }}>
            <CustomButton tooltip={i18n.t('tooltip:search')} { ...customButtonProps }/>
        </Tooltip>
    );
});

interface Props {
    onChange: (current: number) => void;
    current: number;
    total: number;
}
const StylePagination = ({ onChange, current, total }: Props): JSX.Element => {
    const [searchNumber, setSearchNumber] = useState(0);
    const [currentValue, setCurrentValue] = useState<string | number>(current);
    const handleSearch = (inputNumber: number): void => {
        setCurrentValue(inputNumber);
        onChange(inputNumber);
    };
    useEffect(() => {
        setCurrentValue(current);
    }, [current]);
    return (<div className={'StylePaginationClass'}>
        <Button size="middle" disabled={current === 0} icon={<LeftOutlined />} onClick={(): void => onChange(current - 1) }/>
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
                if (Number(val) > total || Number(val) < 0) {
                    return;
                }
                setCurrentValue(Number(val));
                setSearchNumber(Number(val));
            }}
            onPressEnter={(): void => handleSearch(searchNumber)}
        /></span> / <span>{total}</span>
        <Button size="middle" disabled={current === total} icon={<RightOutlined />} onClick={(): void => onChange(current + 1) }/>
    </div>);
};
