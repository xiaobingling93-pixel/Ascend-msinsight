/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import * as React from 'react';
import styled from '@emotion/styled';
import cls from 'classnames';
import { isEmpty } from 'lodash';
import { runInAction } from 'mobx';
import { observer } from 'mobx-react';
// support utils/types
import { useTranslation } from 'react-i18next';
import { level } from '../../../entity/common';
import type { Session } from '../../../entity/session';
import type { KeyedInsightUnit } from './types';
import type { InsightUnit } from '../../../entity/insight';
import { getAutoKey } from '../../../utils/dataAutoKey';
// assets
import { ReactComponent as Arrow } from '../../../assets/images/insights/PullDownIcon.svg';
import { ReactComponent as StickyIcon } from '../../../assets/images/sticky_unit_button_icon.svg';
// components
import { StyledButton } from '../../base/StyledButton';
// trace/platform
import { platform } from '../../../platforms';
import { traceSingle } from '../../../utils/traceLogger';
// common constant variable
import { isPinned, switchPinned } from '../unitPin';
import { useSelectUnit } from './hooks';
import { ReactComponent as Supported } from '../../../assets/images/insights/Supported.svg';
import { StyledTooltip } from '../../base/StyledTooltip';
import { CardUnit } from '../../../insight/units/AscendUnit';
import { StartIcon, PinIcon, UnPinIcon } from 'lib/Icon';
import { UnitProgress } from '../../charts/UnitProgress';
import { type ParseCardsParam, parseCards } from '../../../api/Request';
import { message } from 'antd';

const DefaultInfoContainer = styled.div`
    display: flex;
    justify-content: space-between;
    font-size: 14px;
    height: 100%;

    .insight-lane-info-header {
        margin-left: 8px;
        min-width: 0;
        flex: 1;
        height: 100%;

        .insight-lane-info-name {
            overflow: hidden;
            text-overflow: ellipsis;
            white-space: nowrap;
        }

        .insight-lane-info-outer-name {
            display: flex;
            align-items: center;
            &.noTag {
                height: 100%;
                font-size: 12px;
            }
        }

       .insight-lane-info-tag-info {
            font-size: 12px;
            line-height: 14px;

            .insight-lane-info-tag-text {
                padding: 1px 8px;
                background-color: ${(props): string => props.theme.unitTagInfoBackgroundColor};
                border-radius: 4px;
            }
        }
    }

    .insight-lane-info-header.expandable {
        align-items: center;
    }
`;

interface DefaultInfoProps {
    session: Session;
    unit: KeyedInsightUnit;
    isHovered: boolean;
    hasPinButton: boolean;
    name: string;
}

const DefaultInfo = observer(({ unit, name, session, ...props }: DefaultInfoProps): JSX.Element => {
    const tag = (typeof unit.tag === 'string') ? `${unit.tag}` : unit.tag?.(session, unit.metadata) ?? undefined;
    return <DefaultInfoContainer>
        <div
            key={ `${getAutoKey(unit)} lane info` }
            className={cls('insight-lane-info-header', { expandable: unit.children && unit.children.length > 0 }) }
        >
            <div className={cls('insight-lane-info-outer-name', { noTag: isEmpty(tag) })}>
                <StyledTooltip title={name}>
                    <span className="insight-lane-info-name">{name}</span>
                </StyledTooltip>
                { [...unit.notifications ?? []]?.map((item, index) => {
                    const notifyRes = item(unit.metadata);
                    if (notifyRes !== false) {
                        return <StyledTooltip key={index} title={notifyRes}><Supported style={{ marginLeft: 8 }}/></StyledTooltip>;
                    }
                    return null;
                })}
            </div>
            { !isEmpty(tag) && <div className={ 'insight-lane-info-tag-info' }>
                <span className="insight-lane-info-tag-text">{tag}</span>
            </div> }
        </div>
        <ConfigBar unit={unit} session={session} {...props} />
    </DefaultInfoContainer>;
});

const shouldDisplayStickyButton = (session: Session, isHovered: boolean, hasPinButton: boolean, _isPinned: boolean): boolean => {
    return session.phase === 'download' && ((hasPinButton && isHovered) || _isPinned);
};

