import * as React from 'react';
import { observable } from 'mobx';
import { observer } from 'mobx-react';
import { useTheme } from '@emotion/react';
import { Session } from '../../entity/session';
import { DetailDescriptor, BottomPanelRender } from '../../entity/insight';
import { FilterContainer } from '../FilterContainer';
import { createInteractorProps, TabComponentProps, CommonStateProto as DetailTabsState, TabTitlesProto, TabProto } from './base/Tabs';
import { OptionType, TabState } from '../../entity/tabDependency';
export interface DetailTabs extends TabProto {
    detail: DetailDescriptor<unknown>;
    bottomPanel?: Partial<CommonBottomPanel>;
    tabState?: TabState;
};

const TabTitles = observer(({ tabs, commonState: { activeKey }, interactorProps: { createdToggleProps }, session }:
TabComponentProps<DetailTabs, DetailTabsState>): JSX.Element => {
    const theme = useTheme();
    return (<>
        {tabs.map((tab: DetailTabs, index: number) => (
            <TabTitlesProto
                {...createdToggleProps(index, session)}
                key={index}
                style={{ marginLeft: index === 0 ? 10 : 15 }}
                theme={theme}
                isActive={activeKey === index}
            >
                {tab.title}
            </TabTitlesProto>))}
    </>);
});

export type CommonBottomPanel<DetailProps = any, MoreProps = any> = {
    Detail?: React.FC<DetailProps>;
    detailProps?: DetailProps;
    DetailTitle?: React.FC<Record<string, unknown>> | string;
    More?: React.FC<MoreProps>;
    moreProps?: MoreProps;
    MoreTitle?: React.FC<Record<string, unknown>> | string;
    options?: OptionType[];
};

interface TabPanesProps {
    tabs: DetailTabs[];
    commonBottomPanel?: CommonBottomPanel;
    isShowFilter?: boolean;
    sharedState?: Record<string, unknown>;
};

type TabDetailProps = TabComponentProps<DetailTabs, DetailTabsState> & { height: number } & Pick<CommonBottomPanel, 'Detail' | 'detailProps'>;
const TabDetail = observer(({ commonState, tabs, Detail: CommonDetail, detailProps, ...props }: TabDetailProps): JSX.Element => {
    const { activeKey } = commonState;
    const tab = tabs[activeKey];
    const Detail = tab?.bottomPanel?.Detail;
    if (Detail) {
        return <Detail {...props} {...tab.bottomPanel?.detailProps} commonState={commonState} tabs={tabs} {...tab} />;
    }
    if (CommonDetail) {
        return <CommonDetail {...props} {...detailProps} commonState={commonState} tabs={tabs} {...tab} />;
    }
    return <></>;
});

type TabMoreProps = TabComponentProps<DetailTabs, DetailTabsState> & { height: number } & Pick<CommonBottomPanel, 'More' | 'moreProps'>;
const TabMore = observer(({ tabs, commonState, More: CommonMore, moreProps, ...props }: TabMoreProps): JSX.Element => {
    const { activeKey } = commonState;
    const tab = tabs[activeKey];
    const More = tab?.bottomPanel?.More;
    if (More) {
        return <More {...props} {...tab.bottomPanel?.moreProps} commonState={commonState} tabs={tabs} more={tab.detail?.more} />;
    }
    if (CommonMore) {
        return <CommonMore {...props} {...moreProps} commonState={commonState} tabs={tabs} more={tab.detail?.more} />;
    }
    return <></>;
});

type TabMoreTitleProps = TabComponentProps<DetailTabs, DetailTabsState> & Pick<CommonBottomPanel, 'MoreTitle'>;
const TabMoreTitle = ({ tabs, commonState, MoreTitle: CommonMoreTitle, ...props }: TabMoreTitleProps): JSX.Element => {
    const { activeKey } = commonState;
    const tab = tabs[activeKey];
    const MoreTitle = tab?.bottomPanel?.MoreTitle ?? CommonMoreTitle;
    if (MoreTitle !== undefined) {
        return typeof MoreTitle === 'string' ? <span>{MoreTitle}</span> : <MoreTitle commonState={commonState} tab={tab} {...props} />;
    }
    return <></>;
};

const autoClearDetail = (tabs: DetailTabs[], id: number, session: Session): void => {
    if (session.selectedUnits[0]?.metadata as boolean) {
        (session.selectedUnits[0]?.metadata as any).idList = [];
    }
    session.selectedDetailKeys = [];
    session.selectedDetails = [];
};

export const TabPanes = ({ tabs, commonBottomPanel, isShowFilter, sharedState }: TabPanesProps): ReturnType<BottomPanelRender> => {
    const commonState = observable({ activeKey: 0, ...sharedState });
    const tabsPaneProps = {
        commonState,
        tabs,
        interactorProps: createInteractorProps(tabs, commonState, { actionName: 'createdToggleProps', callback: autoClearDetail }),
    };
    return {
        DetailTitle: ({ session }): JSX.Element => <TabTitles session={session} {...tabsPaneProps} />,
        Detail: tabs[commonState.activeKey].bottomPanel?.Detail ?? commonBottomPanel?.Detail
            ? ({ session, height }): JSX.Element => <TabDetail
                Detail={commonBottomPanel?.Detail}
                detailProps={commonBottomPanel?.detailProps}
                session={session}
                height={height}
                {...tabsPaneProps} />
            : undefined,
        More: tabs[commonState.activeKey].bottomPanel?.More ?? commonBottomPanel?.More
            ? ({ session, height }): JSX.Element => <TabMore
                More={commonBottomPanel?.More}
                moreProps={commonBottomPanel}
                session={session}
                height={height}
                {...tabsPaneProps} />
            : undefined,
        Toolbar: isShowFilter ? observer((): JSX.Element => (<FilterContainer tabState={tabs[commonState.activeKey].tabState as TabState} />)) : undefined,
        MoreTitle: (tabs[commonState.activeKey].bottomPanel?.MoreTitle ?? commonBottomPanel?.MoreTitle) !== undefined
            ? ({ session }): JSX.Element => <TabMoreTitle
                session={session}
                MoreTitle={commonBottomPanel?.MoreTitle}
                {...tabsPaneProps} />
            : undefined,
    };
};
