import React, { ReactNode } from 'react';
import styled from '@emotion/styled';

const ImgWithFallback = ({
    className = '',
}): JSX.Element => {
    const PictureContainer = styled.picture`
        img {
            height: 18px;
            width: 18px;
        }
    `;
    return (
        <PictureContainer>
            <div className={className}></div>
        </PictureContainer>
    );
};

export const Mask: React.FC<{ unitPhase: string; isShowMask: boolean } & { children: ReactNode }> = ({ children, ...props }): JSX.Element => {
    const MaskLayer = styled.div`
        position: absolute;
        display: flex;
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        z-index: 4;
        width: 100%;
        height: 100%;
        justify-content: center;
        align-items: center;
        background-color: ${(p): string => p.theme.maskColor};
    `;
    const Info = styled.div`
        border-radius: 4px;
        position: relative;
        display: flex;
        align-items: center;
        height: 3rem;
        font-size: .8rem;
        .img {
            margin-right: 10px;
        }
    `;
    return <div style={{ position: 'relative' }}>
        {props.isShowMask && <MaskLayer>
            <Info>
                <ImgWithFallback className={'loading'} />
                { props.unitPhase }</Info>
        </MaskLayer>}
        {children}
    </div>;
};
