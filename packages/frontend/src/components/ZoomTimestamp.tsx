import * as React from 'react';
import styled from '@emotion/styled';
import { useTheme } from '@emotion/react';
import { runInAction } from 'mobx';
import { observer } from 'mobx-react';
import { ReactComponent as Reset } from '../assets/images/reset.svg';
import { ReactComponent as Del } from '../assets/images/zoomTimestamp_delete.svg';
import { ReactComponent as Add } from '../assets/images/zoomTimestamp_add.svg';
import { Session } from '../entity/session';
import { traceStart } from '../utils/traceLogger';
import { getDuration } from '../utils/humanReadable';

const TEXT_WIDTH = 70;
const FONT_SIZE = 12;
const Container = styled.div`
    display: flex;
    align-items: center;
    text-align: center;
    font-size: ${FONT_SIZE}px;
    margin-right: 1em;
    svg {
        cursor: pointer;
    }
`;
const Percentage = styled.span`
    margin: 0 .5em;
    width: ${TEXT_WIDTH}px;
`;

export const ZoomTimestamp = observer(({ session }: { session: Session }) => {
    const theme = useTheme();

    const { zoom, isLowerBound, isUpperBound } = React.useMemo(() => ({
        zoom: getDuration(session.domain.duration, { precision: session.isNsMode ? 'ns' : 'ms', maxChars: TEXT_WIDTH / FONT_SIZE }),
        isLowerBound: session.domain.isLowerBound,
        isUpperBound: session.domain.isUpperBound,
    }), [ session.domain.duration, session.endTimeAll ]);
    return <Container>
        <Container>
            <Reset
                fill={isUpperBound ? theme.disableButtonBackgroundColor : theme.activeButtonBackgroundColor}
                onClick={() => {
                    runInAction(() => {
                        session.domainRange = { domainStart: 0, domainEnd: session.endTimeAll ?? session.domain.defaultDuration };
                    });
                }}
            />
        </Container>
        <Del
            fill={isUpperBound ? theme.disableButtonBackgroundColor : theme.activeButtonBackgroundColor}
            onClick={() => {
                runInAction(() => {
                    traceStart('zoomProportion', { action: 'zoomProportion' });
                    session.zoom = { zoomCount: 1 };
                });
            }}
        />
        <Percentage>{zoom}</Percentage>
        <Add
            fill={isLowerBound ? theme.disableButtonBackgroundColor : theme.activeButtonBackgroundColor}
            onClick={() => {
                runInAction(() => {
                    traceStart('zoomProportion', { action: 'zoomProportion' });
                    session.zoom = { zoomCount: -1 };
                });
            }}
        />
    </Container>;
});
