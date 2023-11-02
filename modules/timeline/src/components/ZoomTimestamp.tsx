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
import { StyledTooltip } from './base/StyledTooltip';
import i18n from 'i18next';

const TEXT_WIDTH = 50;
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
    text-overflow: ellipsis;
    width: ${TEXT_WIDTH}px;
`;

const StyledReset = styled(Reset)<{ pathfill: string }>`
    path {
        fill: ${props => props.pathfill};
    }
`;

export const ZoomTimestamp = observer(({ session }: { session: Session }) => {
    const theme = useTheme();

    const { zoom, isLowerBound, isUpperBound, shouldResetDisable } = React.useMemo(() => ({
        zoom: getDuration(session.domain.duration, { precision: session.isNsMode ? 'ns' : 'ms', maxChars: TEXT_WIDTH / FONT_SIZE }),
        isLowerBound: session.domain.isLowerBound,
        isUpperBound: session.domain.isUpperBound,
        shouldResetDisable: session.endTimeAll === undefined,
    }), [ session.domain.duration, session.endTimeAll ]);
    return <Container>
        <Container>
            <StyledTooltip title={i18n.t('tooltip:reset')}><StyledReset
                pathfill={shouldResetDisable ? theme.disableButtonBackgroundColor : theme.activeButtonBackgroundColor}
                onClick={() => {
                    runInAction(() => {
                        session.domainRange = { domainStart: 0, domainEnd: session.endTimeAll ?? session.domain.defaultDuration };
                    });
                }}
            /></StyledTooltip>
        </Container>
        <StyledTooltip title={i18n.t('tooltip:del')}>
            <Del
                fill={isUpperBound ? theme.disableButtonBackgroundColor : theme.activeButtonBackgroundColor}
                onClick={() => {
                    runInAction(() => {
                        traceStart('zoomProportion', { action: 'zoomProportion' });
                        session.zoom = { zoomCount: 1 };
                    });
                }}
            />
        </StyledTooltip>
        <StyledTooltip title={zoom}>
            <Percentage>{zoom}</Percentage>
        </StyledTooltip>
        <StyledTooltip title={i18n.t('tooltip:add')}>
            <Add
                fill={isLowerBound ? theme.disableButtonBackgroundColor : theme.activeButtonBackgroundColor}
                onClick={() => {
                    runInAction(() => {
                        traceStart('zoomProportion', { action: 'zoomProportion' });
                        session.zoom = { zoomCount: -1 };
                    });
                }}
            />
        </StyledTooltip>
    </Container>;
});
