import { useTheme } from '@emotion/react';
import styled from '@emotion/styled';
import { Tooltip } from 'antd';
import { observer } from 'mobx-react';
import React, { useRef, useState } from 'react';
import { ReactComponent as AntdFilterIcon } from '../assets/images/insights/LinkerIcon.svg';
import { Session } from '../entity/session';
import { CustomButton, StyledButton } from './base/StyledButton';
import { SvgType } from './base/rc-table/types';
import i18n from 'i18next';
import { StyledCheckbox } from './base/StyledCheckbox';
import { StyledEmpty } from './base/StyledEmpty';
import { runInAction } from 'mobx';
import { InsightUnit, LinkLines } from '../entity/insight';
import { CardUnit } from '../insight/units/AscendUnit';
import { customDebounce } from '../utils/customDebounce';

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

interface FilterItemProps {
    category: string;
    checkedCategories: string[];
    setCheckedCategories: React.Dispatch<React.SetStateAction<string[]>>;
}
const FilterItem: React.FC<FilterItemProps> = observer(({ category, checkedCategories, setCheckedCategories }) => {
    const isChecked = checkedCategories.includes(category);
    return (
        <p style={{ marginBottom: 0 }}>
            <StyledCheckbox
                checked={isChecked}
                onChange={() => {
                    setCheckedCategories(prev => isChecked ? prev.filter(cat => cat !== category) : prev.concat(category));
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

type FetchLinkLines = (session: Session) => Promise<CategoryEvents['flowDetailList']>;
type UseFetchLinkLines = Map<string, FetchLinkLines>;
const useFetchLinkLines = (displayCategories: string[]): UseFetchLinkLines => React.useMemo(() => new Map(
    displayCategories.map(category => [
        category,
        customDebounce(async (session: Session): Promise<CategoryEvents['flowDetailList']> => {
            const { domainStart, domainEnd } = session.domainRange;
            const { domain: { timePerPx } } = session;
            let res: CategoryEvents['flowDetailList'] = [];
            for (const unit of getCardUnits(session.units)) {
                const { dataSource, cardId } = unit.metadata as { dataSource: DataSource; cardId: string };
                const params = { rankId: cardId, startTime: Math.floor(domainStart), endTime: Math.ceil(domainEnd), category, timePerPx };
                res = res.concat((await window.request(dataSource,
                    { command: 'flow/categoryEvents', params }) as CategoryEvents).flowDetailList
                    .map(data => ({ ...data, cardId })));
            };
            return res;
        }),
    ]),
), [displayCategories]);

const useGetCategories = (session: Session, isSuspend: boolean): string[] => {
    const [categories, setCategories] = React.useState<string[]>([]);
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
    const displayCategories = useGetCategories(session, isSuspend);
    const fetchLinkLinesMap = useFetchLinkLines(displayCategories);
    const [checkedCategories, setCheckedCategories] = React.useState<string[]>([]);

    const isEmptyData = displayCategories.length === 0;

    const updateLinkLines = React.useCallback(async () => {
        const newLines: LinkLines = {};
        for (const category of checkedCategories) {
            const datas = await fetchLinkLinesMap.get(category)?.(session);
            if (datas === undefined) { return; }
            newLines[category] = datas;
        }
        Object.values(session.singleLinkLine)
            .forEach(datas => {
                datas?.forEach((data) => {
                    const { category } = data as unknown as FlowEvent;
                    if (!checkedCategories.includes(category)) {
                        newLines[category] = session.singleLinkLine[category];
                    }
                });
            });
        runInAction(() => {
            session.linkLines = newLines;
            session.renderTrigger = !session.renderTrigger;
        });
    }, [checkedCategories]);

    React.useEffect(() => { updateLinkLines(); }, [session.domainRange.domainStart, session.domainRange.domainEnd, checkedCategories]);
    return (
        <FilterContainer>
            <FilterList>
                {isEmptyData
                    ? <StyledEmpty />
                    : displayCategories.map((category, index) => <FilterItem key={index} category={category} checkedCategories={checkedCategories} setCheckedCategories={setCheckedCategories}/>)}
            </FilterList>
            {!isEmptyData && <FilterButtonLine>
                <StyledButton width={50} onClick={() => setCheckedCategories([...displayCategories])}>
                    All
                </StyledButton>
                <StyledButton width={50} onClick={(() => setCheckedCategories([]))}>
                    None
                </StyledButton>
            </FilterButtonLine>}
        </FilterContainer>
    );
});

export const FilterLinkLine = observer(({ session }: { session: Session}): JSX.Element | null => {
    const theme = useTheme();
    const [customButtonProps, updateCustomButtonProps] = useState({
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
            overlayClassName={'insight-category-search-overlay'} align={{ offset: [-8, 3] }}>
            <CustomButton tooltip={i18n.t('tooltip:linker')} { ...customButtonProps } ref={ref}/>
        </Tooltip>
    );
});
