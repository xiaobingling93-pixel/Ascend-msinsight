/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */
import styled from '@emotion/styled';
import { clamp } from 'lodash';
import React, { useEffect, useMemo, useRef, useState } from 'react';
import { useWatchResize } from './useWatchDomResize';

interface CssProps {
    column: boolean;
    draggableWH: string;
    dragDirection: DragDirection;
    minWH: number;
    splitLineH: string;
}
export interface ViewProps {
    mainContainer: JSX.Element;
    draggableContainer?: JSX.Element;
    slot?: JSX.Element;
    id: string;
}

export enum DragDirection {
    'TOP' = 0,
    'BOTTOM' = 1,
    'LEFT' = 2,
    'RIGHT' = 3,
}

/**
 * @param dragDirection 拖动方向,DragDirection
 * @param draggableWH 可拖动容器默认宽/高
 * @param open 可拖动容器是否默认开启,可选
 */
interface DCProps {
    dragDirection: DragDirection;
    draggableWH: number;
    open?: boolean;
    splitLineRef: React.RefObject<HTMLDivElement>;
}

const MIN_HORIZONTAL_WH = 14;
const MIN_VERTICAL_WH = 10;

const ContainerBase = styled.div<CssProps>`
    display: flex;
    background-color: ${(p): string => p.theme.contentBackgroundColor};
    flex-grow: 1;
    overflow: hidden;
    width: 100%;

    .bottomC {
        background-color: ${(p): string => p.theme.contentBackgroundColor};
        svg + .buttonShow {
            position: absolute;
            g {
                fill: ${(p): string => p.theme.closeDragContainerBG};
            }
            .caret {
                position: absolute;
                cursor: pointer;
                top: 50%;
                right: 0;
                color: ${(p): string => p.theme.switchIconColor};
                svg {
                    width: 10px;
                }
            }
        }

        & > .dragContainer {
            height: 100%;
            width: 100%;
        }
    }
`;
const ContainerLeft = styled(ContainerBase)`
    flex-direction: row;
    & > .topC {
        flex-flow: row;
        overflow: hidden;
        position: relative;
        width: ${(p): string => p.draggableWH};
        & > .dragContainer {
            height: 100%;
            z-index: 1;
        }
        & > .dragContainer[aria-disabled=true] {
            border-right: ${(p): string => p.theme.dividerColor} 2px solid;
            padding-right: 15px;
        }
        & > .splitLine {
            position: absolute;
            height: 100%;
            width: 10px;
            top: 0;
            right: 0;
            z-index: 1;
            background-color: transparent;
            border-right: ${(p): string => p.theme.dividerColor} 0 solid;
            &:hover[aria-disabled=true] {
                border-right-width: 3px;
                cursor: e-resize;
            }
        }
    }
    & > .bottomC {
        flex: 1;
        height: 100%;
        overflow: hidden;
        display: flex;
    }
`;
const ContainerRight = styled(ContainerBase)`
    flex-direction: row-reverse;
    & > .topC {
        flex-flow: row;
        overflow: hidden;
        position: relative;
        width: ${(p): string => p.draggableWH};
        & > .dragContainer {
            height: 100%;
            z-index: 1;
        }
        & > .dragContainer[aria-disabled=true] {
            border-left: ${(p): string => p.theme.dividerColor} 2px solid;
            padding-left: 15px;
        }
        & > .splitLine {
            position: absolute;
            height: 100%;
            width: 10px;
            top: 0;
            left: 0;
            z-index: 1;
            background-color: transparent;
            border-left: ${(p): string => p.theme.dividerColor} 0 solid;
            &:hover[aria-disabled=true] {
                border-left-width: 3px;
                cursor: e-resize;
            }
        }
    }
    & > .bottomC {
        flex: 1;
        height: 100%;
        overflow: hidden;
        display: flex;
    }
`;
const ContainerBottom = styled(ContainerBase)`
    flex-direction: column-reverse;
    & > .topC {
        width: 100%;
        height: ${(p): string => p.draggableWH};
        position: relative;

        & > .dragContainer {
            display: flex;
            height: 100%;
        }

        & > .splitLine {
            position: absolute;
            z-index: 3;
            height: ${(p): string => p.splitLineH};
            width: 100%;
            top: 0;
            background-color: transparent;
            border-top: ${(p): string => p.theme.dividerColor} 0 solid;
            &:hover[aria-disabled=true] {
                border-top-width: 1px;
                cursor: n-resize;
            }
        }
    }

    & > .bottomC {
        width: 100%;
        border-bottom: ${(p): string => p.theme.dividerColor} 2px solid;
        flex: 1;
        flex-flow: row;
        overflow: hidden;
        display: flex;
    }
    & > .bottomC::before {
        width: 100%;
        top: -10px;
        left: 0;
    }
`;
const ContainerTop = styled(ContainerBase)`
    flex-direction: column;
    & > .topC {
        width: 100%;
        height: ${(p): string => p.draggableWH};
        position: relative;

        & > .dragContainer {
            display: flex;
            height: 100%;
        }

        & > .splitLine {
            position: absolute;
            z-index: 3;
            height: ${(p): string => p.splitLineH};
            width: 100%;
            bottom: 0;
            background-color: transparent;
            border-bottom: ${(p): string => p.theme.dividerColor} 0 solid;
            &:hover[aria-disabled=true] {
                border-bottom-width: 1px;
                cursor: n-resize;
            }
        }
    }

    & > .bottomC {
        width: 100%;
        border-top: ${(p): string => p.theme.dividerColor} 2px solid;
        flex: 1;
        flex-flow: row;
        overflow: hidden;
        display: flex;
    }
    & > .bottomC::before {
        width: 100%;
        top: -10px;
        left: 0;
    }
`;

