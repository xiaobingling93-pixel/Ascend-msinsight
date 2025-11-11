/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import styled from '@emotion/styled';
import { Tooltip } from '@insight/lib/components';
import i18n from '@insight/lib/i18n';
import { NOT_APPLICABLE } from './defs';

const StallBarContainer = styled.div`
    line-height: 20px;
    .bar {
        width: calc(100% - 35px);
        min-width: 80px;
        display: inline-block;
        height: 20px;
        float:left;
        color: white;
        background: ${(p): string => p.theme.warningColor};
    }
    .bestbar {
        display: inline-block;
        height: 100%;
        float:left;
        background: ${(p): string => p.theme.colorPalette.tealGreen};
    }
    .inside-label{
        float: right;
        margin-right: 5px;
    }
    .outside-label{
        margin-left: 5px;
    }
`;

// Stall Cycles 阻塞的时钟周期状态条，预期阻塞/实际阻塞百分比
export function StallBar({ real, theoretical }: Record<string, number | undefined>): JSX.Element {
    const realNum = Number(real);
    const bestNum = Number(theoretical);
    const isWrongNumber = isNaN(realNum) || isNaN(bestNum) || bestNum <= 0 || realNum <= 0;
    let bar;
    if (!isWrongNumber) {
        let percent = Number(((bestNum / realNum) * 100).toFixed(0));
        percent = Math.min(100, percent);
        // 绿色条：占比小于50，数字在色条右侧，否则在色条里面
        const bestBar = percent < 50
            ? <>
                <div className={'bestbar'} style={{ width: `${percent}%` }}></div>
                <span className={'outside-label'}>{bestNum}</span>
            </>
            : <div className={'bestbar'} style={{ width: `${percent}%` }}>
                <span className={'inside-label'}>{bestNum}</span>
            </div>;
        // 整条：绿色条占比小于50，数字在整条里面，否则在整条右侧
        bar = percent < 50
            ? <div className={'bar'}>
                {bestBar}
                <span className={'inside-label'}>{realNum}</span>
            </div>
            : <>
                <div className={'bar'}>{bestBar}</div>
                <span className={'outside-label'}>{realNum}</span>
            </>;
    }

    return isWrongNumber
        ? <></>
        : <Tooltip placement="topLeft" title={<div>
            <div>{i18n.t('TheoreticalStallCycles', { ns: 'source' })}: {theoretical} </div>
            <div>{i18n.t('RealStallCycles', { ns: 'source' })}: {real} </div>
        </div>}>
            <StallBarContainer>{bar}</StallBarContainer>
        </Tooltip>;
}

const BarContainer = styled.div`
    min-width: 100px;
    .bar {
        display: inline-block;
        height: 20px;
        float: left;
        text-align: right;
        overflow: hidden;
        color: white;
        background: ${(p): string => p.theme.primaryColor};
        border-left: 1px solid ${(p): string => p.theme.primaryColor};
        border-right: 1px solid ${(p): string => p.theme.primaryColor};
        borderRadius: 1px;
        text-overflow: ellipsis;
    }
    .width0{
        border: none;
    }
    .inside-label{
        margin-right: 2px;
    }
    .outside-label{
        margin-left: 2px;
    }
`;

export enum BarType {
    PERCENT = 0,
}
interface IProps {
    value: number;
    max?: number;
    type?: BarType;
}
// 百分比色条，2种
// 默认：使用当前值/最大值 计算百分比
// BarType。PERCENT：直接使用value作为百分比
function Bar({ value, max = 1, type }: IProps): JSX.Element {
    const valueNum = Number(value);
    const maxNum = Number(max);
    const isWrongNumber = type === BarType.PERCENT
        ? isNaN(valueNum) || valueNum < 0
        : isNaN(valueNum) || valueNum < 0 || isNaN(maxNum) || maxNum <= 0;
    let percent = 0;
    let label;
    if (!isWrongNumber) {
        percent = type === BarType.PERCENT
            ? valueNum
            : Number(((value / max) * 100).toFixed(0));
        percent = Math.min(100, percent);
        label = type === BarType.PERCENT
            ? `${valueNum}%`
            : value;
    }
    if (value === undefined || value === null) {
        return <></>;
    }
    return isWrongNumber
        ? <>{NOT_APPLICABLE}</>
        : (<BarContainer title={`${label}`}><BaseBar value={valueNum} percent={percent} label={label}/></BarContainer>);
};
export default Bar;

interface IBaseBarProps {
    value: number;
    percent: number;
    label: React.ReactNode;
}
function BaseBar({ value, percent, label }: IBaseBarProps): JSX.Element {
    // 百分比小于等于50，数字放在色条右侧，否则放在色条里面
    return percent <= 50
        ? <>
            <div className={`bar width${value}`} style={{ width: `${percent}%` }}></div>
            <span className={'outside-label'}>{label}</span>
        </>
        : <div className={'bar'} style={{ width: `${percent}%` }}>
            <span className={'inside-label'}>{label}</span>
        </div>;
}
