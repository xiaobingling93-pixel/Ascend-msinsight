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
import type { Theme } from '@emotion/react';
import { useTheme } from '@emotion/react';
import styled from '@emotion/styled';
import { Tooltip } from '@insight/lib/components';
import { action } from 'mobx';
import { observer } from 'mobx-react';
import * as React from 'react';
import { useTranslation } from 'react-i18next';
import { ReactComponent as ThumbIcon } from '../assets/images/chartHeader/thumbIcon.svg';
import { useRootStore } from '../context/context';
import type { InsightTemplate } from '../entity/insight';
import type { Session } from '../entity/session';
import { platform } from '../platforms';
import { getAutoKey } from '../utils/dataAutoKey';
import { logger } from '../utils/Logger';

interface ThumbTipProps {
    width: number;
    left: number;
}

interface RecommendedTemplateData {
    timestamp: number;
    templates: string[];
};

const ICON_SIZE = 18;

const ThumbTipContainer = styled.div<ThumbTipProps>`
    overflow: hidden;
    width: ${ICON_SIZE}px;
    height: ${ICON_SIZE}px;
    border-radius: ${ICON_SIZE / 2}px;
    position: absolute;
    z-index: 3;
    transition: width 0.2s linear;
    background-color: rgba(255, 255, 255, 0.2);
    left: calc(${(props): number => props.left}% - ${(ICON_SIZE / 2) + 3}px);
    padding: 0;
    margin: 0 3px;
    width: ${(props): number => props.width}px;
    &:hover {
        z-index: 2;
        cursor: pointer;
    }

    .thumbIcon {
        background-color: ${(props): string => props.theme.thumbIconBackgroundColor}
    }

    .barIcons {
        float: left;
        display: flex;
        align-items: center;
        justify-content: center;
    }
`;

const IconContainer = styled.div`
    width: ${ICON_SIZE}px;
    height: ${ICON_SIZE}px;
    border-radius: 50%;
    padding: 0;
    margin: 0;
`;

const ThumbEntranceBar = styled.div`
    width: 100%;
    height: 100%;
    position: absolute;
    top: 7.5px;
`;

const EmptyIcon = (): JSX.Element => <div>?</div>;

const Icon = React.forwardRef((
    { children, ...props }: { children?: React.ReactElement } & React.HTMLAttributes<unknown>,
    ref?: React.ForwardedRef<HTMLDivElement>,
): JSX.Element => {
    // render ? as placeholder when the children is undefined
    return <IconContainer {...props} ref={ref}>
        {React.cloneElement(children ?? <EmptyIcon />, { style: { width: 15, height: 15 } })}
    </IconContainer>;
});
Icon.displayName = 'insight-icon';

const TipTitle = ({ tipsTexts, template, theme }: { tipsTexts: string[]; template: InsightTemplate; theme: Theme }): JSX.Element => {
    return <span style={{ fontSize: 14, color: theme.fontColor }}>
        {tipsTexts[0]}
        <Icon style={{ display: 'inline-block', verticalAlign: 'text-bottom', margin: '0 2px' }}>{template.icon}</Icon>
        {`${template.name}  ${tipsTexts[1]}`}
    </span>;
};

type TemplatesIconsProps = {
    left: number;
    recommendedTemplates: RecommendedTemplateData['templates'];
} & InitialValue;
const TemplatesIcons = ({ left, recommendedTemplates, createSession, templates, theme, tipsTexts }: TemplatesIconsProps): JSX.Element => {
    // the templates which is filtered by recommendedTemplates
    const filterTemplates = templates.filter((template) => recommendedTemplates.includes(template.id));

    // fold/unfold thumb entrance icons Index state
    const [entranceWidth, setEntranceWidth] = React.useState(ICON_SIZE);
    const onFocusThumbEntrance = (): void => { setEntranceWidth(entranceWidth === ICON_SIZE ? ICON_SIZE * (filterTemplates.length + 1) : ICON_SIZE); };

    return (<ThumbTipContainer
        width={entranceWidth}
        left={left}
        onMouseEnter={onFocusThumbEntrance}
        onMouseLeave={onFocusThumbEntrance}
    >
        <Icon className="thumbIcon barIcons"><ThumbIcon fill={theme.contentBackgroundColor} /></Icon>
        {filterTemplates.map((template) => (
            <Tooltip
                key={getAutoKey(template)}
                title={<TipTitle tipsTexts={tipsTexts} template={template} theme={theme} />}
                overlayInnerStyle={{ borderRadius: 18, padding: '6px 19px', whiteSpace: 'nowrap', textAlign: 'center' }}
                overlayStyle={{ maxWidth: 500, display: entranceWidth < 1.5 * ICON_SIZE ? 'none' : '' }}
                color={theme.toolTipBackgroundColor}
                align={{ offset: [0, -5] }}
                autoAdjustOverflow
                placement="bottom"
            ><Icon className="barIcons" onClick={(): void => createSession(template)}>{template.icon}</Icon></Tooltip>
        ))}
    </ThumbTipContainer>);
};

