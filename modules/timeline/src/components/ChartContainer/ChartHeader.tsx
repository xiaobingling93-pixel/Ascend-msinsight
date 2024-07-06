import styled from '@emotion/styled';
import { observer } from 'mobx-react';
import * as React from 'react';
import TimelineAxis from '../../components/charts/TimelineAxis';
import { Session } from '../../entity/session';
import { ChartRow } from '../base/ChartRow';
import Recommendations from '../Recommendations';
import { ZoomTimestamp } from '../ZoomTimestamp';
import { ButtonGroup } from '../ButtonGroup';
import { TimeMakerAxis } from '../TimeMakerAxis';

const HeaderToolbar = styled.div<{ width?: number }>`
    display: flex;
    justify-content: space-between;
    padding-left: 10px;
    padding-right: 10px;
    width: ${(props): number | undefined => props.width}px;
    margin-top: 4px;
    .title {
        font-size: 12px;
    }

    button{
        z-index: 10;
    }
`;

const HeaderLeft = observer(({ session, laneInfoWidth }: { session: Session; laneInfoWidth: number }) =>
    (<HeaderToolbar width={laneInfoWidth}>
        <ButtonGroup session={session} />
        <ZoomTimestamp session={session} />
    </HeaderToolbar>));

const RightHeader = observer(({ session, showRecommendation, timelineHeight }:
{ session: Session; showRecommendation: boolean; timelineHeight: number }): JSX.Element => {
    return (
        <div onMouseMove = { handleMouseMove } onMouseLeave = { handleMouseLeave }>
            <TimelineAxis session={session} margin={2} timelineHeight={timelineHeight} />
            { showRecommendation ? <Recommendations session={session}/> : <></> }
        </div>
    );
});

const handleMouseMove = (e: React.MouseEvent): void => {
    const existCanvas = document.getElementsByClassName('drawCanvas');
    for (let i = 0; i < existCanvas.length; i++) {
        existCanvas.item(i)?.dispatchEvent(new MouseEvent(e.nativeEvent.type, e.nativeEvent));
    }
};

const handleMouseLeave = (e: React.MouseEvent): void => {
    const existCanvas = document.getElementsByClassName('drawCanvas');
    for (let i = 0; i < existCanvas.length; i++) {
        existCanvas.item(i)?.dispatchEvent(new MouseEvent(e.nativeEvent.type, e.nativeEvent));
    }
};

const ChartHeader = observer(({ session, laneInfoWidth, showRecommendation, timelineHeight }:
{ session: Session; laneInfoWidth: number; showRecommendation: boolean; timelineHeight: number }) => {
    return (
        <div>
            <ChartRow className="timeStamp" leftWidth={laneInfoWidth} rightWidth={`calc(100% - ${laneInfoWidth}px)`}>
                <HeaderLeft session={session} laneInfoWidth={laneInfoWidth} />
                <RightHeader session={session} showRecommendation={showRecommendation} timelineHeight={timelineHeight} />
            </ChartRow>
            <TimeMakerAxis session={session} laneInfoWidth={laneInfoWidth}
                showRecommendation={showRecommendation} timelineHeight={timelineHeight} />
        </div>
    );
});

export default ChartHeader;
