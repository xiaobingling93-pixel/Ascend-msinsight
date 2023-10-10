import { useTheme } from '@emotion/react';
import styled from '@emotion/styled';
import { Pagination, Tooltip } from 'antd';
import { PaginationProps } from 'antd/lib/pagination';
import { observer } from 'mobx-react';
import React, { ChangeEvent, useEffect, useState } from 'react';
import { ReactComponent as AntdSearchIcon } from '../assets/images/insights/ic_search_lined.svg';
import { ReactComponent as AntdCloseIcon } from '../assets/images/insights/ic_close_filled.svg';
import { Session } from '../entity/session';
import { CustomButton } from './base/StyledButton';
import { StyledInput } from './base/StyledInput';
import { SvgType } from './base/rc-table/types';
import { runInAction } from 'mobx';
import { ThreadUnit } from '../insight/units/AscendUnit';

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
    background: ${props => props.theme.tooltipBGColor};
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
    for (const remoteCount of remoteCntArray) {
        if (flag) {
            break;
        }
        for (const rankCount of remoteCount.countList) {
            if (index <= rankCount.count) {
                finalRankId = rankCount.rankId;
                finalDataSource = remoteCount.dataSource;
                flag = true;
                break;
            }
            index -= rankCount.count;
        }
    }
    const slice: SliceData = await window.request(finalDataSource as DataSource, { command: 'search/slice', params: { rankId: finalRankId, searchContent, index } });
    doJumpSlice(session, slice);
};

const doJumpSlice = (session: Session, slice: SliceData): void => {
    if (slice === undefined) {
        console.error('slice is undefined.');
    }
    runInAction(() => {
        session.locateUnit = {
            target: (unit) => {
                return unit instanceof ThreadUnit && (Boolean(unit.metadata.cardId.includes(slice.rankId))) && unit.metadata.processId === slice.pid && unit.metadata.threadId === slice.tid;
            },
            onSuccess: (unit) => {
                const [ rangeStart, rangeEnd ] = calculateDomainRange(session, slice.startTime, slice.duration);
                session.domainRange = { domainStart: rangeStart, domainEnd: rangeEnd };
                session.selectedData = { startTime: slice.startTime, duration: slice.duration, depth: slice.depth, threadId: slice.tid };
            },
        };
    });
};

const calculateDomainRange = (session: Session, startTime: number, duration: number): [ number, number ] => {
    let rangeStart = startTime - duration * 9;
    rangeStart = rangeStart > 0 ? rangeStart : 0;
    const rangeEnd = Math.min(startTime + duration * 10, session.endTimeAll ?? Number.MAX_SAFE_INTEGER);
    return [ rangeStart, rangeEnd ];
};

const CategorySearchContent = (session: Session): JSX.Element => {
    const theme = useTheme();
    const [ paginationData, updatePaginationData ] = useState({ current: 0, total: 0 });
    const [ searchIconVisible, setSearchIconVisible ] = useState(true);
    const [ searchContent, setSearchContent ] = useState('');

    useEffect(() => {
        setSearchIconVisible(true);
        setSearchContent('');
        updatePaginationData({ current: 0, total: 0 });
    }, [session]);
    const onPageChange = (current: number, pageSize: number): void => {
        updatePaginationData(prevState => ({ current, total: prevState.total }));
        jumpSlice(session, searchContent, current);
    };
    const onInputPressEnter = async (): Promise<void> => {
        if (searchContent === '') {
            return;
        }
        const totalCnt = await queryDataCount(session, searchContent);
        updatePaginationData({ current: 1, total: totalCnt });
        onPageChange(1, totalCnt);
        setSearchIconVisible(false);
    };
    const onInputChange = (e: ChangeEvent<HTMLInputElement>): void => {
        const inputContent = e.target.value;
        setSearchContent(inputContent);
        setSearchIconVisible(true);
    };

    return (
        <CustomDiv theme={theme} onClick={(e) => { e.stopPropagation(); }}>
            <StyledInput allowClear={{ clearIcon: <CloseIcon fill={theme.buttonColor.enableClickColor}/> }}
                minwidth={200} height={24} isshow={1} value={searchContent} onChange={onInputChange} onPressEnter={onInputPressEnter}></StyledInput>
            <div className="searchResult">{ searchIconVisible
                ? <CustomButton icon={SearchIcon} onClick={onInputPressEnter}></CustomButton>
                : <StylePagination defaultCurrent={1} pageSize={1} { ...paginationData } onChange={onPageChange} simple/> }
            </div>
        </CustomDiv>
    );
};

export const CategorySearch = observer(({ session }: { session: Session}): JSX.Element | null => {
    const theme = useTheme();
    const [ customButtonProps, updateCustomButtonProps ] = useState({
        isEmphasize: false,
        isDisabled: false,
        isSuspend: false,
        icon: SearchIcon,
    });
    // tooltip显隐控制悬浮效果
    const onTooltipVisibleChange = (visible: boolean): void => {
        updateCustomButtonProps({ ...customButtonProps, isSuspend: visible });
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
            align={{ offset: [ -8, 3 ] }}>
            <CustomButton { ...customButtonProps }/>
        </Tooltip>
    );
});