const transformTimeToLeft = (domainStart: number, domainEnd: number, timestamp: number): number => {
    if (domainEnd === domainStart) {
        return 0;
    }
    return 100 * (timestamp - domainStart) / (domainEnd - domainStart);
};

type ThumbEntranceProps = {
    domainStart: number;
    domainEnd: number;
    recommendedTemplates: RecommendedTemplateData[];
} & InitialValue;
const ThumbEntrance = ({ domainStart, domainEnd, recommendedTemplates, ...props }: ThumbEntranceProps): JSX.Element => {
    return (
        <ThumbEntranceBar>
            {recommendedTemplates.map((recommendedTemplate, index) => {
                if (recommendedTemplate.timestamp > domainStart && recommendedTemplate.timestamp < domainEnd) {
                    return <TemplatesIcons
                        key={index}
                        left={transformTimeToLeft(domainStart, domainEnd, recommendedTemplate.timestamp)}
                        recommendedTemplates={recommendedTemplate.templates}
                        {...props}
                    />;
                } else {
                    return null;
                }
            })}
        </ThumbEntranceBar>
    );
};

interface InitialValue {
    createSession: (template: InsightTemplate) => void;
    templates: InsightTemplate[];
    theme: Theme;
    tipsTexts: string[];
};
const useInitial = (): InitialValue => {
    // thematic
    const theme = useTheme();

    // text resource
    const { t: i18n } = useTranslation();
    // split the text by [&], because the text need display as [text]<Icon/>[text]
    const tipsTexts = i18n('RecommendedTemplate').split('&');
    const { insightStore, sessionStore } = useRootStore();
    return {
        createSession: action(async (template: InsightTemplate) => {
            platform.trace('openRecommend', { template: template.name });
            const sessionConf: Partial<Session> = {
                name: template.name,
                units: insightStore.get(template.id)?.units?.map(Unit => new Unit(null as never)) ?? [], // 对于 host 场景 units 的 parent 将会消失
                availableUnits: insightStore.get(template.id)?.availableUnits?.map(Unit => new Unit(null as never)) ?? [],
                icon: template.icon,
                isNsMode: template.isNsMode,
            };
            sessionStore.activeSession = await sessionStore.newSession(sessionConf) ?? {} as Session;
        }),
        templates: Array.from(insightStore.templates.values()),
        theme,
        tipsTexts,
    };
};

const Recommendations = observer(({ session }: { session: Session }): JSX.Element => {
    /**
     * initialize value, includes:
     * [createSession] -> the function to create a new session,
     * [templates] -> all supportive template array,
     * [theme] -> current DevEco theme,
     * [tipsTexts] -> Chinese/English texts resourcce from i18n.
     **/
    const initialValue = useInitial();

    const { domainRange: { domainStart, domainEnd }, endTimeAll } = session;
    // Fetch wedge
    const fetch = async (_session: Session): Promise<RecommendedTemplateData[]> => [];

    // useState to manage data, watch domain, endTimeAll change to update RecommendedTemplate
    const [recommandedTemplateData, setRecommandedTemplateData] = React.useState<RecommendedTemplateData[]>([]);
    const updateData = async (): Promise<void> => {
        setRecommandedTemplateData(await fetch(session).catch(() => {
            logger('Recommendations', 'fetch data error', 'warn');
            return [];
        }));
    };
    React.useEffect(() => {
        updateData();
    }, [domainStart, domainEnd, endTimeAll]);

    return <ThumbEntrance {...session.domainRange} {...initialValue} recommendedTemplates={recommandedTemplateData} />;
});

export default Recommendations;
