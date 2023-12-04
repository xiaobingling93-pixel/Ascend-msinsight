import { useTheme } from '@emotion/react';
import styled from '@emotion/styled';
import { Tooltip } from 'antd';
import { observer } from 'mobx-react';
import React, { useRef, useState } from 'react';
import { ReactComponent as AntdFilterIcon } from '../assets/images/insights/FunnelIcon.svg';
import { Session } from '../entity/session';
import { CustomButton, StyledButton } from './base/StyledButton';
import { SvgType } from './base/rc-table/types';
import i18n from 'i18next';
import { StyledCheckbox } from './base/StyledCheckbox';
import { StyledEmpty } from './base/StyledEmpty';
import { action, runInAction } from 'mobx';
import { InsightUnit, LinkLines } from '../entity/insight';
import { CardUnit } from '../insight/units/AscendUnit';

const FilterIcon = AntdFilterIcon as SvgType;
const MAX_HEIGHT = 200;
const PADDING_RATIO_TO_MAX_HEIGHT = 0.1;
const FilterContainer = styled.div`
    display: flex;
    flex-direction: column;
    width: 200px;
    max-height: ${MAX_HEIGHT}px;
    padding: ${MAX_HEIGHT * PADDING_RATIO_TO_MAX_HEIGHT}px ${MAX_HEIGHT * PADDING_RATIO_TO_MAX_HEIGHT}px 7px ${MAX_HEIGHT * PADDING_RATIO_TO_MAX_HEIGHT}px;
`;

const FilterList = styled.div`
    max-height: ${MAX_HEIGHT - MAX_HEIGHT * 2 * PADDING_RATIO_TO_MAX_HEIGHT}px;
    overflow-y: scroll;
`;

const FilterButtonLine = styled.div`
    display: flex;
    justify-content: space-evenly;
    align-items: center;
    margin-top: 7px;
`;

const FilterItem = observer(({ session, category, fetchLinkLines }: { session: Session; category: string; fetchLinkLines: FetchLinkLines }) => {
    return (
        <p style={{ marginBottom: 0 }}>
            <StyledCheckbox
                checked={session.linkLines[category] !== undefined}
                onChange={async () => {
                    if (session.linkLines[category] !== undefined) {
                        runInAction(() => {
                            session.linkLines[category] = undefined;
                            session.linkLines = { ...session.linkLines };
                        });
                    } else {
                        const datas = await fetchLinkLines(session, category);
                        runInAction(() => {
                            session.linkLines[category] = datas;
                            session.linkLines = { ...session.linkLines };
                        });
                    }
                }}>
                {category}
            </StyledCheckbox>
        </p>
    );
});

const getCardUnits = (units: InsightUnit[]): InsightUnit[] => {
    return units.flatMap(unit => {
        const res: InsightUnit[] = [];
        if (unit instanceof CardUnit) {
            res.push(unit);
        };
        if (unit.children) {
            res.push(...getCardUnits(unit.children));
        }
        return res;
    });
};

export interface DataBlock {
    pid: number;
    tid: number;
    timestamp: number;
    depth: number;
    height?: number;
};
export interface FlowEvent {
    category: string;
    from: DataBlock;
    to: DataBlock;
    cardId: string;
};
interface CategoryEvents {
    flowDetailList: Array<{
        category: string;
        from: DataBlock;
        to: DataBlock;
    }>;
};

type FetchLinkLines = (session: Session, category: string) => Promise<CategoryEvents['flowDetailList']>;
const useFetchLinkLines = (session: Session): FetchLinkLines => {
    return React.useCallback(async (session: Session, category: string): Promise<CategoryEvents['flowDetailList']> => {
        const { domainStart, domainEnd } = session.domainRange;
        let res: CategoryEvents['flowDetailList'] = [];
        for (const unit of getCardUnits(session.units)) {
            const { dataSource, cardId } = unit.metadata as { dataSource: DataSource; cardId: string };
            res = res.concat((await window.request(dataSource,
                { command: 'flow/categoryEvents', params: { rankId: cardId, startTime: domainStart, endTime: domainEnd, category } }) as CategoryEvents).flowDetailList);
        };
        return res;
    }, [session]);
};

