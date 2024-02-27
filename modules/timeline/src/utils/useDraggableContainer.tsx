/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */
import { CaretLeftOutlined, CaretRightOutlined } from '@ant-design/icons';
import styled from '@emotion/styled';
import { clamp } from 'lodash';
import React, { useEffect, useMemo, useRef, useState } from 'react';
import { ReactComponent as DrawerButton } from '../assets/images/toggle_bg_light.svg';

interface CssProps {
    column: boolean;
    translateXY: number;
    draggableWH: string;
    dragDirection: DragDirection;
    minWH: number;
}
export interface ViewProps {
    mainContainer: JSX.Element;
    draggableContainer?: JSX.Element;
    slot?: JSX.Element;
    id: string;
}

export enum DragDirection {
    'top',
    'bottom',
    'left',
    'right',
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
}

const MIN_HORIZONTAL_WH = 14;
const MIN_VERTICAL_WH = 36;

const ContainerBase = styled.div<CssProps>`
    display: flex;
    background-color: ${p => p.theme.contentBackgroundColor};
    flex-grow: 1;
    overflow: hidden;
    width: 100%;

    .bottomC {
        background-color: ${p => p.theme.contentBackgroundColor};
        svg + .buttonShow {
            position: absolute;
            g {
                fill: ${props => props.theme.closeDragContainerBG};
            }
            .caret {
                position: absolute;
                cursor: pointer;
                top: 50%;
                right: 0;
                color: ${p => p.theme.switchIconColor};
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
    flex-direction: row-reverse;
    border-bottom: ${p => p.theme.dividerColor} 1px solid;
    & > .topC {
        flex: 1;
        flex-flow: row;
        overflow: hidden;
    }
    & > .bottomC {
        position: relative;
        height: 100%;
        width: ${p => p.draggableWH};
        border-right: ${p => p.theme.dividerColor} 2px solid;
        overflow: hidden;
        display: flex;
        & > .splitLine {
            position: absolute;
            height: 100%;
            width: 10px;
            right: 0;
            background-color: transparent;
            border-right: ${p => p.theme.dividerColor} 0 solid;
            &:hover[aria-disabled=false] {
                border-right-width: 1px;
                cursor: e-resize;
            }
        }
        & > .buttonShow {
            position: absolute;
            cursor: pointer;
            top: 50%;
            z-index: 2;
            transform: rotate(90deg);
            right: -33px;
            g {
                fill: ${props => props.theme.closeDragContainerBG};
            }
        }
        & > .caret {
            position: absolute;
            cursor: pointer;
            transform: rotate(180deg);
            z-index: 2;
            top: 50%;
            right: 0;
            color: ${p => p.theme.switchIconColor};
            svg {
                width: 10px;
            }
        }
    }
`;
const ContainerRight = styled(ContainerBase)`
    flex-direction: row;
    height: 100%;
    & > .topC {
        flex: 1;
        flex-flow: row;
        overflow: hidden;
    }
    & > .bottomC {
        position: relative;
        height: 100%;
        width: ${p => p.draggableWH};
        overflow: hidden;
        display: flex;
        & > .dragContainer {
            z-index: 1;
        }
        & > .dragContainer[aria-disabled=true] {
            border-left: ${p => p.theme.dividerColor} 2px solid;
            padding-left: 15px;
        }
        & > .splitLine {
            position: absolute;
            height: 100%;
            width: 10px;
            left: 0;
            z-index: 1;
            background-color: transparent;
            border-left: ${p => p.theme.dividerColor} 0 solid;
            &:hover[aria-disabled=false] {
                border-left-width: 3px;
                cursor: e-resize;
            }
        }
        & > .buttonShow {
            position: absolute;
            cursor: pointer;
            top: 50%;
            z-index: 2;
            transform: rotate(-90deg);
            left: -33px;
            g {
                fill: ${props => props.theme.closeDragContainerBG};
            }
        }
        & > .caret {
            position: absolute;
            cursor: pointer;
            z-index: 2;
            top: 50%;
            left: 2px;
            color: ${p => p.theme.switchIconColor};
            svg {
                width: 10px;
            }
        }
    }
`;
const ContainerBottom = styled(ContainerBase)`
    flex-direction: column;
    & > .topC {
        width: 100%;
        flex: 1;
        flex-flow: row;
        overflow: hidden;
    }

    & > .bottomC {
        width: 100%;
        height: ${p => p.draggableWH};
        border-top: ${p => p.theme.dividerColor} 2px solid;
        position: relative;
        & > .splitLine {
            position: absolute;
            z-index: 3;
            height: 10px;
            width: 100%;
            top: 0;
            background-color: transparent;
            border-top: ${p => p.theme.dividerColor} 0 solid;
            &:hover[aria-disabled=false] {
                border-top-width: 1px;
                cursor: n-resize;
            }
        }
        & > .buttonShow {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: calc(50% - 39px);
            z-index: 4;

            g {
                fill: ${p => p.theme.closeDragContainerBG};
            }
        }

        & > .caret {
            position: absolute;
            cursor: pointer;
            z-index: 4;
            left: calc(50% - 6px);;
            top: -2px;
            transform: rotate(90deg);
            color: ${p => p.theme.switchIconColor};

            svg {
                width: 10px;
            }
        }
    }
    & > .bottomC::before {
        width: 100%;
        top: -10px;
        left: 0;
    }

`;
const ContainerTop = styled(ContainerBase)`
    flex-direction: column-reverse;
    .topC {
      height: calc(100vh - ${p => p.translateXY === 0 ? p.draggableWH : p.minWH}px);
    }
    .bottomC {
        top: 0;
        left: 0;
        right: 0;
        height: ${p => p.draggableWH}px;
        .buttonShow {
            position: absolute;
            bottom: 0;
            left: calc(50% - 39px);
            z-index: 4;
        }
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
        case DragDirection.top:
            offset = domDragRect.bottom - e.clientY;
            if (offset <= 8 && offset > 0 && isOpen.current) {
                movingState.current = {
                    ...baseMS,
                    startX: domDragRect.x,
                    startY: domDragRect.bottom,
                };
            }
            break;
        case DragDirection.bottom:
            offset = e.clientY - domDragRect.top;
            if (offset <= 8 && offset > 0 && isOpen.current) {
                movingState.current = {
                    ...baseMS,
                    startX: domDragRect.x,
                    startY: domDragRect.top,
                };
            }
            break;
        case DragDirection.left:
            offset = domDragRect.right - e.clientX;
            if (offset <= 8 && offset > 0 && isOpen.current) {
                movingState.current = {
                    ...baseMS,
                    startX: domDragRect.left,
                    startY: domDragRect.y,
                };
            }
            break;
        default:
            offset = e.clientX - domDragRect.left;
            if (offset <= 8 && offset > 0 && isOpen.current) {
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

const handleMouseMove = (container: React.RefObject<HTMLDivElement>, draggable: React.RefObject<HTMLDivElement>, movingState: React.MutableRefObject<MovingState>,
    dragDirection: DragDirection, MIN_DRAG_WH: number) => (e: MouseEvent): void => {
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
    let offsetY: number, offsetX: number;
    switch (dragDirection) {
        case DragDirection.bottom:
            offsetY = e.y - moving.startY;
            if (Math.abs(offsetY) >= 5) {
                domDrag.style.height = `${clamp(dom.clientHeight - e.y, MIN_DRAG_WH, dom.clientHeight - MIN_DRAG_WH)}px`;
            }
            break;
        case DragDirection.top:
            offsetY = e.y - moving.startY;
            if (Math.abs(offsetY) >= 5) {
                domDrag.style.height = `${clamp(e.y, MIN_DRAG_WH, dom.clientHeight - MIN_DRAG_WH)}px`;
            }
            break;
        case DragDirection.left:
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

const handleMouseUp = (container: React.RefObject<HTMLDivElement>, draggable: React.RefObject<HTMLDivElement>, movingState: React.MutableRefObject<MovingState>,
    dragDirection: DragDirection, MIN_DRAG_WH: number) => (e: MouseEvent): void => {
    const dom = container.current;
    const domDrag = draggable.current;
    const moving = movingState.current;
    if (moving.stat !== 'moved' || !dom || !domDrag) { moving.stat = 'idle'; return; }
    let dragWHTmp: number;
    switch (dragDirection) {
        case DragDirection.top:
            dragWHTmp = clamp(e.y, MIN_DRAG_WH, dom.clientHeight - MIN_DRAG_WH);
            domDrag.style.height = `${dragWHTmp / dom.clientHeight * 100}%`;
            window.dispatchEvent(new Event('topResize'));
            break;
        case DragDirection.bottom:
            dragWHTmp = clamp(dom.clientHeight - e.y, MIN_DRAG_WH, dom.clientHeight - MIN_DRAG_WH);
            domDrag.style.height = `${dragWHTmp / dom.clientHeight * 100}%`;
            window.dispatchEvent(new Event('bottomResize'));
            break;
        case DragDirection.left:
            dragWHTmp = clamp(e.clientX, 245, dom.clientWidth * 0.4);
            domDrag.style.width = `${dragWHTmp / dom.clientWidth * 100}%`;
            window.dispatchEvent(new Event('leftResize'));
            break;
        case DragDirection.right:
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
const pxConvert = (px: number, container: number[], dragDirection: DragDirection): string => {
    if (container[0] === 0 || container[1] === 0) { return String(px) + 'px'; }
    if (dragDirection <= 1) {
        if (dragDirection === 1) { px += 4; } // bottom面板需要加上分割线的宽度
        return String(px / container[1] * 100) + '%';
    } else {
        return String(px / container[0] * 100) + '%';
    }
};

const handleDraggableShow = (dragDirection: DragDirection, container: number[], isOpen: React.MutableRefObject<boolean>, MIN_DRAG_WH: number, draggable: React.RefObject<HTMLDivElement>,
    dragTranslate: number, setDragTranslate: React.Dispatch<React.SetStateAction<number>>) => (): void => {
    const domDrag = draggable.current; if (!domDrag) { return; }
    if (dragDirection <= 1) {
        if (isOpen.current) { // open -> close
            setDragTranslate(domDrag.clientHeight);
            domDrag.style.height = `${MIN_DRAG_WH}px`;
        } else { // close -> open
            domDrag.style.height = `${pxConvert(dragTranslate, container, dragDirection)}`;
            setDragTranslate(0);
        }
    } else {
        if (isOpen.current) { // open -> close
            setDragTranslate(domDrag.clientWidth);
            domDrag.style.width = `${MIN_DRAG_WH}px`;
        } else { // close -> open
            domDrag.style.width = `${pxConvert(dragTranslate, container, dragDirection)}`;
            setDragTranslate(0);
        }
    }
    isOpen.current = !isOpen.current;
    window.dispatchEvent(new Event('resize'));
};

const containerMap: Map<DragDirection, typeof ContainerBase> = new Map([
    [DragDirection.top, ContainerTop],
    [DragDirection.bottom, ContainerBottom],
    [DragDirection.left, ContainerLeft],
    [DragDirection.right, ContainerRight],
]);

/**
 * 在当前布局下创建两个容器，其中draggable容器可拖动改变宽/高，可点击收起隐藏，main容器自适应改变，根据传入拖动方向不同，提供leftResize/rightResize等事件
 * @param props
 * @return [view, handleOpen]
 * view：可拖动布局构造函数；
 * handleOpen：显示/隐藏可拖动容器；
 */
export const useDraggableContainer = (props: DCProps): [ ((props: ViewProps) => JSX.Element), ((needOpen?: boolean) => void) ] => {
    const { draggableWH, dragDirection, open = true } = props;
    const container = useRef<HTMLDivElement>(null); const draggable = useRef<HTMLDivElement>(null);
    const [dragWh, setDragWh] = useState(String(draggableWH));
    const [autoPopUp, setAutoPopUp] = useState(true);
    const [containerWH, setContainerWH] = useState([0, 0]);
    useEffect(() => { setDragWh(pxConvert(draggableWH, containerWH, dragDirection)); }, [draggableWH, containerWH, dragDirection]);
    const MIN_DRAG_WH = useMemo(() => dragDirection <= 1 ? MIN_VERTICAL_WH : MIN_HORIZONTAL_WH, [dragDirection]);
    const [dragTranslate, setDragTranslate] = useState(open ? 0 : draggableWH); // 可拖动的距离范围。0 | 具体某个值
    const isOpen = useRef(dragTranslate === 0);
    useEffect(() => {
        const dom = container.current;
        if (dom) {
            setContainerWH([dom.clientWidth, dom.clientHeight]);
        }
    }, [isOpen.current, setContainerWH]);
    const movingState = useRef<MovingState>({ stat: 'idle', startX: 0, startY: 0, screenY: 0, screenX: 0 });
    const onMousedown = getHandleMouseDown(dragDirection, draggable, movingState, isOpen);
    const onMousemove = handleMouseMove(container, draggable, movingState, dragDirection, MIN_DRAG_WH);
    const onMouseup = handleMouseUp(container, draggable, movingState, dragDirection, MIN_DRAG_WH);
    const showDraggable = handleDraggableShow(dragDirection, containerWH, isOpen, MIN_DRAG_WH, draggable, dragTranslate, setDragTranslate);
    const handleOpen = (needOpen = false): void => { if (needOpen && !isOpen.current && autoPopUp) { showDraggable(); setAutoPopUp(false); } };
    const Container = containerMap.get(dragDirection) as typeof ContainerBase;
    // 分割线混乱问题解决
    useEffect(() => {
        if (!isOpen.current && dragTranslate !== 0 && draggable.current && dragDirection === DragDirection.right) {
            draggable.current.style.width = `${MIN_DRAG_WH}px`;
        }
    });
    const view = (props: ViewProps): JSX.Element => {
        return <Container key={props.id} ref={container} column translateXY={dragTranslate} draggableWH={open ? dragWh : pxConvert(MIN_DRAG_WH, containerWH, dragDirection)} dragDirection={dragDirection} minWH={MIN_DRAG_WH}
            onMouseUp={e => onMouseup(e.nativeEvent)} onMouseDown={e => onMousedown(e.nativeEvent)} onMouseMove={e => onMousemove(e.nativeEvent)}>
            <div className={'topC'}> {props.mainContainer} </div>
            <div className={'bottomC'} ref={draggable}>
                <div className={'dragContainer'} aria-disabled={dragTranslate !== 0}>{props.draggableContainer}</div>
                <DrawerButton className={'buttonShow'} onClick={() => showDraggable()} />
                {dragTranslate ? <CaretLeftOutlined onClick={() => showDraggable()} className={'caret'}/> : <CaretRightOutlined onClick={() => showDraggable()} className={'caret'}/>}
                <div className={'splitLine'} aria-disabled={dragTranslate !== 0} />
            </div>
            {props.slot}
        </Container>;
    };
    return [view, handleOpen];
};
