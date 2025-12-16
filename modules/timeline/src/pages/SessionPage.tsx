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

import styled from '@emotion/styled';
import { observer } from 'mobx-react-lite';
import React, { useEffect, useState } from 'react';
import { BottomPanel } from '../components/BottomPanel';
import { ChartContainer } from '../components/ChartContainer';
import type { Session } from '../entity/session';
import { stateTexts } from '../utils/constant';
import { DragDirection, useDraggableContainer } from '@insight/lib';
import { useActionManager } from '../context/context';
import { actions } from '../actions/register';

const ImgWithFallback = ({
    className = '',
}): JSX.Element => {
    const PictureContainer = styled.picture`
        img {
            height: 48px;
            width: 48px;
        }
    `;
    return (
        <PictureContainer>
            <div className={className}></div>
        </PictureContainer>
    );
};

const StatePopover = observer(({ session }: { session: Session }) => {
    const stateText = stateTexts[session.phase];
    const shouldDisplay = stateText !== '';
    const Mask = styled.div`
        position: absolute;
        display: ${shouldDisplay ? 'flex' : 'none'};
        z-index: 4;
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        width: 100%;
        height: 100%;
        justify-content: center;
        align-items: center;
        background-color: ${(props): string => props.theme.maskColor};
    `;
    const Info = styled.div`
        border-radius: 4px;
        position: relative;
        display: flex;
        align-items: center;
        width: 16rem;
        height: 3rem;
        font-size: 1.12rem;
        .img {
            margin-right: 10px;
        }
    `;
    return <Mask>
        <Info className={'info'}>
            <ImgWithFallback className={'loading'}/>
            {stateText}</Info>
    </Mask>;
});

export const BOTTOM_HEIGHT = 332;
export const SessionPage = observer(({ session }: { session: Session }): any => {
    const actionManager = useActionManager();
    // 用户是否主动收起底部面板（收起后不在自动弹起）
    const [hasUserHiddenBottomPanel, setHasUserHiddenBottomPanel] = useState(false);
    const [view, handleOpen, togglePanel] = useDraggableContainer({
        draggableWH: BOTTOM_HEIGHT,
        dragDirection: DragDirection.BOTTOM,
        open: false,
    });
    useEffect(() => {
        if (hasUserHiddenBottomPanel) {
            return;
        }
        if (session.selectedUnitKeys.length > 0 && (session.selectedRange !== undefined || session.selectedData !== undefined)) {
            handleOpen(true);
            setHasUserHiddenBottomPanel(true);
        }
    }, [session.selectedUnitKeys, session.selectedRange, session.selectedData]);

    useEffect(() => {
        if (session.doContextSearch !== undefined || session.showEvent !== undefined) {
            handleOpen(true);
        }
    }, [session.doContextSearch, session.showEvent]);

    useEffect(() => {
        if (session.showBottomPanel === null) {
            return;
        }
        togglePanel();
        setHasUserHiddenBottomPanel(true);
    }, [session.showBottomPanel]);

    useEffect(() => {
        actionManager.registerAll(actions);
    }, []);

    return view({
        mainContainer: <ChartContainer session={session} actionManager={actionManager} interactive />,
        draggableContainer: <BottomPanel session={session} />,
        slot: <StatePopover session={session} />,
        id: 'SessionPage',
        padding: 16,
    });
});
