import * as React from 'react';
import styled from '@emotion/styled';
import { observer } from 'mobx-react';
import { RowProps } from 'antd/lib/grid';
import { Session } from '../entity/session';
import { ChartRowLeft, ChartRowRight } from './base/ChartRow';
import { useTranslation } from 'react-i18next';
import { TimelineMarkerElement } from './TimelineMarker';
import { useTheme } from '@emotion/react';

const ChartRowContainer = styled.div`
    display: flex;
    align-items: center;
    height: 15px;
    border-top: solid 1px ${(props): string => props.theme.tableBorderColor};
`;
export const TIME_MARKER_AXIS_HEIGHT = 15;
export interface ChartRowProps extends RowProps {
    className?: string;
    key?: React.Key;
    children: [ JSX.Element | null, JSX.Element ];
    leftWidth: number;
    rightWidth?: React.CSSProperties['width'];
}

const ChartRow = observer((props: ChartRowProps) => {
    return <ChartRowContainer className={ props.className }>
        <ChartRowLeft width={props.leftWidth}>
            { props.children[0] ?? <div/> }
        </ChartRowLeft>
        <ChartRowRight width={props.rightWidth}>
            { props.children[1] }
        </ChartRowRight>
    </ChartRowContainer>;
});

export const TimeMakerAxis = observer(({ session, laneInfoWidth, showRecommendation, timelineHeight }:
{ session: Session; laneInfoWidth: number; showRecommendation: boolean; timelineHeight: number }) => {
    const { t } = useTranslation();
    if (session.name === t('Realtime Monitor')) {
        return null;
    }
    const theme = useTheme();
    return <ChartRow className="timeMakerAxis" leftWidth={laneInfoWidth} rightWidth={`calc(100% - ${laneInfoWidth}px)`}>
        <div></div>
        <TimelineMarkerElement session={ session } theme={theme}/>
    </ChartRow>;
});
