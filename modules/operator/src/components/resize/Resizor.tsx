/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { useRef } from 'react';
import './Resizor.css';

const Resizor = (props: any): JSX.Element => {
    const divRef: any = useRef(null);
    let isDown = false;
    let offsetX: number;
    let initalWidth: number;
    let initalNextWidth: number;

    function handleMouseDown(event: any): void {
        event.preventDefault();
        if (!isDown) {
            isDown = true;
            offsetX = event.clientX;
            window.addEventListener('mousemove', handleMouseMove);
            window.addEventListener('mouseup', handleMouseUp);
            const dom = divRef.current.parentNode;
            dom.style.pointerEvents = 'none';
            initalWidth = dom.offsetWidth;
            const nextBrother = dom.nextElementSibling;
            if (nextBrother !== null) {
                initalNextWidth = nextBrother.offsetWidth;
            }
        }
    };

    function handleMouseMove(event: any): void {
        event.preventDefault();
        if (isDown && divRef?.current as boolean) {
            const deltaX = event.clientX - offsetX;
            const dom = divRef.current.parentNode;
            const width = initalWidth + deltaX;
            const nextBrother = dom.nextElementSibling;
            if (nextBrother !== null) {
                const nextWidth = initalNextWidth - deltaX;
                props.onResize(deltaX, width, nextWidth);
                return;
            }
            props.onResize(deltaX, width);
        }
    }
    function handleMouseUp(event: any): void {
        event.preventDefault();
        isDown = false;
        offsetX = 0;
        window.removeEventListener('mousemove', handleMouseMove);
        window.removeEventListener('mouseup', handleMouseUp);
        const parentNode = divRef.current.parentNode;
        parentNode.style.pointerEvents = null;
    }

    return <div ref={divRef} className={'resizor'} onMouseDown={handleMouseDown} ></div>;
};
export default Resizor;