interface PinButtonProps {
    session: Session;
    unit: KeyedInsightUnit;
    isHovered: boolean;
    hasPinButton: boolean;
    isPinned: boolean;
}
const PinButton = observer(({ session, unit, isHovered, hasPinButton, isPinned }: PinButtonProps): JSX.Element => {
    const { t } = useTranslation();
    const style = { backgroundColor: 'transparent', marginLeft: 10 };
    const placeholder = <StyledButton style={style} icon={<StickyIcon fill="transparent" />} />;
    return <>
        {shouldDisplayStickyButton(session, isHovered, hasPinButton, isPinned)
            ? <StyledTooltip title={t(`headerButtonTooltip:${isPinned ? 'UnpinButton' : 'PinButton'}`)}>
                <StyledButton
                    style={style}
                    icon={isPinned ? <PinIcon/> : <UnPinIcon/>}
                    onClick={(e: React.MouseEvent): void => {
                        e.stopPropagation();
                        e.preventDefault();
                        let execute = (): void => {};
                        const { pinnedUnits } = session;
                        if (!isPinned) {
                            execute = (): void => {
                                platform.trace('stickyLane', {});
                                pinnedUnits.unshift(unit);
                                session.pinnedUnits = [...pinnedUnits];
                            };
                        } else {
                            execute = (): void => {
                                pinnedUnits.splice(pinnedUnits.indexOf(unit), 1);
                                session.pinnedUnits = [...pinnedUnits];
                            };
                        }
                        runInAction(() => {
                            execute();
                            switchPinned(unit);
                        });
                    }}
                />
            </StyledTooltip>
            : placeholder}
    </>;
});

interface ConfigBarProps {
    session: Session;
    unit: KeyedInsightUnit;
    isHovered: boolean;
    hasPinButton: boolean;
}
const ConfigBar = observer(({ session, unit, isHovered, hasPinButton }: ConfigBarProps): JSX.Element => {
    return <div className="insight-lane-configbar">
        <div style={{ display: 'flex', marginLeft: 5 }}>
            {unit.configBar?.(session, unit.metadata)}
            <PinButton
                session={session}
                unit={unit}
                isHovered={isHovered}
                hasPinButton={hasPinButton}
                isPinned={isPinned(unit)}
            />
        </div>
    </div>;
});

interface UnitInfoContentProps {
    unit: KeyedInsightUnit;
    session: Session;
    isHovered: boolean;
    hasPinButton: boolean;
    isPinned: boolean;
}

const InsightLaneInfoContainer = styled.div`
    height: 100%;
    display: flex;
    justify-content: space-between;
`;

function getProgressVisiable(unit: KeyedInsightUnit): boolean {
    return unit instanceof CardUnit && unit.metadata?.cardName !== 'Host' &&
        (unit.phase === 'analyzing' || unit.phase === 'download') && unit.shouldParse;
};

function getParserVisiable(unit: KeyedInsightUnit): boolean {
    return unit instanceof CardUnit && unit.metadata?.cardName !== 'Host' && unit.shouldParse;
}

const UnitInfoContent = observer(({ unit, session, ...props }: UnitInfoContentProps): JSX.Element => {
    const info = unit.renderInfo?.(session, unit.metadata, unit) ?? `${unit.name}`;
    if (typeof (info) === 'string') {
        return <DefaultInfo
            session={session}
            unit={unit}
            name={info}
            {...props}
        />;
    }
    const [loading, setLoading] = React.useState<boolean>(!session.isPending);
    React.useEffect(() => {
        if (session.isParserLoading) {
            setLoading(true);
        } else {
            setLoading(false);
        }
    }, [session.isParserLoading]);
    const handleStartClick = (): void => {
        const param: ParseCardsParam = { cards: [] };
        if (unit instanceof CardUnit && unit.metadata?.cardName !== '' && unit.metadata?.cardName !== 'Host') {
            param.cards.push(unit.metadata.cardId);
        };
        parseCards(param).then(() => {
            setLoading(true);
        }).catch(err => {
            message.error(err);
        });
    };
    return <InsightLaneInfoContainer className="insight-lane-info">
        {info}
        { !unit.shouldParse && unit.configBar && <ConfigBar
            session={session}
            unit={unit}
            {...props}
        />}
        { (getProgressVisiable(unit) && loading)
            ? <div><UnitProgress unit={unit} realProgress={unit.progress} showProgress={unit.showProgress}/></div>
            : <></> }
        { getParserVisiable(unit)
            ? <div>
                <StyledButton style={{ backgroundColor: 'transparent' }}
                    icon={<StartIcon height={14} width={14}/>}
                    loading={loading} onClick={(): void => handleStartClick()}/>
            </div>
            : <></> }
    </InsightLaneInfoContainer>;
});

