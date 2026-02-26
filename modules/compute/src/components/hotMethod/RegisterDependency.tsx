/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
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

import React from 'react';
import styled from '@emotion/styled';

enum RegisterState {
    Space = 0,
    Read = 1,
    Write = 2,
    ReadAndWrite = 3,
    InUsed = 4,
}

enum RegisterProgressState {
    Begin = 0,
    Finish = 1,
}

// 寄存器信息
export interface RegisterTrack {
    name: string; // 寄存器名称，如 "R0", "R1"
    state: RegisterState; // 当前行的状态
    length: number;
    progress: RegisterProgressState; // 结束
}

// 组件 Props
interface RegisterDependencyProps {
    tracks: RegisterTrack[]; // 寄存器轨道数组
    height?: number; // 行高
    trackWidth?: number; // 每个轨道宽度
    cellPadding?: number; // 列表上下的padding
}

// ============ Styled Components ============

const REG_COLOR = '#c586c0';
const LINE_WIDTH = 1;
const ARROW_HEIGHT = 12;

const TrackContainer = styled.div<{ width: number; height: number }>`
  flex: none;
  width: ${props => props.width}px;
  height: ${props => props.height}px;
  position: relative;
  cursor: pointer;
`;

const VerticalLine = styled.div<{
    state: RegisterState;
    cellPadding: number;
    progress: RegisterProgressState;
}>`
  position: absolute;
  width: ${LINE_WIDTH}px;
  background-color: ${REG_COLOR};
  left: 50%;
  transform: translateX(-50%);
  z-index: 0;

  ${({ state, progress, cellPadding }) => {
        // 外溢位置（连接上一行/下一行）
        const overflow = `-${cellPadding}px`;
        // 中心截断位置（配合箭头）
        const centerStart = 'calc(50% - 5px)'; // 开始箭头的中心偏移
        const centerEnd = 'calc(50% - 6px)'; // 结束箭头的中心偏移

        let top = overflow;
        let bottom = overflow;

        if (state === RegisterState.Write || state === RegisterState.ReadAndWrite) {
            if (progress === RegisterProgressState.Begin) top = centerStart;
        }

        if (state === RegisterState.Read || state === RegisterState.ReadAndWrite) {
            if (progress === RegisterProgressState.Finish) bottom = centerEnd;
        }

        return `
      top: ${top};
      bottom: ${bottom};
    `;
    }}
`;

const StartArrow = styled.div`
    position: absolute;
    top: 50%;
    right: calc(50% - 1px);
    transform: translateY(-50%);
    width: 0;
    height: 0;
    border-top: ${ARROW_HEIGHT / 2}px solid transparent;
    border-bottom: ${ARROW_HEIGHT / 2}px solid transparent;
    border-right: 8px solid ${REG_COLOR};
    z-index: 1;
`;

const EndArrow = styled.div`
    position: absolute;
    top: 50%;
    left: calc(50% - 3px);
    width: 8px;
    height: 8px;
    border-top: ${LINE_WIDTH}px solid ${REG_COLOR};
    border-right: ${LINE_WIDTH}px solid ${REG_COLOR};
    transform: translate(0, -50%) rotate(45deg);
    background-color: transparent;
    z-index: 1;
`;

const DependencyContainer = styled.div<{ height: number }>`
  display: flex;
  height: ${props => props.height}px;
  align-items: center;
`;

// 单个轨道组件
const Track: React.FC<{
    track: RegisterTrack;
    height: number;
    width: number;
    cellPadding: number;
}> = ({ track, height, width, cellPadding }) => {
    const { name, state, length, progress } = track;

    if (state === RegisterState.Space) {
        return <TrackContainer width={width} height={height} />;
    }

    const tooltipTitle = `${name} (Length: ${length})`;

    return (
        <TrackContainer title={tooltipTitle} width={width} height={height}>
            <VerticalLine state={state} cellPadding={cellPadding} progress={progress} />
            {(state === RegisterState.Write || state === RegisterState.ReadAndWrite) && <StartArrow />}
            {(state === RegisterState.Read || state === RegisterState.ReadAndWrite) && <EndArrow />}
        </TrackContainer>
    );
};

// 主组件：寄存器依赖可视化
export const RegisterDependency: React.FC<RegisterDependencyProps> = ({
    tracks,
    height = 20,
    trackWidth = 16,
    cellPadding = 6,
}) => {
    return (
        <DependencyContainer height={height}>
            {tracks.map((track, index) => (
                <Track
                    key={index}
                    track={track}
                    height={height}
                    width={trackWidth}
                    cellPadding={cellPadding}
                />
            ))}
        </DependencyContainer>
    );
};
