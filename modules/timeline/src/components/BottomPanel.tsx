import styled from '@emotion/styled';
import { Card, Tabs } from 'antd/lib/index';
import { CardProps } from 'antd/lib/card';
import { isEmpty } from 'lodash';
import { observer } from 'mobx-react';
import React, { ReactNode, useEffect, useRef, useState } from 'react';
import { BottomPanelRender, TriggerEvent } from '../entity/insight';
import { Session } from '../entity/session';
import { BOTTOM_HEIGHT } from '../pages/SessionPage';
import { DragDirection, useDraggableContainer } from '../utils/useDraggableContainer';
import { SimpleTabularDetail } from './details/SimpleDetail';
import { ChartErrorBoundary } from './error/ChartErrorBoundary';
import { FILTER_HEIGHT } from './FilterContainer';
import eventBus, { useEventBus } from '../utils/eventBus';
import { getDetailViewItem } from './detailViews/DetailView';

interface CssProps {
    className?: string;
}

interface BottomPanelProps {
    session: Session;
}

type DataCardType = {
    height: number;
    session: Session;
};

export const DETAIL_HEADER_HEIGHT_PX = 38;
const MORE_HEADER_HEIGHT_PX = 22;

const Container = styled.div`
    display: flex;
    justify-content: flex-start;
    overflow: hidden;
    color: lightgray;
    height: 100%;
    width: 100%;
    border-bottom: 1px solid ${props => props.theme.backgroundColor};
    transition: height 0.5s;
    flex-shrink: 0;
    z-index: 3; // to cover tooltip, z-index of tooltip equals 2
    .ant-card, .ant-card-head {
        border-radius: 0;
        line-height: 1.2;
    }
    .ant-tabs-tab {
        padding: 0 2px;
    }
    .ant-tabs-content {
        height: 100%;
    }
    .ant-tabs-tabpane {
        height: 100%;
    }
    .title {
        text-align: start;
        border-bottom: 1px solid ${p => p.theme.solidLine};
        height: ${DETAIL_HEADER_HEIGHT_PX - 2}px; // 2: draggable border-top width
        &>span {
            margin: 0 8px;
            line-height: ${DETAIL_HEADER_HEIGHT_PX - 2}px;
            font-size: 1.14295rem;
            color: ${p => p.theme.fontColor};
        }
    }
    .ant-tabs-content-holder{
        height:100%;
    }
`;

const StyledCard = styled((props: CardProps & { width?: number}) => <Card {...props}/>)`
    height: 100%;
    width: 100%;
    .ant-card-head-wrapper {
        width: 100%;
    }
    .empty {
        margin: 0 auto;
        align-self: center;
        color: ${props => props.theme.fontColor};
    }
    .ant-card-head-title {
        padding: 0;
    }
    & > .ant-card-head {
      position: relative;
      font-weight: normal;
      display: flex;
      border-bottom: 1px solid ${p => p.theme.solidLine};
      padding: 0 16px;
      background-color: ${p => p.theme.cardHeadBackgroundColor};
      color: ${p => p.theme.fontColor};
      font-size: 0.875rem;
      width: 100%;

    }
    .ant-card-body {
        display: flex;
        background-color: ${p => p.theme.contentBackgroundColor};
        height: 100%;
        width: 100%;
        padding: 0;
    }
`;

const StyledMoreCard = styled(StyledCard)`
    background-color: ${p => p.theme.contentBackgroundColor};
    & > .ant-card-head {
        min-height: 0;
        font-size: 1rem;
        background-color: ${p => p.theme.contentBackgroundColor};
        border-left: ${p => p.theme.dividerColor} 1px solid;
    }
    .ant-card-body {
        border-left: ${p => p.theme.dividerColor} 1px solid;
        border-top: 1px solid ${p => p.theme.solidLine};
    }
    .ant-card-head-wrapper {
        background-color: ${p => p.theme.contentBackgroundColor};
    }
    .ant-card-head-title {
        background-color: ${p => p.theme.contentBackgroundColor};
        height: ${MORE_HEADER_HEIGHT_PX}px;
        padding-top: 3px;
        text-align: start;
        & span {
            margin-right: 16px;
            font-size: 1rem;
        }
    }
`;

