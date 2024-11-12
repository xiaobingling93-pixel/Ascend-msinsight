/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import styled from '@emotion/styled';
import { Tooltip } from 'ascend-components';
import i18n from 'ascend-i18n';

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
            <div>

            </div>
            <div>

            </div>
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

// Cycles 时钟周期百分比色条，当前值/最大值百分比
const Bar = ({ value = 0, max = 1 }: {value: number;max: number}): JSX.Element => {
    const num = Number(value);
    const maxNum = Number(max);
    const isWrongNumber = isNaN(num) || isNaN(maxNum) || maxNum <= 0 || num < 0;
    let bar;
    if (!isWrongNumber) {
        let percent = Number(((value / max) * 100).toFixed(0));
        percent = Math.min(100, percent);
        // 百分比小于50，数字放在色条右侧，否则放在色条里面
        bar = percent < 50
            ? <>
                <div className={`bar width${num.toFixed(0)}`} style={{ width: `${percent}%` }}></div>
                <span className={'outside-label'}>{value}</span>
            </>
            : <div className={'bar'} style={{ width: `${percent}%` }}>
                <span className={'inside-label'}>{value}</span>
            </div>;
    }
    return isWrongNumber
        ? <></>
        : (<BarContainer title={`${num}`}>{bar}</BarContainer>);
};
export default Bar;