const useGetCategories = (session: Session, isSuspend: boolean): string[] => {
    const [ categories, setCategories ] = React.useState<string[]>([]);
    const unitsRef = React.useRef<string>();
    React.useEffect(() => {
        const cardUnits = getCardUnits(session.units);
        const cardUnitsString = JSON.stringify(cardUnits);
        if (!isSuspend || cardUnitsString === unitsRef.current) { return; }
        unitsRef.current = cardUnitsString;
        const fetchList: Array<Promise<{ category: string[] }>> = [];
        for (const unit of cardUnits) {
            const { dataSource, cardId } = unit.metadata as { dataSource: DataSource; cardId: string };
            fetchList.push(window.request(dataSource, { command: 'flow/categoryList', params: { rankId: cardId } }));
        }

        Promise.all(fetchList).then((results) => {
            const curCategories = new Set<string>();
            results.forEach(({ category }) => {
                category.forEach((cat) => {
                    curCategories.add(cat);
                });
            });
            setCategories([...curCategories]);
            runInAction(() => {
                curCategories.forEach((category) => {
                    session.linkLines[category] = undefined;
                });
            });
        });
    }, [isSuspend]);
    return categories;
};
const LinkLineFilterBody = observer(({ session, isSuspend }: { session: Session; isSuspend: boolean }): JSX.Element => {
    const fetchLinkLines = useFetchLinkLines(session);
    const categories = useGetCategories(session, isSuspend);

    const isEmptyData = categories.length === 0;
    return (
        <FilterContainer>
            <FilterList>
                {isEmptyData
                    ? <StyledEmpty />
                    : categories.map((category, index) => <FilterItem key={index} session={session} category={category} fetchLinkLines={fetchLinkLines} />)}
            </FilterList>
            {!isEmptyData && <FilterButtonLine>
                <StyledButton width={50} onClick={async () => {
                    const newLines: LinkLines = {};
                    for (const category of categories) {
                        const datas = await fetchLinkLines(session, category);
                        newLines[category] = datas;
                    }
                    runInAction(() => { session.linkLines = { ...newLines }; });
                }}>
                    All
                </StyledButton>
                <StyledButton width={50} onClick={action(() => {
                    const newLines: Record<string, undefined> = {};
                    categories.forEach(category => {
                        newLines[category] = undefined;
                    });
                    session.linkLines = { ...newLines };
                })}>
                    None
                </StyledButton>
            </FilterButtonLine>}
        </FilterContainer>
    );
});

export const FilterLinkLine = observer(({ session }: { session: Session}): JSX.Element | null => {
    const theme = useTheme();
    const [ customButtonProps, updateCustomButtonProps ] = useState({
        isEmphasize: false,
        isSuspend: false,
        icon: FilterIcon,
    });
    // tooltip显隐控制悬浮效果
    const onTooltipVisibleChange = (visible: boolean): void => {
        updateCustomButtonProps({ ...customButtonProps, isSuspend: visible });
    };
    const ref = useRef<HTMLButtonElement>(null);
    return (
        <Tooltip overlayStyle={{ maxWidth: 1000 }}
            title={<LinkLineFilterBody session={session} isSuspend={customButtonProps.isSuspend} />}
            trigger="click"
            placement="right"
            onVisibleChange={onTooltipVisibleChange}
            color={theme.tooltipBGColor}
            overlayInnerStyle={{ color: theme.tooltipFontColor, padding: 0, borderRadius: 20 }}
            overlayClassName={'insight-category-search-overlay'} align={{ offset: [ -8, 3 ] }}>
            <CustomButton tooltip={i18n.t('tooltip:filter')} { ...customButtonProps } ref={ref}/>
        </Tooltip>
    );
});