const DetailContainer = styled.div`
    width: 100%;
    transition: width 0.5s;
    .emptyContainer {
        height: 100%;
        display: flex;
    }
`;

const MoreContainer = styled.div`
    overflow: hidden;
    display: flex;
    position: relative;
    width: 100%;
    height: 100%;
`;

const NoDetail = (): JSX.Element => <div className="empty">No Detail</div>;

interface BottomPanelReactNodes {
    detailTitle: ReactNode;
    detail: ReactNode;
    moreTitle: ReactNode;
    more: ReactNode;
    toolbar: ReactNode;
}

const useTriggerEvent = (session: Session): TriggerEvent => {
    const [event, setEvent] = React.useState<TriggerEvent>('SELECTED_RANGE');
    React.useEffect(() => {
        if (event !== 'SELECTED_RANGE') {
            setEvent('SELECTED_RANGE');
        }
    }, [session.selectedRange]);
    React.useEffect(() => {
        if (event !== 'SELECTED_DATA') {
            setEvent('SELECTED_DATA');
        }
    }, [session.selectedData]);
    return event;
};

const useBottomPanelReactNodes = (session: Session, height: number): BottomPanelReactNodes => {
    const { selectedUnitKeys, selectedUnits: [sessionUnit] } = session;
    const triggerEvent = useTriggerEvent(session);
    const bottomPanelComponents = React.useMemo(() => {
        return sessionUnit?.bottomPanelRender?.(session, triggerEvent, sessionUnit?.metadata);
    }, [session, selectedUnitKeys, triggerEvent, session.units.length]);
    const contentHeight = bottomPanelComponents?.Toolbar !== undefined
        ? (height - DETAIL_HEADER_HEIGHT_PX - FILTER_HEIGHT)
        : (height - DETAIL_HEADER_HEIGHT_PX);
    return React.useMemo(() => {
        const detailTitle = getDetailTitleContent(session, bottomPanelComponents);
        eventBus.emit('setDetailTitle', detailTitle);
        return {
            detailTitle,
            detail: getDetailContent(session, contentHeight, bottomPanelComponents),
            moreTitle: getMoreTitle(session, bottomPanelComponents),
            // More Container has extra height
            more: getMoreContent(session, contentHeight - MORE_HEADER_HEIGHT_PX, bottomPanelComponents),
            toolbar: getFilterContent(session, bottomPanelComponents),
        };
    }, [bottomPanelComponents, height]);
};

/* decide what to put in Detail container */
const getDetailContent = (session: Session, height: number, bottomPanelComponents?: ReturnType<BottomPanelRender>): JSX.Element => {
    if (session.selectedUnitKeys.length === 0) {
        return <div className="emptyContainer"><NoDetail/></div>;
    }
    return bottomPanelComponents?.Detail
        ? <bottomPanelComponents.Detail session={session} height={height} />
        : <SimpleTabularDetail detail={session.selectedUnits[0]?.detail} session={session} height={height}/>;
};

/* decide what to put in More container */
const getMoreContent = (session: Session, height: number, bottomPanelComponents?: ReturnType<BottomPanelRender>): JSX.Element | undefined => {
    if (session.selectedUnitKeys.length === 0) {
        return <NoDetail/>;
    }
    return bottomPanelComponents?.More && <bottomPanelComponents.More session={session} height={height} />;
};

/* Details head container */
const getDetailTitleContent = (session: Session, bottomPanelComponents?: ReturnType<BottomPanelRender>): JSX.Element | undefined => {
    if (typeof bottomPanelComponents?.DetailTitle === 'string') {
        return <span>{bottomPanelComponents?.DetailTitle}</span>;
    }
    return bottomPanelComponents?.DetailTitle && <bottomPanelComponents.DetailTitle session={session} />;
};