interface MovingState {
    stat: 'idle' | 'movable' | 'moved';
    startX: number;
    startY: number;
    screenX: number;
    screenY: number;
}
const getHandleMouseDown = (dragDirection: DragDirection, draggable: React.RefObject<HTMLDivElement>,
    movingState: React.MutableRefObject<MovingState>, isOpen: React.MutableRefObject<boolean>) => (e: MouseEvent): void => {
    const domDrag = draggable.current;
    if (!domDrag) { return; }
    let offset; const baseMS: MovingState = { stat: 'movable', startX: 0, startY: 0, screenX: e.screenX, screenY: e.screenY };
    const domDragRect = domDrag.getBoundingClientRect();
    switch (dragDirection) {
        case DragDirection.TOP:
            offset = domDragRect.bottom - e.clientY;
            if (offset <= 10 && offset > 0 && isOpen.current) {
                movingState.current = {
                    ...baseMS,
                    startX: domDragRect.x,
                    startY: domDragRect.bottom,
                };
            }
            break;
        case DragDirection.BOTTOM:
            offset = e.clientY - domDragRect.top;
            if (offset <= 10 && offset > 0 && isOpen.current) {
                movingState.current = {
                    ...baseMS,
                    startX: domDragRect.x,
                    startY: domDragRect.top,
                };
            }
            break;
        case DragDirection.LEFT:
            offset = domDragRect.right - e.clientX;
            if (offset <= 10 && offset > 0 && isOpen.current) {
                movingState.current = {
                    ...baseMS,
                    startX: domDragRect.left,
                    startY: domDragRect.y,
                };
            }
            break;
        default:
            offset = e.clientX - domDragRect.left;
            if (offset <= 10 && offset > 0 && isOpen.current) {
                movingState.current = {
                    ...baseMS,
                    startX: domDragRect.right,
                    startY: domDragRect.y,
                };
            }
            break;
    }
};

const RIGHT_PERCENT = 0.99;

interface ImouseAction {
    container: React.RefObject<HTMLDivElement>;
    draggable: React.RefObject<HTMLDivElement>;
    movingState: React.MutableRefObject<MovingState>;
    dragDirection: DragDirection;
    MIN_DRAG_WH: number;
    containerOffsetTop: number;
}
const handleMouseMove = (params: ImouseAction) => (e: MouseEvent): void => {
    const { container, draggable, movingState, dragDirection, MIN_DRAG_WH, containerOffsetTop } = params;
    const dom = container.current;
    const domDrag = draggable.current;
    const moving = movingState.current;
    if (e.buttons !== 1) {
        moving.stat = 'idle';
        return;
    }
    if (!dom || !domDrag) { return; }
    if (moving.stat === 'idle') { return; }
    if (Math.abs(e.screenY - moving.screenY) < 2 && Math.abs(e.screenX - moving.screenX) < 2) { return; }
    let offsetY: number;
    let offsetX: number;
    switch (dragDirection) {
        case DragDirection.BOTTOM:
            offsetY = e.y - moving.startY;
            if (Math.abs(offsetY) >= 5) {
                domDrag.style.height = `${clamp(dom.clientHeight - e.y + containerOffsetTop, MIN_DRAG_WH, dom.clientHeight - MIN_DRAG_WH)}px`;
            }
            break;
        case DragDirection.TOP:
            offsetY = e.y - moving.startY;
            if (Math.abs(offsetY) >= 5) {
                domDrag.style.height = `${clamp(e.y - containerOffsetTop, MIN_DRAG_WH, dom.clientHeight - MIN_DRAG_WH)}px`;
            }
            break;
        case DragDirection.LEFT:
            offsetX = e.x - moving.startX;
            if (Math.abs(offsetX) >= 5) {
                domDrag.style.width = `${clamp(e.x, 245, dom.clientWidth * 0.4)}px`;
            }
            break;
        default:
            offsetX = e.x - moving.startX;
            if (Math.abs(offsetX) >= 5) {
                domDrag.style.width = `${clamp(moving.startX - e.clientX, MIN_DRAG_WH, dom.clientWidth * RIGHT_PERCENT)}px`;
            }
            break;
    }
    moving.stat = 'moved';
};

