import styled from '@emotion/styled/dist/emotion-styled.cjs';
import { observer } from 'mobx-react';
import React from 'react';

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

const StatePopover = observer(() => {
    const Mask = styled.div`
      position: absolute;
      display: flex;
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
        <Info className={'info'}><ImgWithFallback className={'loading'}/>waiting...</Info>
    </Mask>;
});

export default StatePopover;
