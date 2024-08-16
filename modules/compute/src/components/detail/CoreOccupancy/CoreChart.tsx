/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { observer } from 'mobx-react';
import React, { useEffect, useMemo, useState } from 'react';
import { useTranslation } from 'react-i18next';
import ResizeObserver from 'resize-observer-polyfill';
import styled from '@emotion/styled';
import { useTheme } from '@emotion/react';
import {
    sizeConfig,
    MIN_CHART_WIDTH,
    type CoreDrawData,
    getDrawData,
    getSubCoreColor,
    getLegendData,
    isLastInRow,
    isInLastRow,
} from './draw';
import { type ICondition } from './Filter';
import { type Session } from '../../../entity/session';
import { type ICore } from './Index';
import { LimitHit } from '../../LimitSet';

const MAX_CORE_NUMBER = 1000;

const Container = styled.div`
    .chart-box {
        min-width: ${MIN_CHART_WIDTH}px;
        margin: 10px 0 20px;
        overflow: auto;
    }
    .core-group {
        width: calc(100% - ${sizeConfig.legend.width}px);
        display: inline-block;
    }
    .core {
        height: ${sizeConfig.core.height}px;
        width: ${sizeConfig.core.width}px;
        margin: 0 ${sizeConfig.core.widthSpace}px ${sizeConfig.core.heightSpace}px 0;
        &.lastInRow {
            margin-right: 0;
        }
        &.lastRow {
            margin-bottom: 0;
        }
    }
    .core-border {
        stroke: ${(p): string => p.theme.borderColorLighter};
        stroke-dasharray: 5, 5;
    }
    .core-name {
        font-size: 14px;
        font-weight: bold;
        fill: ${(p): string => p.theme.textColorPrimary};
    }
    .subCore > rect {
        stroke: ${(p): string => p.theme.borderColorLighter};
        stroke-opacity: 0.5;
    }
    .subCore > text {
        font-size: 12px;
        fill: #ffffff;
    }
    .legend {
        float: right;
    }

    .legend text {
        font-size: 12px;
        fill: ${(p): string => p.theme.textColorPrimary};
    }
`;

const CoreChart = observer(({ condition, data }:
{data: ICore[];condition: ICondition;session?: Session}): JSX.Element => {
    const [chartWidth, setChartWidth] = useState(MIN_CHART_WIDTH);
    const [limit, setLimit] = useState({ maxSize: MAX_CORE_NUMBER, overlimit: false, current: 0 });
    const [drawData, setDrawData] = useState<CoreDrawData[]>([]);
    const ref = React.useRef(null);
    const { t } = useTranslation('details');
    const theme = useTheme();
    const legendData = useMemo(() => getLegendData(theme), [theme]);

    // 监听画布宽度变动
    React.useEffect(() => {
        // 初始宽度
        if (ref.current !== null && (ref.current as any).offsetWidth > MIN_CHART_WIDTH) {
            setChartWidth((ref.current as any).offsetWidth);
        }
        const reObserver = new ResizeObserver(([entry]) => {
            window.requestAnimationFrame(() => {
                const { width } = entry.contentRect;
                if (width > MIN_CHART_WIDTH) {
                    setChartWidth(width);
                }
            });
        });
        if (ref.current !== null) {
            reObserver.observe(ref.current);
        }
        return () => {
            reObserver.disconnect();
        };
    }, []);
    // 数据量是否超过限制
    useEffect(() => {
        setLimit({ ...limit, overlimit: data.length > limit.maxSize, current: data.length });
    }, [data.length]);
    // 画图
    useEffect(() => {
        const newDrawData = getDrawData({ svgWidth: chartWidth, data: data.slice(0, limit.maxSize), ...condition });
        setDrawData(newDrawData);
    }, [data, condition, chartWidth]);
    return <Container>
        {limit.overlimit && <LimitHit maxSize={limit.maxSize} name={`${t('Current Count')} (${limit.current})`}/>}
        <div className={'chart-box'} style={{ height: `${sizeConfig.chartHeight}px` }} ref={ref} >
            <div className={'core-group'}>
                {
                    drawData.map((core, coreIndex) => (
                        <svg className={`core ${isLastInRow(coreIndex) ? 'lastInRow' : ''} ${isInLastRow(coreIndex) ? 'lastRow' : ''}`} key={core.name}>
                            <g transform={`translate(${sizeConfig.core.border},${sizeConfig.core.border})`}>
                                <rect className="core-border" rx="2" ry="2" fill="none"
                                    width={sizeConfig.core.width - (2 * sizeConfig.core.border)}
                                    height={sizeConfig.core.height - (2 * sizeConfig.core.border)} ></rect>
                                <text className="core-name" textAnchor="middle" dominantBaseline="middle"
                                    x={sizeConfig.core.width / 2} y={sizeConfig.core.title.top}>{core.name}</text>
                                <g transform={`translate(${(sizeConfig.core.width - sizeConfig.subCore.width) / 2},${sizeConfig.subCore.top})`}
                                    width={sizeConfig.subCore.width}>
                                    {
                                        core.children.map((subCore, subCoreIndex) => (
                                            <g className="subCore" key={core.name + subCore.name}
                                                transform={`translate(0,${subCoreIndex * ((sizeConfig.subCore.height * 2) + sizeConfig.subCore.heightSpace)})`}>
                                                {[subCore.name, subCore.value].map((text, index) => (
                                                    <>
                                                        <rect x="0" rx="2" ry="0" y={index * sizeConfig.subCore.height} width={sizeConfig.subCore.width}
                                                            height={sizeConfig.subCore.height} fill={getSubCoreColor(subCore.level, theme)} ></rect>
                                                        <text textAnchor="middle" dominantBaseline="middle" x={sizeConfig.subCore.width / 2} y={sizeConfig.subCore.height * (index + 0.5)}>{text}</text>
                                                    </>
                                                ))}
                                            </g>
                                        ))
                                    }
                                </g>
                            </g>
                        </svg>
                    ))
                }
            </div>
            <svg className={'legend'} width={sizeConfig.legend.width} height={sizeConfig.legend.height}>
                <g transform={`translate(${sizeConfig.legend.left},${sizeConfig.legend.top})`}>
                    <text x="0" y="0">{t('Level')}</text>
                    <g transform={`translate(0,${sizeConfig.legend.title.height})`}>
                        {
                            legendData.map((legend, index) => (
                                <>
                                    <rect x="0" y={sizeConfig.legend.block.height * index} width={sizeConfig.legend.block.width} height={sizeConfig.legend.block.height} fill={legend.color}></rect>
                                    <text x={sizeConfig.legend.block.width + 15} y={sizeConfig.legend.block.height * (index + 0.65)} textAnchor="middle" >{legend.level}</text>
                                </>
                            ))
                        }
                    </g>
                </g>
            </svg>
        </div>
    </Container>;
});

export default CoreChart;
