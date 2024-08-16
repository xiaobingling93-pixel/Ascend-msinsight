/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/

import * as React from 'react';
import styled from '@emotion/styled';
import { runInAction } from 'mobx';
import { observer } from 'mobx-react';
import { ResetIcon, PlusIcon as Add, MinusIcon as Del } from 'ascend-icon';
import type { Session } from '../entity/session';
import { traceStart } from '../utils/traceLogger';
import { getDuration } from '../utils/humanReadable';
import { StyledTooltip } from './base/StyledTooltip';
import { useTranslation } from 'react-i18next';

const TEXT_WIDTH = 50;
const FONT_SIZE = 12;
const Container = styled.div`
    display: flex;
    align-items: center;
    text-align: center;
    font-size: ${FONT_SIZE}px;
    margin-right: 1em;
    line-height: 20px;
    height:32px;
    svg {
        cursor: pointer;
    }
`;
const Percentage = styled.span`
    margin: 0 .5em;
    text-overflow: ellipsis;
    width: ${TEXT_WIDTH}px;
`;

export const ZoomTimestamp = observer(({ session }: { session: Session }) => {
    const { t } = useTranslation();
    const { zoom, isLowerBound, isUpperBound, shouldResetDisable } = React.useMemo(() => ({
        zoom: getDuration(session.domain.duration, { precision: session.isNsMode ? 'ns' : 'ms', maxChars: TEXT_WIDTH / FONT_SIZE }),
        isLowerBound: session.domain.isLowerBound,
        isUpperBound: session.domain.isUpperBound,
        shouldResetDisable: session.endTimeAll === undefined,
    }), [session.domain.duration, session.endTimeAll]);
    return <Container>
        <Container>
            <StyledTooltip title={t('tooltip:reset')}><ResetIcon
                disabled={shouldResetDisable}
                onClick={(): void => {
                    runInAction(() => {
                        session.domainRange = { domainStart: 0, domainEnd: session.endTimeAll ?? session.domain.defaultDuration };
                        session.contextMenu.zoomHistory = [];
                    });
                }}
            /></StyledTooltip>
        </Container>
        <StyledTooltip title={t('tooltip:del')}>
            <Del
                disabled={isUpperBound}
                onClick={(): void => {
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
        <StyledTooltip title={t('tooltip:add')}>
            <Add
                disabled={isLowerBound}
                onClick={(): void => {
                    runInAction(() => {
                        traceStart('zoomProportion', { action: 'zoomProportion' });
                        session.zoom = { zoomCount: -1 };
                    });
                }}
            />
        </StyledTooltip>
    </Container>;
});