const ExpandIcon = observer(({ unit, session }: { unit: KeyedInsightUnit; session: Session }): JSX.Element | null => {
    const onExpand = React.useCallback(async (_unit: KeyedInsightUnit) => {
        const spreadUnits = _unit.spreadUnits;
        if (spreadUnits?.phase === 'expand') {
            await spreadUnits.action?.(_unit, session);
        }
        runInAction(() => {
            _unit.isExpanded = !_unit.isExpanded;
            if (_unit.isExpanded) {
                platform.trace(`unfold${_unit.name.replace(/\s*/g, '')}`, {});
                _unit.children?.forEach(item => {
                    if (item.collapsible && !item.isExpanded && item.collapseAction !== undefined) {
                        item.collapseAction?.(item);
                        item.isExpanded = true;
                    }
                });
            }
            session.renderTrigger = !session.renderTrigger;
        });
        _unit.collapseAction?.(_unit);
    }, [session]);
    if (unit.children !== undefined || (unit.collapsible && unit.collapseAction)) {
        return <div style={{ float: 'left', height: '20px', marginLeft: '6px', top: 'calc(50% - 10px)', position: 'relative' }}
            onClick={(e: React.MouseEvent<HTMLElement, MouseEvent>): Promise<void> => onExpand(unit)}>
            <Arrow style={{ transform: `rotate(${unit.isExpanded ? 0 : '-90deg'})`, cursor: 'pointer' }}
                className={`insight-unit-${unit.isExpanded ? 'expanded' : 'fold'}`} />
        </div>;
    }
    return null;
});

const UnitInfoContainer = styled.div<{ unit: InsightUnit; laneInfoWidth: number }>`
    flex-grow: 0;
    flex-shrink: 0;
    width: ${(props): number => props.laneInfoWidth}px;
    flex-basis: ${(props): number => props.laneInfoWidth}px;
    height: ${(props): number => props.unit.height()}px;
    padding-left: ${(props): number => 36 * ((props.unit as any)[level] ?? 0)}px;
    text-align: left;
`;

interface UnitInfoProps {
    session: Session;
    unit: KeyedInsightUnit;
    laneInfoWidth: number;
    hasExpandIcon: boolean;
    hasPinButton: boolean;
    isPinned: boolean;
    height: number;
    className: string;
}

export const UnitInfo = observer(({ session, unit, laneInfoWidth, hasExpandIcon, className, ...props }: UnitInfoProps): JSX.Element => {
    const [isHovered, setIsHovered] = React.useState(false);
    const selectUnit = useSelectUnit(session);
    return <UnitInfoContainer
        className={`unit-info ${className ?? ''}`}
        unit={unit}
        laneInfoWidth={laneInfoWidth}
        onMouseOver={(): void => {
            if (!isHovered) {
                setIsHovered(true);
            }
        }}
        onMouseLeave={(): void => { setIsHovered(false); }}
        onMouseDown={(): void => {
            selectUnit(unit);
            traceSingle('selectLane', [unit.name]);
        }}
    >
        {hasExpandIcon && <ExpandIcon unit={unit} session={session}/>}
        <UnitInfoContent
            unit={unit}
            session={session}
            isHovered={isHovered}
            {...props}
        />
    </UnitInfoContainer>;
});
