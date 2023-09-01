import * as React from 'react';
import styled from '@emotion/styled';
import type { Theme } from '@emotion/react';
import { runInAction } from 'mobx';
import { Session } from '../../../entity/session';

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

export const createInteractorProps = <Tab extends TabProto, CommonState extends CommonStateProto>(tabs: Tab[], commonState: CommonState, tabActionCallback?: TabActionCallback<Tab>): InteractorProps => {
    return {
        createdToggleProps: (id: number, session: Session) => {
            return {
                onClick: (e: React.MouseEvent<HTMLElement, MouseEvent>) => {
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