const handleMouseUp = (params: ImouseAction) => (e: MouseEvent): void => {
    const { container, draggable, movingState, dragDirection, MIN_DRAG_WH, containerOffsetTop } = params;
    const dom = container.current;
    const domDrag = draggable.current;
    const moving = movingState.current;
    const isDomInvalid = !dom || !domDrag || dom.clientHeight === 0 || dom.clientWidth === 0;
    if (moving.stat !== 'moved' || isDomInvalid) {
        moving.stat = 'idle';
        return;
    }
    let dragWHTmp: number;
    switch (dragDirection) {
        case DragDirection.TOP:
            dragWHTmp = clamp(e.y - containerOffsetTop, MIN_DRAG_WH, dom.clientHeight - MIN_DRAG_WH);
            domDrag.style.height = `${dragWHTmp / dom.clientHeight * 100}%`;
            window.dispatchEvent(new Event('topResize'));
            break;
        case DragDirection.BOTTOM:
            dragWHTmp = clamp(dom.clientHeight - e.y + containerOffsetTop, MIN_DRAG_WH, dom.clientHeight - MIN_DRAG_WH);
            domDrag.style.height = `${dragWHTmp / dom.clientHeight * 100}%`;
            window.dispatchEvent(new Event('bottomResize'));
            break;
        case DragDirection.LEFT:
            dragWHTmp = clamp(e.clientX, 245, dom.clientWidth * 0.4);
            domDrag.style.width = `${dragWHTmp / dom.clientWidth * 100}%`;
            window.dispatchEvent(new Event('leftResize'));
            break;
        case DragDirection.RIGHT:
            dragWHTmp = clamp(moving.startX - e.clientX, MIN_DRAG_WH, dom.clientWidth * RIGHT_PERCENT);
            domDrag.style.width = `${dragWHTmp / dom.clientWidth * 100}%`;
            window.dispatchEvent(new Event('rightResize'));
            break;
        default:
            break;
    }
    movingState.current = {
        stat: 'idle',
        startX: 0,
        startY: 0,
        screenY: 0,
        screenX: 0,
    };
    window.dispatchEvent(new Event('resize'));
};

// 主容器宽高未确定时,初始化为设定的px值,主容器挂载结束后,后续宽高设置为百分比
const pxConvert = (originPx: number, container: number[], dragDirection: DragDirection): string => {
    let px = originPx;
    if (container[0] === 0 || container[1] === 0) { return `${px}px`; }
    if (dragDirection <= 1) {
        if (dragDirection === 1) { px += 4; } // bottom面板需要加上分割线的宽度
        return `${px / container[1] * 100}%`;
    } else {
        return `${px / container[0] * 100}%`;
    }
};

const containerMap: Map<DragDirection, typeof ContainerBase> = new Map([
    [DragDirection.TOP, ContainerTop],
    [DragDirection.BOTTOM, ContainerBottom],
    [DragDirection.LEFT, ContainerLeft],
    [DragDirection.RIGHT, ContainerRight],
]);

const getOffsetTop = (ele: HTMLElement): number => {
    return (
        ele.offsetTop + (ele.offsetParent ? getOffsetTop(ele.offsetParent as HTMLElement) : 0)
    );
};

interface Iswitch {
    dragDirection: DragDirection;
    containerWH: number[];
    isOpen: React.MutableRefObject<boolean>;
    MIN_DRAG_WH: number;
    draggable: React.RefObject<HTMLDivElement>;
    dragTranslate: number;
    setDragTranslate: React.Dispatch<React.SetStateAction<number>>;
    needOpen: boolean;
}