/* More title */
const getMoreTitle = (session: Session, bottomPanelComponents?: ReturnType<BottomPanelRender>): JSX.Element => {
    const Title = bottomPanelComponents?.MoreTitle;
    if (typeof Title === 'string' || Title === undefined) {
        return <span>{Title ?? 'More'}</span>;
    }
    return <Title session={session} />;
};

/* Filter container in bottom */
const getFilterContent = (session: Session, bottomPanelComponents?: ReturnType<BottomPanelRender>): (JSX.Element | undefined) => {
    return bottomPanelComponents?.Toolbar && <bottomPanelComponents.Toolbar session={session} />;
};

const DataCard = observer(({ session, height }: DataCardType) => {
    const { detail, moreTitle, more, toolbar } = useBottomPanelReactNodes(session, height);
    const [view] = useDraggableContainer({ dragDirection: DragDirection.right, draggableWH: 590 });
    return <div style={{ width: '100%', zIndex: 3, height: '100%' }}>
        {
            !isEmpty(more)
                ? view({
                    mainContainer: <StyledCard
                        bordered={ false }>
                        <ChartErrorBoundary className={'detail-more-error'}>
                            <DetailContainer>
                                {detail}
                            </DetailContainer>
                        </ChartErrorBoundary>
                    </StyledCard>,
                    draggableContainer: <StyledMoreCard
                        className="moreContainer"
                        title={moreTitle}
                        bordered={ false } >
                        <ChartErrorBoundary className={'more-error'}>
                            <MoreContainer>
                                {more}
                            </MoreContainer>
                        </ChartErrorBoundary>
                    </StyledMoreCard>,
                    id: 'detailMore',
                })
                : <StyledCard
                    bordered={ false }>
                    <ChartErrorBoundary className={'detail-error'}>
                        <DetailContainer>
                            {detail}
                        </DetailContainer>
                    </ChartErrorBoundary>
                </StyledCard>
        }
        {toolbar}
    </div>;
});

export const BottomPanel = observer((props: BottomPanelProps & CssProps) => {
    const { session } = props;
    const [bottomHeight, setBottomHeight] = useState(BOTTOM_HEIGHT);
    const [item, setItem] = useState< string >('DataCard');
    const ref = useRef<HTMLDivElement>(null);
    useEffect(() => {
        const bottomResize = (): void => setBottomHeight(ref.current?.clientHeight ?? BOTTOM_HEIGHT);
        window.addEventListener('resize', bottomResize);
        window.addEventListener('bottomResize', bottomResize);
        return () => {
            window.removeEventListener('bottomResize', bottomResize);
            window.removeEventListener('resize', bottomResize);
        };
    }, [setBottomHeight]);
    useEffect(() => {
        setItem('DataCard');
    }, [session.selectedData, session.selectedRange]);

    const items = [
        getDataCardItem(bottomHeight, session),
        getDetailViewItem(session),
    ];

    return (<Container ref={ref} className="bottomPanelContainer">
        <Tabs style={{ width: '100%' }} items={items} activeKey={item} onTabClick={key => setItem(key)}/>
    </Container>);
});

function getDataCardItem(bottomHeight: number, session: Session): any {
    return {
        label: <DataCardTitle/>,
        key: 'DataCard',
        children: <DataCard height={bottomHeight} session={session}/>,
    };
}

const DataCardTitle = (): JSX.Element => {
    const [detailTitle, setDetailTitle] = useState<JSX.Element | undefined>(undefined);
    useEventBus('setDetailTitle', (data) => setDetailTitle(data as JSX.Element));
    return (<div className={'title'}>{detailTitle !== undefined ? detailTitle : <span>Details</span>}</div>);
};
