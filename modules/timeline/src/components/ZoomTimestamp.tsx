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

import * as React from 'react';
import styled from '@emotion/styled';
import { runInAction } from 'mobx';
import { observer } from 'mobx-react';
import { ResetIcon, PlusIcon as Add, MinusIcon as Del } from '@insight/lib/icon';
import type { Session } from '../entity/session';
import { traceStart } from '../utils/traceLogger';
import { getDuration } from '../utils/humanReadable';
import { Tooltip } from '@insight/lib/components';
import { useTranslation } from 'react-i18next';

const TEXT_WIDTH = 50;
const FONT_SIZE = 12;
const Container = styled.div`
    display: flex;
    align-items: center;
    text-align: center;
    font-size: ${FONT_SIZE}px;
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
    user-select: none;
`;
const Controller = styled.div`
  display: flex;
  align-items: center;
  text-align: center;
  background: ${(props): string => props.theme.bgColorCommon};
  border-radius: 4px;
  padding: 1px 6px;
  margin-left: 8px;
`;

export const ZoomTimestamp = observer(({ session }: { session: Session }) => {
    const { t } = useTranslation();
    const { zoom, isLowerBound, isUpperBound, isResetDisabled } = React.useMemo(() => ({
        zoom: getDuration(session.domain.duration, { precision: session.isNsMode ? 'ns' : 'ms', maxChars: TEXT_WIDTH / FONT_SIZE }),
        isLowerBound: session.domain.isLowerBound,
        isUpperBound: session.domain.isUpperBound,
        isResetDisabled: session.endTimeAll === undefined,
    }), [session.domain.duration, session.endTimeAll]);
    return <Container>
        <Tooltip title={t('tooltip:reset')}><ResetIcon
            data-testid={'tool-reset'}
            disabled={isResetDisabled}
            onClick={(): void => {
                runInAction(() => {
                    session.setDomainWithoutHistory({ domainStart: 0, domainEnd: session.endTimeAll ?? session.domain.defaultDuration });
                    session.contextMenu.zoomHistory = [];
                });
            }}
        /></Tooltip>
        <Controller>
            <Tooltip title={t('tooltip:del')}>
                <Del
                    data-testid={'tool-zoom-in'}
                    disabled={isUpperBound}
                    onClick={(): void => {
                        runInAction(() => {
                            traceStart('zoomProportion', { action: 'zoomProportion' });
                            session.zoom = { zoomCount: 1 };
                        });
                    }}
                />
            </Tooltip>
            <Tooltip title={zoom}>
                <Percentage>{zoom}</Percentage>
            </Tooltip>
            <Tooltip title={t('tooltip:add')}>
                <Add
                    data-testid={'tool-zoom-out'}
                    disabled={isLowerBound}
                    onClick={(): void => {
                        runInAction(() => {
                            traceStart('zoomProportion', { action: 'zoomProportion' });
                            session.zoom = { zoomCount: -1 };
                        });
                    }}
                />
            </Tooltip>
        </Controller>
    </Container>;
});