const switchOpen = (params: Iswitch): void => {
    const { dragDirection, containerWH, isOpen, MIN_DRAG_WH, draggable, dragTranslate, setDragTranslate, needOpen } = params;
    const domDrag = draggable.current; if (!domDrag) { return; }
    if (dragDirection <= 1) {
        if (needOpen) {
            if (!isOpen.current) { // close -> open
                domDrag.style.height = `${pxConvert(dragTranslate, containerWH, dragDirection)}`;
            }
        } else {
            if (isOpen.current) { // open -> close
                setDragTranslate(domDrag.clientHeight);
            }
            domDrag.style.height = '0px';
        }
    } else {
        if (needOpen) {
            if (!isOpen.current) { // close -> open
                domDrag.style.width = `${pxConvert(dragTranslate, containerWH, dragDirection)}`;
            }
        } else {
            if (isOpen.current) { // open -> close
                setDragTranslate(domDrag.clientWidth);
            }
            domDrag.style.width = `${MIN_DRAG_WH}px`;
        }
    }
    isOpen.current = needOpen;
    window.dispatchEvent(new Event('resize'));
};

/**
 * 在当前布局下创建两个容器，其中draggable容器可拖动改变宽/高，main容器自适应改变，根据传入拖动方向不同，提供leftResize/rightResize等事件
 * @param props
 * @return [view, handleSwitchOpen]
 * view：可拖动布局构造函数；
 * handleSwitchOpen：显示/隐藏可拖动容器；
 */
export const useDraggableContainerEx = (props: DCProps): [ ((props: ViewProps) => JSX.Element), ((needOpen: boolean) => void) ] => {
    const { draggableWH, dragDirection, splitLineRef, open = true } = props;
    const [containerHeight, container] = useWatchResize<HTMLDivElement>('height');
    const draggable = useRef<HTMLDivElement>(null);
    const [dragWh, setDragWh] = useState(String(draggableWH));
    const [containerWH, setContainerWH] = useState([0, 0]);
    const [containerOffsetTop, setContainerOffsetTop] = useState(0);
    useEffect(() => { setDragWh(pxConvert(draggableWH, containerWH, dragDirection)); }, [draggableWH, containerWH, dragDirection]);
    const MIN_DRAG_WH = useMemo(() => dragDirection <= 1 ? MIN_VERTICAL_WH : MIN_HORIZONTAL_WH, [dragDirection]);
    const [dragTranslate, setDragTranslate] = useState(open ? 0 : draggableWH); // 可拖动的距离范围。0 | 具体某个值
    const isOpen = useRef(open);

    useEffect(() => {
        const dom = container.current;
        if (dom) {
            setContainerWH([dom.clientWidth, dom.clientHeight]);
        }
    }, [setContainerWH, containerHeight]);
    useEffect(() => {
        const dom = container.current;
        if (dom) {
            const offsetTop = getOffsetTop(dom);
            setContainerOffsetTop(offsetTop);
        }
    }, [setContainerOffsetTop]);
    const movingState = useRef<MovingState>({ stat: 'idle', startX: 0, startY: 0, screenY: 0, screenX: 0 });
    const onMousedown = getHandleMouseDown(dragDirection, draggable, movingState, isOpen);
    const onMousemove = handleMouseMove({ container, draggable, movingState, dragDirection, MIN_DRAG_WH, containerOffsetTop });
    const onMouseup = handleMouseUp({ container, draggable, movingState, dragDirection, MIN_DRAG_WH, containerOffsetTop });
    const handleSwitchOpen = (needOpen: boolean): void => {
        switchOpen({
            dragDirection,
            containerWH,
            isOpen,
            MIN_DRAG_WH,
            draggable,
            dragTranslate,
            setDragTranslate,
            needOpen,
        });
    };
    const Container = containerMap.get(dragDirection) as typeof ContainerBase;
    const view = (vProps: ViewProps): JSX.Element => {
        return <Container key={vProps.id} ref={container} column draggableWH={open ? dragWh : '0px'}
            splitLineH={open ? '10px' : '0px'} dragDirection={dragDirection} minWH={MIN_DRAG_WH}
            onMouseUp={(e): void => onMouseup(e.nativeEvent)} onMouseDown={(e): void => onMousedown(e.nativeEvent)}
            onMouseMove={(e): void => onMousemove(e.nativeEvent)}>
            <div className={'topC'} ref={draggable}>
                <div className={'dragContainer'} aria-disabled={isOpen.current}>{vProps.draggableContainer}</div>
                <div className={'splitLine'} aria-disabled={isOpen.current} ref={splitLineRef} />
            </div>
            <div className={'bottomC'}> {vProps.mainContainer} </div>
            {vProps.slot}
        </Container>;
    };
    return [view, handleSwitchOpen];
};
