import styled from '@emotion/styled';
import { observer } from 'mobx-react-lite';
import loading from '../assets/images/loading.webp';
import React, { useEffect } from 'react';
import { BottomPanel } from '../components/BottomPanel';
import { ChartContainer } from '../components/ChartContainer';
import { Session } from '../entity/session';
import { stateTexts } from '../utils/constant';
import { DragDirection, useDraggableContainer } from '../utils/useDraggableContainer';

const ImgWithFallback = ({
    src = '',
    fallback = '',
    type = 'image/webp',
    ...delegated
}): JSX.Element => {
    const PictureContainer = styled.picture`
        img {
            height: 48px;
            width: 48px;
        }
    `;
    return (
        <PictureContainer>
            <source srcSet={src} type={type} />
            <img src={fallback} {...delegated} />
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
        background-color: ${props => props.theme.maskColor};
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
            <ImgWithFallback className={'img'} src={loading} />
            {stateText}</Info>
    </Mask>;
});

export const BOTTOM_HEIGHT = 332;
export const SessionPage = observer(function SessionPage({ session }: { session: Session }) {
    const [ view, handleOpen ] = useDraggableContainer({ draggableWH: BOTTOM_HEIGHT, dragDirection: DragDirection.bottom, open: false });
    useEffect(() => {
        if (session.selectedUnitKeys.length > 0 && (session.selectedRange !== undefined || session.selectedData !== undefined)) {
            handleOpen(true);
        }
    }, [ session.selectedUnitKeys, session.selectedRange, session.selectedData ]);

    return view({
        mainContainer: <ChartContainer session={session} interactive />,
        draggableContainer: <BottomPanel session={session} />,
        slot: <StatePopover session={session} />,
        id: 'SessionPage',
    });
});
