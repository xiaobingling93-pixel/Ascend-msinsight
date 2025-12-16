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
import type * as React from 'react';
import styled from '@emotion/styled';
import type { Theme } from '@emotion/react';
import { runInAction } from 'mobx';
import type { Session } from '../../../entity/session';

export interface TabComponentProps<Tab extends TabProto, CommonState extends CommonStateProto> {
    tabs: Tab[];
    commonState: CommonState;
    session: Session;
    interactorProps: InteractorProps;
};

export interface TabTitlesProtoProps {
    theme: Theme;
    isActive: boolean;
};

export const TabTitlesProto = styled.span((props: TabTitlesProtoProps) => ({
    display: 'inline-block',
    color: props.isActive ? props.theme.fontColor : 'rgb(148, 148, 148)',
    height: '100%',
    borderBottom: props.isActive ? '3px solid rgb(48, 122, 248)' : '',
}));

export interface TabProto {
    title: string;
};

export interface CommonStateProto {
    activeKey: number;
}

export interface InteractorProps {
    createdToggleProps: (id: number, session: Session) => { onClick: (e: React.MouseEvent<HTMLElement, MouseEvent>) => void };
};

export interface TabActionCallback<Tab> {
    actionName: keyof InteractorProps;
    callback: (tabs: Tab[], id: number, session: Session) => void;
};

export const createInteractorProps = <Tab extends TabProto, CommonState extends CommonStateProto>(
    tabs: Tab[], commonState: CommonState, tabActionCallback?: TabActionCallback<Tab>): InteractorProps => {
    return {
        createdToggleProps: (id: number, session: Session): any => {
            return {
                onClick: (e: React.MouseEvent<HTMLElement, MouseEvent>): void => {
                    e.preventDefault();
                    runInAction(() => {
                        commonState.activeKey = id;
                        if (tabActionCallback?.actionName === 'createdToggleProps' && typeof tabActionCallback?.callback === 'function') {
                            tabActionCallback.callback(tabs, id, session);
                        }
                    });
                },
            };
        },
    };
};
