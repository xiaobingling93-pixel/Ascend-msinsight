/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

import * as React from 'react';
import { observer } from 'mobx-react';
import { observable } from 'mobx';
import { useTheme } from '@emotion/react';
import type { Session } from '../../../entity/session';
import type { BottomPanelSingleRender } from '../../../entity/insight';
import { TabTitlesProto, createInteractorProps } from '../base/Tabs';
import type { TabProto, TabComponentProps, CommonStateProto } from '../base/Tabs';

interface MoreRender<T extends object> {
    More: (props: T) => JSX.Element;
    moreProps: Omit<T, 'session' | 'height'> | Record<string, unknown>;
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

type TabMoresReturn<More extends keyof ReturnType<BottomPanelSingleRender>,
    Title extends Exclude<keyof ReturnType<BottomPanelSingleRender>, More>> = Pick<ReturnType<BottomPanelSingleRender>, More | Title>;
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
