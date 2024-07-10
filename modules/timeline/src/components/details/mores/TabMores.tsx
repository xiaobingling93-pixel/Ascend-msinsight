import * as React from 'react';
import { observer } from 'mobx-react';
import { observable } from 'mobx';
import { useTheme } from '@emotion/react';
import { Session } from '../../../entity/session';
import { BottomPanelRender } from '../../../entity/insight';
import { TabProto, TabComponentProps, CommonStateProto, TabTitlesProto, createInteractorProps } from '../base/Tabs';

interface MoreRender<T extends object> {
    More: (props: T) => JSX.Element;
    moreProps: Omit<T, 'session' | 'height'> | {};
}

export interface MoreTabs<T extends object> extends TabProto {
    render: MoreRender<T>;
};

type MoreComponentProps<T extends object, MoreTabsState extends CommonStateProto> = TabComponentProps<MoreTabs<T>, MoreTabsState> & { height: number };
const Mores = observer(<T extends object, MoreTabsState extends CommonStateProto>(
    { tabs, commonState: { activeKey }, session, height }: MoreComponentProps<T, MoreTabsState>): JSX.Element => {
    const tab = tabs[activeKey];
    const props = Object.assign(tab.render.moreProps, {
        session,
        height,
    }) as T;
    return <tab.render.More { ...props } />;
});

const MoreTitles = observer(<T extends object, MoreTabsState extends CommonStateProto>(
    { tabs, commonState: { activeKey }, interactorProps: { createdToggleProps }, session }:
    TabComponentProps<MoreTabs<T>, MoreTabsState>): JSX.Element => {
    const theme = useTheme();
    return (<>
        {tabs.map((tab: MoreTabs<T>, index: number) => (
            <TabTitlesProto
                {...createdToggleProps(index, session)}
                key={index}
                style={{ marginLeft: index === 0 ? 0 : 10 }}
                theme={theme}
                isActive={activeKey === index}
            >
                {tab.title}
            </TabTitlesProto>))}
    </>);
});

interface TabMoresProps<T extends object> {
    tabs: Array<MoreTabs<T>>;
    state?: Record<string, unknown>;
};

type TabMoresReturn<More extends keyof ReturnType<BottomPanelRender>, Title extends Exclude<keyof ReturnType<BottomPanelRender>, More>> = Pick<ReturnType<BottomPanelRender>, More | Title>;
export const TabMores = function<T extends object, MoreTabsState extends CommonStateProto>({ tabs, state }: TabMoresProps<T>): TabMoresReturn<'More', 'MoreTitle'> {
    const commonState = observable(Object.assign(state ?? {}, { activeKey: 0 })) as MoreTabsState;
    const getProps = (session: Session): TabComponentProps<MoreTabs<T>, MoreTabsState> => {
        return {
            tabs,
            commonState,
            interactorProps: createInteractorProps(tabs, commonState),
            session,
        };
    };

    return {
        More: ({ session, height }) => <Mores {...getProps(session)} height={height} />,
        MoreTitle: ({ session }) => <MoreTitles {...getProps(session)} />,
    };
};
