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
import { observer } from 'mobx-react';
import { Button } from '@insight/lib/components';
import { message } from 'antd';
import type { RowProps } from 'antd/lib/grid';
import type { Session } from '../entity/session';
import { ChartRowLeft, ChartRowRight } from './base/ChartRow';
import { useTranslation } from 'react-i18next';
import { TimelineMarkerElement } from './TimelineMarker';
import { useTheme } from '@emotion/react';
import { StartIcon } from '@insight/lib/icon';
import { parseCards } from '../api/request';
import type { ParseCardsParam } from '../api/interface';
import { CardUnit } from '../insight/units/AscendUnit';

const ChartRowContainer = styled.div`
    display: flex;
    align-items: center;
    height: 15px;
    border-top: solid 1px ${(props): string => props.theme.borderColor};
`;

const StartParseButton = styled(Button)`
    display: flex;
    align-items: center;
    width: 160px;
    height: 100%;
    border-radius: ${(props): string => props.theme.borderRadiusSmall};
    justify-content: center;
    background: ${(props): string => props.theme.primaryColor};
    border: none;
    color: #F4F6FA;
    margin: auto;
    font-size: 12px;
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
    const handleStartClick = (): void => {
        const param: ParseCardsParam = { cards: [], dbPaths: [] };
        session.units.forEach((unit) => {
            if (unit instanceof CardUnit && unit.metadata?.cardName !== '' && unit.metadata?.cardName !== 'Host') {
                param.cards.push(unit.metadata.cardId);
                param.dbPaths.push(unit.metadata.dbPath);
            }
        });
        session.modeOfParse = 'global_parse';
        parseCards(param).then(() => {
            session.isParserLoading = true;
        }).catch(err => {
            message.error(err);
        });
    };
    return <ChartRow className="timeMakerAxis" leftWidth={laneInfoWidth} rightWidth={`calc(100% - ${laneInfoWidth + theme.scrollBarWidth}px)`}>
        { session.isPending
            ? <StartParseButton
                type="primary"
                icon={ <StartIcon style={{ marginRight: '4px' }} color="#ffffff" height={14} width={14}></StartIcon> }
                loading={session.isParserLoading}
                onClick={(): void => handleStartClick()}
            >
                {t('Start Global Analysis')}
            </StartParseButton>
            : <div></div> }
        <TimelineMarkerElement session={ session } theme={theme}/>
    </ChartRow>;
});
