/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import styled from '@emotion/styled';
import React, { useEffect, useMemo, useRef, useState } from 'react';
import { createPortal } from 'react-dom';
import type { Session } from '../../entity/session';
import { CHARTINTERACTOR_NAME } from '../ChartContainer/ChartContainer';
import { EventType, useEventBus } from '../../utils/eventBus';
import { PAGE_PADDING } from './ChartInteractor/draw';

export interface TooltipArg {
    x: number;
    y: number;
    /* Map<PropertyName, Value>
     * Tooltip display:
     * Property1: Value1
     * Property2: Value2
     * Property3: Value3
     * Property4: Value4
    */
    content: Map<string, string>;
};

// 默认展示在鼠标右侧，needReverse为true时转为左侧
const Tooltip = styled.div<{ needReverse?: boolean }>(props => ({
    position: 'absolute',
    backgroundColor: props.theme.bgColorLight,
    color: props.theme.textColorSecondary,
    borderRadius: 10,
    padding: '0 10px 0 10px',
    whiteSpace: 'pre',
    zIndex: 2,
    pointerEvents: 'none',
    transform: 'translatey(-50%)',
    boxShadow: props.theme.tooltipBoxShadow,
    lineHeight: 1.0,
    fontSize: '12px',
    '&::before': {
        position: 'absolute',
        content: '""',
        height: 10,
        width: 10,
        background: props.theme.bgColorLight,
        top: '50%',
        left: props.needReverse ? 'unset' : -2,
        right: props.needReverse ? -2 : 'unset',
        transform: props.needReverse ? 'translate(0, -50%) rotate(-45deg)' : 'translate(0, -50%) rotate(45deg)',
        zIndex: -1,
        pointerEvents: 'none',
    },
}));

const TooltipComp = (tooltipArg: TooltipArg): JSX.Element => {
    const tooltipDiv = useRef<HTMLDivElement>(null);
    const [table, setTable] = useState<JSX.Element[]>([]);
    const [needReverse, setReverse] = useState<boolean>(false);
    useEffect(() => {
        if (!tooltipDiv?.current?.parentElement) { return; }
        const nowX = tooltipArg.x;
        if (tooltipDiv.current.clientWidth + tooltipArg.x > tooltipDiv.current.parentElement.clientWidth) {
            tooltipDiv.current.style.left = `${nowX - tooltipDiv.current.clientWidth - 5}px`;
            setReverse(true);
        } else {
            tooltipDiv.current.style.left = `${nowX + 5}px`;
            setReverse(false);
        }
        tooltipDiv.current.style.top = `${tooltipArg.y}px`;
    });
    useEffect(() => {
        const tableContent: JSX.Element[] = [];
        const trContent: JSX.Element[][] = [];
        let row = 0;
        tooltipArg.content.forEach((value, key) => {
            trContent[row] = trContent[row] ?? [];
            trContent[row].push(<td style={{ textAlign: 'right' }} key={`key_${key}`}>{trContent[row].length === 0 ? key : `  ${key}`}</td>);
            trContent[row].push(<td key={`colon_${key}`}>:  </td>);
            trContent[row].push(<td style={{ textAlign: 'right' }} key={`value_${key}`}>{value}</td>);
            row = (row + 1) % 5;
        });
        trContent.map((value, index) => {
            return tableContent.push(<tr key={`tr_${index}`}>{value}</tr>);
        });
        setTable(tableContent);
    }, [tooltipArg.content]);
    return createPortal(
        <Tooltip ref={tooltipDiv} needReverse={needReverse}><table><tbody>{table}</tbody></table></Tooltip>,
        document.getElementById(CHARTINTERACTOR_NAME) as Element,
    );
};

export interface TooltipProps<F, T> {
    data?: F;
    session: Session;
    x?: (data: F) => number;
    mouseX: number | null;
    calcHeight: (data: F) => number;
    dataset: T;
    dom: React.RefObject<HTMLDivElement>;
    renderContent: (data: F) => Map<string, string> | undefined;
}

export function TooltipComponent<F, T>({ data, session, x, calcHeight, mouseX, dataset, dom, renderContent }: TooltipProps<F, T>): JSX.Element | null {
    const { domainStart, domainEnd } = session.domainRange;
    const [scrollTop, setScrollTop] = useState(0);
    // 监听scroll事件
    useEventBus(EventType.UNITWRAPPERSCROLL, (value) => setScrollTop(value as number));
    const [left, top] = useMemo(() => {
        if (data === undefined || mouseX === null) {
            return [null];
        }
        const _left = x ? x(data) : mouseX;
        const _top = calcHeight(data);
        const rect = dom.current?.getBoundingClientRect();
        return [_left, ((rect?.top ?? 0) - PAGE_PADDING) + _top];
    }, [domainStart, domainEnd, mouseX, dataset, scrollTop]);

    const content = data !== undefined ? renderContent(data) : undefined;
    if (!content || left === null) { return null; }

    return <TooltipComp x={left} content={content} y={top}/>;
}
